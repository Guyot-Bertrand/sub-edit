#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/seeking.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <mpv/client.h>
#include <mpv/render.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

/// What every player is built with.
///
/// None of the four is decoration. `config=no` keeps a developer's own
/// `~/.config/mpv` from deciding whether the gate passes; `terminal=no` keeps
/// mpv's chatter off our output; `pause=yes` because opening a film is not
/// watching it, and the window would have to stop it again on the next line.
///
/// **`sub-auto=no` is decision D2, held where it would otherwise be lost.**
/// Handed `film.mp4`, mpv loads the `film.srt` lying beside it of its own
/// accord — measured, and it is exactly the file being edited. The picture
/// would then show what the disk holds while the table shows what was typed,
/// the two parting company at the first keystroke, and the replica this player
/// draws would land on top of a stale one. The overlay is the only subtitle
/// this player is ever to know.
///
/// **`hwdec=auto-copy` is issue #647**: the card decodes and the picture is copied back, which
/// the software render accepts where it refuses the zero-copy kind. A machine without a card
/// decodes on the processor all the same; `Preferences…` can turn it off through
/// `setHardwareDecoding`.
///
/// **`keep-open=yes` is what lets a film end without being lost** — issue #614.
/// By default mpv unloads the file when playback reaches its end, and the next
/// question — the position, the duration, a step back — is answered « property
/// unavailable ». Measured: one `frame-step` past the last frame and the film was
/// gone. Kept open, the film stays on its last frame, held, and everything asked
/// of it afterwards still has an answer; `isPlaying` says it stopped.
constexpr std::array<std::pair<const char*, const char*>, 7> kEveryPlayer{{
    {"config", "no"},
    {"terminal", "no"},
    {"pause", "yes"},
    {"sub-auto", "no"},
    {"keep-open", "yes"},
    {"vo", "libmpv"},
    {"hwdec", "auto-copy"},
}};

/// What a player that makes no sound is built with — the shape of every test.
///
/// `ao=null` is what a runner without a sound device needs, and a player nobody can see
/// is not one anybody should hear. **`vo=libmpv` is in `kEveryPlayer`**, and it is what
/// lets the same player run with no screen — measured: it opens, seeks and answers
/// without a window or a display, where `vo=auto` loaded nothing — ADR 0041.
///
/// A player that is heard is left mpv's own audio output. Checking that a subtitle lands
/// on the right line is done as much by ear as by eye, and a `subedit` that played films
/// silently would have made that harder for the sake of one shared constant.
constexpr std::pair<const char*, const char*> kSilent{"ao", "null"};

/// How long one wait for an event may take.
///
/// Generous on purpose: this is not a budget but a way out of a wait that
/// would otherwise hold the caller forever. Opening a file takes milliseconds.
constexpr double kEventTimeoutSeconds = 5.0;

/// How many events may go by before a wait gives up on the one it wants.
constexpr int kMaxEventsAwaited = 100;

constexpr double kMillisecondsPerSecond = 1000.0;

/// Which overlay of libmpv the replica is drawn on.
///
/// One and always the same: an overlay is replaced by writing to its own
/// number, so a player that varied it would stack every line it ever drew.
constexpr const char* kOverlayId = "1";

/// The height the overlay's coordinates are read against.
///
/// mpv's own default for this command. It is what makes the text scale with
/// the picture rather than with the window: a film played in a corner and one
/// played full screen get a replica of the same relative size.
constexpr const char* kOverlayHeight = "720";

/// How many words the overlay command takes, the closing `nullptr` counted.
constexpr std::size_t kOverlayCommandWords = 10;

/// Where the replica sits: centred, at the foot of the picture, which is where
/// a viewer's eye already goes looking for it.
constexpr const char* kBottomCentre = "{\\an2}";

/// What a player that could not be built answers.
constexpr const char* kNotStarted = "the video player could not be started";

/// How many parameters a software render takes, the closing invalid one counted.
constexpr std::size_t kRenderParameters = 6;

/// How long one slice of a wait lasts, in seconds: between two slices the render context
/// is serviced. Short enough that a frame announced during a wait is taken at once.
constexpr double kPumpSeconds = 0.005;

/// A buffer of one pixel, which is all a frame needs to be taken when nobody wants to
/// see it.
constexpr int kScratchSide = 1;

/// Takes the picture libmpv has announced, drawing it nowhere that matters.
///
/// **A caller that waits holds the thread the window paints on**, and with `vo=libmpv`
/// the output waits for each picture to be rendered — measured, issue #611: up to 200 ms
/// a picture, so a seek that waited for the first one stalled 400 ms, whatever the
/// distance. Taking the frame here, into one pixel, is what lets the output go on; the
/// widget draws the real picture at its own size when the wait is over and the event
/// loop runs again.
void serviceRender(mpv_render_context* render) {
    if (render == nullptr || (mpv_render_context_update(render) & MPV_RENDER_UPDATE_FRAME) == 0U)
        return;

    std::array<int, 2> size{kScratchSide, kScratchSide};
    std::array<unsigned char, Picture::kBytesAPixel> pixel{};
    std::size_t stride = pixel.size();
    int noWait = 0;
    std::array<mpv_render_param, kRenderParameters> parameters{
        mpv_render_param{MPV_RENDER_PARAM_SW_SIZE, size.data()},
        mpv_render_param{MPV_RENDER_PARAM_SW_FORMAT, const_cast<char*>("bgr0")}, // NOLINT
        mpv_render_param{MPV_RENDER_PARAM_SW_STRIDE, &stride},
        mpv_render_param{MPV_RENDER_PARAM_SW_POINTER, pixel.data()},
        mpv_render_param{MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &noWait},
        mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr}};
    (void)mpv_render_context_render(render, parameters.data());
}

/// The next event, waiting up to `kEventTimeoutSeconds` for one — and servicing the render
/// context while it waits. Hands back the `MPV_EVENT_NONE` of the last slice when nothing
/// came.
[[nodiscard]] const mpv_event* nextEvent(mpv_handle* player, mpv_render_context* render) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::duration<double>(kEventTimeoutSeconds);
    const mpv_event* event = nullptr;
    do {
        serviceRender(render);
        event = mpv_wait_event(player, kPumpSeconds);
    } while (event->event_id == MPV_EVENT_NONE && std::chrono::steady_clock::now() < deadline);
    return event;
}

/// Waits for `wanted`, and stops early on an answer that is not it.
///
/// `MPV_EVENT_END_FILE` means the player gave up on the file, and
/// `MPV_EVENT_NONE` that nothing came within the timeout. Both are answers,
/// and the caller reads which one it got from the event it is handed back.
[[nodiscard]] const mpv_event*
waitFor(mpv_handle* player, mpv_render_context* render, mpv_event_id wanted) {
    const mpv_event* event = nullptr;
    for (int seen = 0; seen < kMaxEventsAwaited; ++seen) {
        event = nextEvent(player, render);
        if (event->event_id == wanted || event->event_id == MPV_EVENT_END_FILE ||
            event->event_id == MPV_EVENT_NONE)
            break;
    }
    return event;
}

/// Throws away every event already waiting, so that the next wait is for what
/// happens after it and not for what happened before.
///
/// **Issue #610 found why this exists.** Opening a film ends at `FILE_LOADED`, and
/// the `PLAYBACK_RESTART` that follows it — the first frame being ready — was left in
/// the queue. A `seek` that waited for « a restart » took that one and returned before
/// its own jump was done, and the restart of that jump stayed behind for the next seek:
/// from then on every seek returned one jump early, and the picture on screen was always
/// the previous one. `position()` did not show it, because `time-pos` answers the target
/// at once.
void discardPendingEvents(mpv_handle* player) {
    while (mpv_wait_event(player, 0.0)->event_id != MPV_EVENT_NONE) {
    }
}

/// The loudest the player is ever set to. mpv goes to 130, past what the
/// recording holds; a window offering a slider offers the part that is not
/// distortion.
constexpr int kMaxVolume = 100;

/// Places playback at `target` seconds and waits until it is there.
///
/// `absolute+exact` asks for the frame itself rather than the keyframe before it.
///
/// **It is written rather than relied upon.** Measured: mpv already lands exactly
/// here without it, its `hr-seek` defaulting to precise seeks for absolute positions —
/// so no test can tell the two apart, and none pretends to. What the word buys is
/// that stepping by frames rests on something this file asks for, and not on a
/// default that may be revisited upstream.
void seekTo(mpv_handle* player, mpv_render_context* render, double target) {
    const std::string where = std::to_string(target);
    std::array<const char*, 4> command{"seek", where.c_str(), "absolute+exact", nullptr};
    // Nothing from before this order may be taken for its answer.
    discardPendingEvents(player);
    // The event is waited for and not read: what it says is « playback has
    // resumed », and there is nothing else it could say that a caller of
    // `seek` would act on. A command refused is not waited for at all, which
    // is what keeps a mistaken order from holding the caller five seconds.
    if (mpv_command(player, command.data()) >= 0) [[maybe_unused]]
        const mpv_event* restarted = waitFor(player, render, MPV_EVENT_PLAYBACK_RESTART);
}

/// Reads one entry of libmpv's `track-list` as an audio track, or nothing when the entry
/// is a video or a subtitle track — or is not a map at all.
[[nodiscard]] std::optional<core::AudioTrack> audioTrackOf(const mpv_node& entry) {
    // Written as a bound rather than an early return: an entry that is not a map has no
    // field to read and is therefore not an audio track — one answer, not a mishap.
    const int fields = entry.format == MPV_FORMAT_NODE_MAP ? entry.u.list->num : 0;

    bool audio = false;
    core::AudioTrack track;
    for (int field = 0; field < fields; ++field) {
        const std::string_view key = entry.u.list->keys[field];
        const mpv_node& value = entry.u.list->values[field];
        if (key == "type" && value.format == MPV_FORMAT_STRING)
            audio = std::string_view{value.u.string} == "audio";
        else if (key == "id" && value.format == MPV_FORMAT_INT64)
            track.id = static_cast<int>(value.u.int64);
        else if (key == "lang" && value.format == MPV_FORMAT_STRING)
            track.language = value.u.string;
        else if (key == "title" && value.format == MPV_FORMAT_STRING)
            track.title = value.u.string;
        else if (key == "selected" && value.format == MPV_FORMAT_FLAG)
            track.selected = value.u.flag != 0;
    }
    return audio ? std::optional{std::move(track)} : std::nullopt;
}

/// Lifts the stop `playUntil` set, so that it belongs to that call alone.
void clearStop(mpv_handle* player) {
    mpv_set_property_string(player, "end", "none");
}

/// Reads a property mpv answers with a number, or nothing if it has none.
///
/// Written as one expression rather than as a guard and a return: « the player
/// does not know this one » is an answer of the same rank as the number, not a
/// mishap on the way to it. It is also the only shape a test can walk in
/// whole — nothing makes an open file forget how long it is.
[[nodiscard]] std::optional<double> seconds(mpv_handle* player, const char* name) {
    double value = 0.0;
    const bool known = mpv_get_property(player, name, MPV_FORMAT_DOUBLE, &value) >= 0;
    return known ? std::optional{value} : std::nullopt;
}

[[nodiscard]] std::int64_t millisecondsOf(double value) {
    return std::llround(value * kMillisecondsPerSecond);
}

} // namespace

void MpvPlayer::TerminateAndDestroy::operator()(mpv_handle* player) const noexcept {
    mpv_terminate_destroy(player);
}

void MpvPlayer::FreeRenderContext::operator()(mpv_render_context* context) const noexcept {
    mpv_render_context_free(context);
}

std::string assEventOf(std::string_view line) {
    if (line.empty())
        return {};

    std::string event{kBottomCentre};
    for (const char character : line) {
        switch (character) {
        case '\n':
            // The hard break of ASS. A `\n` is the soft one, which libass
            // honours or not depending on the wrapping mode — and a break the
            // author wrote is not a suggestion.
            event += "\\N";
            break;
        case '\r':
            // Never drawn: a text read from a file with Windows endings would
            // otherwise carry one before every break.
            break;
        default:
            event += character;
            break;
        }
    }

    return event;
}

std::expected<MpvPlayer, core::PlayerError> MpvPlayer::create(Sound sound) {
    // **libmpv refuses to start unless `LC_NUMERIC` is « C », and Qt sets it to
    // the user's.** `QApplication` calls `setlocale(LC_ALL, "")` when it is
    // built, which in a French session makes the decimal mark a comma; libmpv
    // then answers « Non-C locale detected. This is not supported. » and gives
    // no handle at all.
    //
    // Set here rather than left to whoever builds a player, because forgetting
    // it is silent until the first film. It also settles a second thing that
    // was waiting to bite: `std::to_string(double)` follows this same locale,
    // and the position `seek` writes would have gone out as « 1,000000 ».
    //
    // This was found by moving the class here. In `subedit_core` its tests ran
    // in a process with no `QApplication`, hence in the « C » locale, and the
    // defect could not show — while every real window would have met it.
    std::setlocale(LC_NUMERIC, "C");

    Handle handle{mpv_create()};

    // Written as one running answer rather than as a check per call: the three
    // ways of failing here — no memory for a handle, an option this libmpv
    // does not know, an initialisation it refuses — are one and the same event
    // to whoever asked for a player, and none of them can be brought about
    // from a test.
    bool ready = handle != nullptr;
    for (const auto& [name, value] : kEveryPlayer)
        ready = ready && mpv_set_option_string(handle.get(), name, value) >= 0;

    if (sound == Sound::Off)
        ready = ready && mpv_set_option_string(handle.get(), kSilent.first, kSilent.second) >= 0;

    ready = ready && mpv_initialize(handle.get()) >= 0;

    // **The render context is made before any film is loaded** — with `vo=libmpv` the
    // output waits for it — and it is the one thing here that depends on a built handle.
    // `sw` is the software API: pixels in a buffer the caller provides.
    mpv_render_context* context = nullptr;
    if (ready) {
        std::array<mpv_render_param, 2> parameters{
            mpv_render_param{MPV_RENDER_PARAM_API_TYPE,
                             const_cast<char*>(MPV_RENDER_API_TYPE_SW)}, // NOLINT
            mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr}};
        ready = mpv_render_context_create(&context, handle.get(), parameters.data()) >= 0;
    }
    RenderContext render{context};

    if (!ready)
        return std::unexpected(core::PlayerError{.reason = kNotStarted});

    auto notifier = std::make_unique<Notifier>();
    mpv_render_context_set_update_callback(
        render.get(),
        [](void* target) {
            auto* notified = static_cast<Notifier*>(target);
            const std::scoped_lock hold{notified->lock};
            if (notified->notify)
                notified->notify();
        },
        notifier.get());

    return MpvPlayer{std::move(handle), std::move(notifier), std::move(render)};
}

std::expected<void, core::PlayerError> MpvPlayer::open(const std::filesystem::path& video) {
    m_open = false;

    // What mpv calls a load that did not happen, and what this answers unless
    // mpv has something more precise to say. It covers the command being
    // refused — which takes a malformed command, not a bad file — and nothing
    // coming back at all.
    int refusal = MPV_ERROR_LOADING_FAILED;

    const std::string path = video.string();
    std::array<const char*, 3> load{"loadfile", path.c_str(), nullptr};
    if (mpv_command(m_handle.get(), load.data()) >= 0) {
        // **Only the end of the film just asked for is an answer** — issue
        // #468. Loading over an open film first ends that one, and mpv says so
        // with an `END_FILE` of its own; taken for the refusal of the new
        // film, it made every second film, and every return to a tab, fail
        // with « loading failed ». The new film is known by the playlist
        // entry its `START_FILE` names.
        std::optional<std::int64_t> entry;
        bool answered = false;
        for (int seen = 0; seen < kMaxEventsAwaited && !answered; ++seen) {
            const mpv_event* event = nextEvent(m_handle.get(), m_render.get());
            // Nothing within the timeout is an answer as well: no film came.
            answered = event->event_id == MPV_EVENT_NONE;

            if (event->event_id == MPV_EVENT_START_FILE) {
                entry = static_cast<const mpv_event_start_file*>(event->data)->playlist_entry_id;
            } else if (event->event_id == MPV_EVENT_FILE_LOADED && entry.has_value()) {
                m_open = true;
                // **The first frame is part of being open**: what is asked next may be
                // the picture, and until this restart there is none — and left
                // unread, it would be taken by the first `seek` for its own.
                [[maybe_unused]] const mpv_event* first =
                    waitFor(m_handle.get(), m_render.get(), MPV_EVENT_PLAYBACK_RESTART);
                return {};
            } else if (event->event_id == MPV_EVENT_END_FILE) {
                const auto* ended = static_cast<const mpv_event_end_file*>(event->data);
                // **A directory makes mpv answer « success »** — nothing
                // failed, and nothing played either. Reporting that word as the
                // reason a video would not open is how a message stops meaning
                // anything.
                answered = ended->playlist_entry_id == entry;
                if (answered && ended->error < 0)
                    refusal = ended->error;
            }
        }
    }

    return std::unexpected(core::PlayerError{.reason = mpv_error_string(refusal)});
}

std::optional<core::Duration> MpvPlayer::duration() const {
    if (!m_open)
        return std::nullopt;

    return seconds(m_handle.get(), "duration").transform([](double length) {
        return core::Duration::fromMilliseconds(millisecondsOf(length));
    });
}

std::optional<core::Timestamp> MpvPlayer::position() const {
    if (!m_open)
        return std::nullopt;

    return seconds(m_handle.get(), "time-pos").transform([](double where) {
        return core::Timestamp::fromMilliseconds(millisecondsOf(where));
    });
}

void MpvPlayer::onFrameReady(std::function<void()> notify) {
    // Under the lock the callback takes: when this returns, no notification is running
    // and none will start — the widget that asked may be destroyed.
    const std::scoped_lock hold{m_notifier->lock};
    m_notifier->notify = std::move(notify);
}

bool MpvPlayer::render(std::span<unsigned char> pixels, int width, int height, std::size_t stride) {
    constexpr std::size_t kRow = Picture::kBytesAPixel;
    if (!m_open || width <= 0 || height <= 0 || stride < static_cast<std::size_t>(width) * kRow ||
        pixels.size() < stride * static_cast<std::size_t>(height))
        return false;

    // libmpv wants to be told each update has been taken before it raises the next.
    (void)mpv_render_context_update(m_render.get());

    std::array<int, 2> size{width, height};
    int noWait = 0;
    std::size_t rowBytes = stride;
    std::array<mpv_render_param, kRenderParameters> parameters{
        mpv_render_param{MPV_RENDER_PARAM_SW_SIZE, size.data()},
        mpv_render_param{MPV_RENDER_PARAM_SW_FORMAT, const_cast<char*>("bgr0")}, // NOLINT
        mpv_render_param{MPV_RENDER_PARAM_SW_STRIDE, &rowBytes},
        mpv_render_param{MPV_RENDER_PARAM_SW_POINTER, pixels.data()},
        // **Drawn now, not at the time the frame is due**: the window paints when a
        // frame is announced, and a render that waited for its target time would hold
        // the window's own thread.
        mpv_render_param{MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &noWait},
        mpv_render_param{MPV_RENDER_PARAM_INVALID, nullptr}};
    return mpv_render_context_render(m_render.get(), parameters.data()) >= 0;
}

std::optional<Picture> MpvPlayer::picture() const {
    if (!m_open)
        return std::nullopt;

    // `screenshot-raw` answers a node: the size, the row stride, the pixel
    // format and the bytes. It works where there is no screen — measured, issue
    // #610 — which is what lets a test read a picture; `render` is the road the
    // window's own picture takes, and has its cases beside this one's.
    std::array<const char*, 3> command{"screenshot-raw", "video", nullptr};
    mpv_node answer{};
    const bool answered = mpv_command_ret(m_handle.get(), command.data(), &answer) >= 0;

    // A refusal — no picture yet, a film with none — is one answer with the others
    // that are not a picture: nothing.
    std::optional<Picture> picture;
    if (answered && answer.format == MPV_FORMAT_NODE_MAP) {
        int width = 0;
        int height = 0;
        std::size_t stride = 0;
        std::string format;
        const mpv_byte_array* bytes = nullptr;
        for (int at = 0; at < answer.u.list->num; ++at) {
            const std::string_view key = answer.u.list->keys[at];
            const mpv_node& value = answer.u.list->values[at];
            if (key == "w")
                width = static_cast<int>(value.u.int64);
            else if (key == "h")
                height = static_cast<int>(value.u.int64);
            else if (key == "stride")
                stride = static_cast<std::size_t>(value.u.int64);
            else if (key == "format")
                format = value.u.string;
            else if (key == "data")
                bytes = value.u.ba;
        }

        // Only the layout the readers expect: anything else is not guessed at.
        const std::size_t row = static_cast<std::size_t>(width) * Picture::kBytesAPixel;
        if (format == "bgr0" && bytes != nullptr && width > 0 && height > 0 && stride >= row &&
            bytes->size >= stride * static_cast<std::size_t>(height)) {
            Picture found{.width = width, .height = height};
            found.pixels.resize(row * static_cast<std::size_t>(height));
            const auto* source = static_cast<const unsigned char*>(bytes->data);
            for (int y = 0; y < height; ++y)
                std::memcpy(found.pixels.data() + (static_cast<std::size_t>(y) * row),
                            source + (static_cast<std::size_t>(y) * stride),
                            row);
            picture = std::move(found);
        }
    }
    mpv_free_node_contents(&answer);
    return picture;
}

void MpvPlayer::seek(core::Timestamp position) {
    if (!m_open)
        return;

    clearStop(m_handle.get());
    seekTo(m_handle.get(),
           m_render.get(),
           static_cast<double>(position.milliseconds()) / kMillisecondsPerSecond);
}

void MpvPlayer::stepFrames(int frames) {
    if (!m_open || frames == 0)
        return;

    // **A step is a seek by whole frames, and not mpv's `frame-step`.** Measured,
    // issue #614: `frame-step` raises no event when its frame is on screen, so there
    // is nothing to wait for and the position read right after is the one before;
    // `frame-step <n>` is refused by the libmpv of 0.36; and `frame-back-step` is a
    // seek anyway. Written as a seek, a step waits like every other order here.
    //
    // It lands where it should because `time-pos` is the start of the frame on
    // screen — D4 — and a position that is a whole number of frames from it is
    // the start of another frame, however far from the nearest millisecond.
    //
    // **The arithmetic is the core's** — issue #644: a rational one, rounded once, that stops at
    // the last frame, which starts one frame before the end. What is read here is the rate mpv
    // reports, turned into a rate before anything is counted with it.
    const std::optional<double> reported = seconds(m_handle.get(), "container-fps");
    const std::optional<core::FrameRate> rate =
        reported.has_value() ? core::frameRateNear(*reported) : std::nullopt;
    const std::optional<core::Timestamp> here = position();
    const std::optional<core::Duration> length = duration();
    if (!rate.has_value() || !here.has_value() || !length.has_value())
        return;

    pause();
    seek(core::steppedTo(*here, *length, *rate, frames));
}

void MpvPlayer::play() {
    if (!m_open)
        return;

    clearStop(m_handle.get());
    mpv_set_property_string(m_handle.get(), "pause", "no");
}

void MpvPlayer::playUntil(core::Timestamp end) {
    if (!m_open)
        return;

    // Already there, or past: nothing to play. Setting `end` behind the position
    // would let mpv play on to the end of the film before noticing.
    const std::optional<core::Timestamp> here = position();
    if (!here.has_value() || end <= *here)
        return;

    // **mpv's own `end`, which stops on the frame** — measured: with `end` at
    // 1.5 s on a film of 25 images a second, playback holds on the frame that
    // starts at 1.48 s, the last one that starts before it, and `time-pos` stays
    // there. A follower polling every 100 ms would have gone on for up to three
    // frames before it noticed.
    const std::string stop =
        std::to_string(static_cast<double>(end.milliseconds()) / kMillisecondsPerSecond);
    mpv_set_property_string(m_handle.get(), "end", stop.c_str());
    mpv_set_property_string(m_handle.get(), "pause", "no");
}

void MpvPlayer::pause() {
    if (m_open)
        mpv_set_property_string(m_handle.get(), "pause", "yes");
}

void MpvPlayer::showSubtitle(std::string_view line) {
    if (!m_open)
        return;

    const std::string event = assEventOf(line);

    // « none » rather than an empty event: it is how this command is told to
    // draw nothing at all, and an empty `ass-events` leaves the last line
    // where it was.
    std::array<const char*, kOverlayCommandWords> command{"osd-overlay",
                                                          kOverlayId,
                                                          event.empty() ? "none" : "ass-events",
                                                          event.c_str(),
                                                          "0",
                                                          kOverlayHeight,
                                                          "0",
                                                          "no",
                                                          "no",
                                                          nullptr};

    // The answer is dropped, and it is the only place here that does so: there
    // is nothing a window would do about an overlay libmpv would not draw, and
    // no picture to read it back from anyway. What this file can get wrong on
    // its own is `assEventOf`, which is tested out in the open.
    (void)mpv_command(m_handle.get(), command.data());
}

bool MpvPlayer::isPlaying() const {
    // One expression, for the reason `seconds` gives: a player with nothing
    // open is not playing, and neither is one whose answer did not come.
    int paused = 1;
    return m_open && mpv_get_property(m_handle.get(), "pause", MPV_FORMAT_FLAG, &paused) >= 0 &&
           paused == 0;
}

int MpvPlayer::volume() const {
    // Answered with nothing open as well: the volume is the player's, not the film's.
    return static_cast<int>(std::llround(seconds(m_handle.get(), "volume").value_or(0.0)));
}

void MpvPlayer::setHardwareDecoding(bool allowed) {
    // `auto-copy` decodes on the card and copies the picture back, which is what the software
    // render needs (ADR 0041); mpv falls back to the processor by itself when no card answers.
    mpv_set_property_string(m_handle.get(), "hwdec", allowed ? "auto-copy" : "no");
}

void MpvPlayer::setVolume(int volume) {
    // A double, which is what mpv's property is: set from a string it would be
    // read through the locale.
    double level = static_cast<double>(std::clamp(volume, 0, kMaxVolume));
    mpv_set_property(m_handle.get(), "volume", MPV_FORMAT_DOUBLE, &level);
}

std::vector<core::AudioTrack> MpvPlayer::audioTracks() const {
    std::vector<core::AudioTrack> tracks;
    if (!m_open)
        return tracks;

    mpv_node list{};
    if (mpv_get_property(m_handle.get(), "track-list", MPV_FORMAT_NODE, &list) >= 0 &&
        list.format == MPV_FORMAT_NODE_ARRAY) {
        for (int at = 0; at < list.u.list->num; ++at) {
            if (std::optional<core::AudioTrack> track = audioTrackOf(list.u.list->values[at]))
                tracks.push_back(std::move(*track));
        }
    }
    mpv_free_node_contents(&list);
    return tracks;
}

void MpvPlayer::selectAudioTrack(int id) {
    // An identifier the video does not have would make mpv switch the sound off,
    // which is a choice and not a mistake the caller should be able to make by typo.
    const std::vector<core::AudioTrack> tracks = audioTracks();
    const bool known =
        std::ranges::any_of(tracks, [id](const core::AudioTrack& t) { return t.id == id; });
    if (!known)
        return;

    auto chosen = static_cast<std::int64_t>(id);
    mpv_set_property(m_handle.get(), "aid", MPV_FORMAT_INT64, &chosen);
}

} // namespace subedit::gui
