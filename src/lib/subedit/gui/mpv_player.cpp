#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <mpv/client.h>

#include <array>
#include <clocale>
#include <cmath>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// What a player is built with, and the whole of it.
///
/// **`vo=null` is what makes this work without a screen, and it was measured
/// rather than taken on promise.** With `vo=auto` and no display, the very
/// same sequence loads nothing: mpv answers `end-file`, and the duration comes
/// back « property unavailable ». Which is what the tests need, and what the
/// continuous integration needs — and it is what #176 will replace with a
/// window to draw into.
///
/// The other four are not decoration either. `config=no` keeps a developer's
/// own `~/.config/mpv` from deciding whether the gate passes; `terminal=no`
/// keeps mpv's chatter off our output; `ao=null` because a runner has no sound
/// device; `pause=yes` because opening a film is not watching it.
constexpr std::array<std::pair<const char*, const char*>, 5> kOptions{{
    {"config", "no"},
    {"terminal", "no"},
    {"vo", "null"},
    {"ao", "null"},
    {"pause", "yes"},
}};

/// How long one wait for an event may take.
///
/// Generous on purpose: this is not a budget but a way out of a wait that
/// would otherwise hold the caller forever. Opening a file takes milliseconds.
constexpr double kEventTimeoutSeconds = 5.0;

/// How many events may go by before a wait gives up on the one it wants.
constexpr int kMaxEventsAwaited = 100;

constexpr double kMillisecondsPerSecond = 1000.0;

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

std::expected<MpvPlayer, core::PlayerError> MpvPlayer::create() {
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
    for (const auto& [name, value] : kOptions)
        ready = ready && mpv_set_option_string(handle.get(), name, value) >= 0;
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

bool MpvPlayer::isPlaying() const {
    // One expression, for the reason `seconds` gives: a player with nothing
    // open is not playing, and neither is one whose answer did not come.
    int paused = 1;
    return m_open && mpv_get_property(m_handle.get(), "pause", MPV_FORMAT_FLAG, &paused) >= 0 &&
           paused == 0;
}

} // namespace subedit::gui
