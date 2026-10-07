#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

/// One audio track of a video, as the file declares it.
///
/// **The identifier is the file's own and means nothing outside it** — the
/// first track of one film is not the first of another, which is why a choice
/// of track is not carried from one video to the next. Language and title are
/// whatever the container wrote, empty when it wrote nothing: the menu shows
/// what there is and never invents a label.
struct AudioTrack {
    /// What `VideoPlayer::selectAudioTrack` takes.
    int id = 0;

    /// An ISO 639 code as the file spells it (`fra`, `eng`), or empty.
    std::string language{};

    std::string title{};

    /// Whether this is the track playing now.
    bool selected = false;

    friend bool operator==(const AudioTrack&, const AudioTrack&) = default;
};

/// A video player, seen from the core.
///
/// One of the five points where this project knows the variation is real — the
/// video player has been named in the design principles since the foundations.
/// Behind it today: libmpv, decided by ADR 0020.
///
/// **It knows nothing of Qt, of libmpv, or of a window** — and that is the
/// whole of what the core keeps. The implementation lives in `subedit_gui`,
/// beside the window it exists for: a player is a thing of the interface, and
/// the domain has no business depending on a media library to reason about
/// one. `check-architecture.sh` has held the Qt half of that line since
/// phase 0; the rest is this file being the only one here that names a player.
///
/// What it exposes is what the window needs and not one thing more: opening,
/// placing, playing, the replica of a subtitle — and, with phase 14, stepping
/// by frames, the volume, the audio tracks, and playing up to a position.
/// **Seeking was exact from the first day**, ahead of what depended on it:
/// `seek … absolute+exact` costs no more than an approximate seek, and writing
/// it early made it something a test could hold before anything rested on it.
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
    ///
    /// **The start of the picture on screen**, to the millisecond, and not what
    /// was last asked for: after `seek(437)` on the video above, 459. A mark set from
    /// this position is therefore always the start of a frame, and seeking back to it
    /// shows the same frame.
    [[nodiscard]] virtual std::optional<Timestamp> position() const = 0;

    /// Places playback at `position`, **exactly**, and waits until it is there.
    ///
    /// Exactly means a frame and not a keyframe: the player decodes from the
    /// keyframe before it, so that what is on screen is a picture of the film and
    /// not the nearest place the file could be entered.
    ///
    /// **And the frame is the nearest one to `position`**, which is not always
    /// the frame on screen at that instant — measured on a 23.976 video, issue #610:
    /// the first half of the interval a frame occupies lands on that frame, the
    /// second half on the next. A position asked at 437 ms, inside the frame that
    /// spans 417.08 to 458.79 ms, shows the frame that starts at 458.79 ms, and
    /// `position()` then answers 459. Whole milliseconds are not the cause: every
    /// frame of that video lands on itself when asked at the millisecond it starts.
    ///
    /// It waits for the same reason `open` does — the core has no event loop
    /// of its own, and a caller told to ask again later would have to grow one.
    ///
    /// Does nothing when no video is open. Selecting a line before choosing a
    /// film is an ordinary thing to do, not a mistake to report.
    virtual void seek(Timestamp position) = 0;

    /// Moves playback by `frames` pictures — forward when positive, back when
    /// negative — and waits until the picture is there. Zero does nothing.
    ///
    /// **It stops at the ends rather than going past them**: stepping back from
    /// the first frame stays on it, and stepping forward from the last stays
    /// on the last. A step is a gesture repeated by a held key, and one that
    /// ran off the film would end it. Playback is held afterwards, like every
    /// step of a player — a caller that was playing is not playing any more.
    ///
    /// Does nothing when no video is open.
    virtual void stepFrames(int frames) = 0;

    /// Starts playback, and lets it run to the end of the film.
    /// Does nothing when no video is open.
    virtual void play() = 0;

    /// Starts playback and holds it again at `end`, **to the frame** — a
    /// follower that stopped playback every 100 ms would let it run on by up to
    /// three frames. Does not wait: playback runs by itself, and the caller
    /// reads `isPlaying()` and `position()` as it does for `play()`.
    ///
    /// The stop belongs to this call alone: a later `play()` runs on past
    /// `end`. An `end` that playback has already reached plays nothing.
    ///
    /// Does nothing when no video is open.
    virtual void playUntil(Timestamp end) = 0;

    /// Holds playback where it is. Does nothing when no video is open.
    virtual void pause() = 0;

    /// Draws `line` over the picture, or clears it when `line` is empty.
    ///
    /// **Decision D2, and the reason this is a method rather than a file.**
    /// A player knows how to load a subtitle file, and for an editor that
    /// would be the wrong road: the file would have to be written again at
    /// every keystroke. What is drawn comes from the same `Project` the table
    /// shows, so what is on the picture is what was just typed.
    ///
    /// **The text is in the Sub Station Alpha vocabulary** — issue #408 —: what
    /// `replicaOf` writes from the subtitle's own text, its tags understood
    /// (ADR 0009 keeps the model's text raw; ADR 0031 is the pivot). Italics, bold,
    /// underline and colour are override blocks, the braces of the visible text
    /// are escaped, and what has no equivalent on screen is already gone.
    ///
    /// Does nothing when no video is open, like every other order here.
    virtual void showSubtitle(std::string_view line) = 0;

    /// Whether the video is playing right now.
    ///
    /// Here so that one thing knows: a window keeping its own idea of it
    /// beside the player's would have two, and they would part company the
    /// first time playback stopped on its own at the end of the film.
    [[nodiscard]] virtual bool isPlaying() const = 0;

    /// The volume, from 0 (silence) to 100. The player's own, not the
    /// system's — and still answered with nothing open, since it is the
    /// player's and not the film's.
    [[nodiscard]] virtual int volume() const = 0;

    /// Sets the volume. A value outside 0 to 100 is brought back to the
    /// nearest bound, so that a caller adding a step never has to clamp first.
    virtual void setVolume(int volume) = 0;

    /// The audio tracks of the open video, in the file's order — none when no
    /// video is open, and none for a video without sound. **Neither is an
    /// error**: the menu shows an empty list.
    [[nodiscard]] virtual std::vector<AudioTrack> audioTracks() const = 0;

    /// Plays the track `id` of `audioTracks()`. An identifier the video does
    /// not have changes nothing.
    virtual void selectAudioTrack(int id) = 0;

protected:
    VideoPlayer() = default;
    VideoPlayer(const VideoPlayer&) = default;
    VideoPlayer(VideoPlayer&&) = default;
    VideoPlayer& operator=(const VideoPlayer&) = default;
    VideoPlayer& operator=(VideoPlayer&&) = default;
};

} // namespace subedit::core
