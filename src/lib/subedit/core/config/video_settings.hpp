#pragma once

// What the video player retains — issue #615, Gaupol's `video_player.seek_length`,
// `video_player.context_length` and `video_player.volume`, and the frame step of issue #618.

namespace subedit::core {

/// The bounds of the jump of `Seek Backward` and `Seek Forward`, in seconds.
///
/// At least one second: a jump of zero is a gesture that does nothing, and a
/// value that large in a file is a value somebody mistyped.
inline constexpr int kSmallestSeekLengthSeconds = 1;
inline constexpr int kLargestSeekLengthSeconds = 3600;

/// The bounds of the lead-in, in milliseconds. Zero is a lead-in: no context.
inline constexpr int kLargestContextLengthMilliseconds = 60000;

/// Gaupol's defaults: a jump of thirty seconds and a lead-in of one.
inline constexpr int kDefaultSeekLengthSeconds = 30;
inline constexpr int kDefaultContextLengthMilliseconds = 1000;

/// The bounds of the frame step, in **frames** and never in milliseconds — issue #618.
///
/// One at least: it is the picture-by-picture, and a step of none does nothing. A thousand at most,
/// some forty seconds of film at the usual rates: far past any jump somebody sets in frames, and a
/// value in a file beyond it is one that was mistyped.
inline constexpr int kSmallestStepFrames = 1;
inline constexpr int kLargestStepFrames = 1000;

/// The bounds of the volume, in per cent.
inline constexpr int kLargestVolume = 100;

/// How the player is driven, kept from one session to the next.
///
/// **The three are Gaupol's, with Gaupol's defaults**: the jump is thirty
/// seconds, the lead-in one second, and the volume is the player's own until
/// somebody moves it. The audio track is not here, and that is on purpose — a
/// track number belongs to its file, and carrying it to the next film would
/// pick whatever happens to have that number there.
struct VideoSettings {
    /// How far `Seek Backward` and `Seek Forward` go, in seconds.
    int seekLengthSeconds = kDefaultSeekLengthSeconds;

    /// How much of the film before a selection `Seek Selection Start` and
    /// `Seek Selection End` show, in milliseconds — Gaupol's `context_length`.
    int contextLengthMilliseconds = kDefaultContextLengthMilliseconds;

    /// How many frames `Step Backward` and `Step Forward` move playback, and how many a nudge of
    /// an edge moves it — **the same step for both**. A count of frames, so that a step of N is
    /// always N pictures, whatever the frame rate of the film.
    int stepFrames = kSmallestStepFrames;

    /// The volume, 0 to 100.
    int volume = kLargestVolume;

    friend bool operator==(const VideoSettings&, const VideoSettings&) = default;
};

} // namespace subedit::core
