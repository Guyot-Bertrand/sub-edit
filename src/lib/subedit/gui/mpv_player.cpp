#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <mpv/client.h>

#include <array>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::gui {

namespace {

/// What every player is built with, whatever it draws into.
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
constexpr std::array<std::pair<const char*, const char*>, 4> kEveryPlayer{{
    {"config", "no"},
    {"terminal", "no"},
    {"pause", "yes"},
    {"sub-auto", "no"},
}};

/// What a player with nowhere to draw is built with — the shape of every test.
///
/// **`vo=null` is what makes this work without a screen, and it was measured
/// rather than taken on promise.** With `vo=auto` and no display, the very
/// same sequence loads nothing: mpv answers `end-file`, and the duration comes
/// back « property unavailable ». `ao=null` follows it for the same reason a
/// runner has no sound device — and a player nobody can see is not one anybody
/// should hear.
///
/// A player that *is* given a window is left mpv's own defaults for both.
/// Checking that a subtitle lands on the right line is done as much by ear as
/// by eye, and a `subedit` that played films silently would have made that
/// harder for the sake of one shared constant.
constexpr std::array<std::pair<const char*, const char*>, 2> kNowhereToDraw{{
    {"vo", "null"},
    {"ao", "null"},
}};

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

/// Which of libmpv's GPU contexts draws into a window we hand over.
///
/// EGL on X11, and named rather than probed — see where it is set for what
/// probing did instead.
constexpr const char* kX11Context = "x11egl";

/// What a player that could not be built answers.
constexpr const char* kNotStarted = "the video player could not be started";

/// Waits for `wanted`, and stops early on an answer that is not it.
///
/// `MPV_EVENT_END_FILE` means the player gave up on the file, and
/// `MPV_EVENT_NONE` that nothing came within the timeout. Both are answers,
/// and the caller reads which one it got from the event it is handed back.
[[nodiscard]] const mpv_event* waitFor(mpv_handle* player, mpv_event_id wanted) {
    const mpv_event* event = nullptr;
    for (int seen = 0; seen < kMaxEventsAwaited; ++seen) {
        event = mpv_wait_event(player, kEventTimeoutSeconds);
        if (event->event_id == wanted || event->event_id == MPV_EVENT_END_FILE ||
            event->event_id == MPV_EVENT_NONE)
            break;
    }
    return event;
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
        case '{':
        case '}':
            // What would open and close an override block. Escaped, so that a
            // subtitle saying « {laughs} » says it rather than disappearing
            // into a tag libass does not recognise.
            event += '\\';
            event += character;
            break;
        default:
            event += character;
            break;
        }
    }

    return event;
}

std::expected<MpvPlayer, core::PlayerError> MpvPlayer::create(std::uintptr_t window) {
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

    if (window == 0) {
        for (const auto& [name, value] : kNowhereToDraw)
            ready = ready && mpv_set_option_string(handle.get(), name, value) >= 0;
    } else {
        // Set before `mpv_initialize` because that is the only moment libmpv
        // reads it — measured, and the reason a player belongs to one surface
        // for its whole life.
        ready = ready &&
                mpv_set_option_string(handle.get(), "wid", std::to_string(window).c_str()) >= 0;

        // **And the context is named rather than probed, which was a defect
        // before it was a line.** Adopting a native window is an X11
        // mechanism; left to choose, mpv picks by what the session offers, and
        // on a machine where `WAYLAND_DISPLAY` is set it picks Wayland — where
        // `wid` means nothing. Measured, with the window handed over and the
        // context left free: mpv opened **a window of its own**, beside ours,
        // and the picture appeared everywhere except where it had been asked
        // for. Named, the very same run draws inside our window.
        //
        // Which is also why `mpvPlayers` hands a window over on the `xcb`
        // platform and on no other: a number that is not an X window would
        // send this straight into an X error.
        ready = ready && mpv_set_option_string(handle.get(), "gpu-context", kX11Context) >= 0;
    }

    ready = ready && mpv_initialize(handle.get()) >= 0;

    if (!ready)
        return std::unexpected(core::PlayerError{.reason = kNotStarted});

    return MpvPlayer{std::move(handle)};
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
        const mpv_event* event = waitFor(m_handle.get(), MPV_EVENT_FILE_LOADED);
        if (event->event_id == MPV_EVENT_FILE_LOADED) {
            m_open = true;
            return {};
        }

        if (event->event_id == MPV_EVENT_END_FILE) {
            // **A directory makes mpv answer « success »** — nothing failed,
            // and nothing played either. Reporting that word as the reason a
            // video would not open is how a message stops meaning anything.
            const int reported = static_cast<const mpv_event_end_file*>(event->data)->error;
            if (reported < 0)
                refusal = reported;
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

void MpvPlayer::seek(core::Timestamp position) {
    if (!m_open)
        return;

    const std::string target =
        std::to_string(static_cast<double>(position.milliseconds()) / kMillisecondsPerSecond);

    // `absolute+exact` asks for the frame itself rather than the keyframe
    // before it.
    //
    // **It is written rather than relied upon.** Measured: mpv already lands
    // exactly here without it, its `hr-seek` defaulting to precise seeks for
    // absolute positions — so no test can tell the two apart, and none
    // pretends to. What the word buys is that phase 14 rests on something this
    // file asks for, and not on a default that may be revisited upstream.
    std::array<const char*, 4> command{"seek", target.c_str(), "absolute+exact", nullptr};
    // The event is waited for and not read: what it says is « playback has
    // resumed », and there is nothing else it could say that a caller of
    // `seek` would act on. A command refused is not waited for at all, which
    // is what keeps a mistaken order from holding the caller five seconds.
    if (mpv_command(m_handle.get(), command.data()) >= 0) [[maybe_unused]]
        const mpv_event* restarted = waitFor(m_handle.get(), MPV_EVENT_PLAYBACK_RESTART);
}

void MpvPlayer::play() {
    if (m_open)
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

} // namespace subedit::gui
