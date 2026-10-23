#pragma once

#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace subedit::core {

/// The defaults of `AssemblySettings`, starting points tuned on the output of a
/// model of real size.
inline constexpr std::int64_t kDefaultMergeGapMilliseconds = 400;
inline constexpr std::int64_t kDefaultMaxDurationMilliseconds = 7000;
inline constexpr std::size_t kDefaultMaxCharacters = 84;

/// The limits the post-processing holds a subtitle to. The defaults are starting
/// points, to be tuned on the output of a model of real size; they are settings
/// (#777), never constants of the engine.
struct AssemblySettings {
    /// Two segments closer than this, and not separated by the end of a
    /// sentence, are one subtitle.
    Duration maxGap = Duration::fromMilliseconds(kDefaultMergeGapMilliseconds);

    /// The longest a subtitle stays on screen.
    Duration maxDuration = Duration::fromMilliseconds(kDefaultMaxDurationMilliseconds);

    /// The most characters one subtitle holds, line breaks not counted: two
    /// lines of 42. Cutting that text into lines is the line-breaking task's
    /// job, which already exists.
    std::size_t maxCharacters = kDefaultMaxCharacters;
};

/// Turns what an engine cut into subtitles.
///
/// An engine cuts where it likes. This step, **independent of any engine**,
/// does two things (D7 of the phase 22 spec):
///
/// - it **merges** a segment into the one before when they are close, when the
///   first does not end a sentence, when they have the same speaker (or none),
///   and when the result still respects the limits;
/// - it **splits** a segment that exceeds the limits, at a word boundary — at the
///   timed words when the engine gave them, else in proportion to the letters.
///
/// It works with no speaker and with no timed words, and a word longer than the
/// limit is kept whole rather than cut. The result carries the main text only;
/// positions are those of the engine, untouched.
[[nodiscard]] std::vector<Subtitle> assembleSubtitles(std::span<const TranscribedSegment> segments,
                                                      const AssemblySettings& settings = {});

} // namespace subedit::core
