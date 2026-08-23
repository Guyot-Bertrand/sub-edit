#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

namespace subedit::core {

/// Why a video did not open, **in the player's own words**.
///
/// One field and no kind of our own, unlike `FileError` and `LaunchError`.
/// Those carry a kind because their callers act on it — a save that failed for
/// want of permission is not reported as one that failed for want of a
/// directory. Here every refusal leads to the same thing: the window says the
/// video could not be opened, names the file, and stays usable. A taxonomy
/// nobody branches on is a taxonomy nobody keeps right.
///
/// **The file is not named here**, though the reason could carry it: the
/// window has the path it asked about, and naming it twice in one sentence is
/// how a message stops being read.
struct PlayerError {
    /// What the player answered. English, like everything the user reads.
    std::string reason;

    friend bool operator==(const PlayerError&, const PlayerError&) = default;
};

/// A video player, seen from the core.
///
/// One of the five points where this project knows the variation is real —
/// « lecteur vidéo » has been named in the design principles since the
/// foundations. Behind it today: libmpv, decided by
/// [ADR 0020](../../../../../docs/adr/0020-libmpv-pour-le-lecteur-integre.md).
///
/// **It knows nothing of Qt, of libmpv, or of a window** — and that is the
/// whole of what the core keeps. The implementation lives in `subedit_gui`,
/// beside the window it exists for: a player is a thing of the interface, and
/// the domain has no business depending on a media library to reason about
/// one. `check-architecture.sh` has held the Qt half of that line since
/// phase 0; the rest is this file being the only one here that names a player.
///
/// What it exposes is what phase 6 needs and not one thing more. Frame
/// stepping and marking a position belong to phase 14, which is all that is
/// left of it — with one exception, made on purpose: **seeking is exact from
/// the first day.** It is phase 14 that will depend on it, but
/// `seek … absolute+exact` costs no more than an approximate seek, and writing
/// it now makes it something a test can hold years before anything rests on it.
///
/// **One thread.** A player is opened, asked and driven from the thread that
/// built it. Nothing here is guarded, because nothing needs to be: the window
/// drives it from the one thread a window has.
class VideoPlayer {

public:
    virtual ~VideoPlayer() = default;

    /// Loads `video`, or says why it could not be.
    ///
    /// Waits until the player has the file open, so that what follows this
    /// call can ask about it. A file that opens takes milliseconds; one that
    /// does not is refused just as quickly.
    [[nodiscard]] virtual std::expected<void, PlayerError>
    open(const std::filesystem::path& video) = 0;

    /// How long the open video lasts, as its container declares it — nothing
    /// when no video is open.
    ///
    /// The other bound of the timeline, and what phase 6 warns against
    /// crossing. It comes from here and not from `ffprobe` — the player knows
    /// it already, and two sources for one answer would be one too many (D7).
    [[nodiscard]] virtual std::optional<Duration> duration() const = 0;

    /// Where playback stands — nothing when no video is open.
    [[nodiscard]] virtual std::optional<Timestamp> position() const = 0;

    /// Places playback at `position`, **exactly**, and waits until it is there.
    ///
    /// Exactly means the frame asked for, not the keyframe before it: what
    /// a subtitle is checked against is the picture at its own start.
    ///
    /// It waits for the same reason `open` does — the core has no event loop
    /// of its own, and a caller told to ask again later would have to grow one.
    ///
    /// Does nothing when no video is open. Selecting a line before choosing a
    /// film is an ordinary thing to do, not a mistake to report.
    virtual void seek(Timestamp position) = 0;

    /// Starts playback. Does nothing when no video is open.
    virtual void play() = 0;

    /// Holds playback where it is. Does nothing when no video is open.
    virtual void pause() = 0;

    /// Whether the video is playing right now.
    ///
    /// Here so that one thing knows: a window keeping its own idea of it
    /// beside the player's would have two, and they would part company the
    /// first time playback stopped on its own at the end of the film.
    [[nodiscard]] virtual bool isPlaying() const = 0;

protected:
    VideoPlayer() = default;
    VideoPlayer(const VideoPlayer&) = default;
    VideoPlayer(VideoPlayer&&) = default;
    VideoPlayer& operator=(const VideoPlayer&) = default;
    VideoPlayer& operator=(VideoPlayer&&) = default;
};

} // namespace subedit::core
