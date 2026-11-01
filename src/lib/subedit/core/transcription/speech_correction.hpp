#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/alignment_retiming.hpp>
#include <subedit/core/transcription/speech_detector.hpp>

#include <cstdint>
#include <span>
#include <vector>

namespace subedit::core {

/// How far the file has to move, `shiftMilliseconds`, around `at` — a time of the file once its
/// frame rate has been put right.
struct SpeechShift {
    Timestamp at;
    std::int64_t shiftMilliseconds = 0;

    friend bool operator==(const SpeechShift&, const SpeechShift&) = default;
};

/// What the sound says about a subtitle file as a whole (phase 23, #820): a frame rate to put
/// right, then a shift that may change slowly or by a step along the film.
///
/// **It says nothing about a single line**: it is found from when somebody speaks and when a
/// subtitle is shown, never from what is said, so it holds for a text in any language — and it
/// cannot tell that one line is out of place in a file that is otherwise right.
struct SpeechCorrection {
    /// The times of the file are divided by this before the shifts apply: 25 / 23.976 for a file
    /// timed for 25 frames per second on a film that runs at 23.976.
    double ratio = 1.0;

    /// In order of time. Empty when nothing could be found, and then nothing is corrected.
    std::vector<SpeechShift> shifts;

    /// How well the corrected subtitles lie on the speech, from 0 to 1: the median, along the
    /// film, of the correlation of the two. Low when the sound and the file have little to do with
    /// one another (the wrong film, a film with almost no speech).
    double agreement = 0.0;

    [[nodiscard]] bool fitted() const { return !shifts.empty(); }

    /// Where a time of the file goes.
    [[nodiscard]] Timestamp corrected(Timestamp time) const;
};

/// Finds the correction that lays `lines` on `speech`.
///
/// The frame rate is chosen among the usual ones (any two of 23.976, 24, 25, 29.97 and 30), and is
/// changed from 1 only when it fits clearly better. The shift is then found for each stretch of two
/// minutes along the film, and the path through those shifts is the one that agrees with the speech
/// and changes least: a constant offset, a drift, and a second half that is late are all paths.
[[nodiscard]] SpeechCorrection estimateSpeechCorrection(std::span<const SpeechSpan> speech,
                                                        std::span<const LineTimes> lines);

} // namespace subedit::core
