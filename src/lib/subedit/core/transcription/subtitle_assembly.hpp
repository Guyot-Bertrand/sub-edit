#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/transcription/assembly_limits.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace subedit::core {

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

    /// A cut that leaves less than this (in time or in characters) is avoided when another
    /// exists.
    Duration minDuration = Duration::fromMilliseconds(kMinSubtitleDurationMilliseconds);
    std::size_t minCharacters = kMinSubtitleCharacters;
};

/// The limits the post-processing is held to, from the settings.
[[nodiscard]] AssemblySettings assemblyOf(const TranscriptionSettings& settings);

/// What the post-processing made, and where each subtitle comes from.
struct Assembly {
    std::vector<Subtitle> subtitles;

    /// For each subtitle, the indices (in the segments given) of the pieces it was made from, in
    /// order. A subtitle may come from several pieces, and a piece cut in several subtitles is the
    /// source of each of them. A piece dropped (see `assemble`) is the source of none.
    std::vector<std::vector<std::size_t>> sources;
};

/// Turns what an engine cut into subtitles, and says where each one comes from.
///
/// It first **drops the pieces that contain no letter and no digit** — `...`, `…`, `♪`, `-`: what
/// an engine writes for a passage with no speech when it is forbidden to write notes. They would
/// be subtitles made of nothing, and would stop their neighbours being merged.
///
/// The steps are those of `assembleSubtitles`, whose result is the `subtitles` of this one.
[[nodiscard]] Assembly assemble(std::span<const TranscribedSegment> segments,
                                const AssemblySettings& settings = {});

/// Turns what an engine cut into subtitles.
///
/// An engine cuts where it likes. This step, **independent of any engine**, does three things, in
/// this order and each over the whole set before the next (D7 of the phase 22 spec):
///
/// - it **drops** the pieces with no letter and no digit;
/// - it **merges** a segment into the one before when they are close, when the first does not end a
///   sentence, when they have the same speaker (or none), and when the result still respects the
///   limits;
/// - it **cuts** a segment that exceeds the limits, **at punctuation first** (an end of sentence,
///   then a clause) and in the middle of a sentence only when there is none, balancing the halves
///   and avoiding one too small — at the timed words when the engine gave them, else in proportion
///   to the letters.
///
/// It works with no speaker and with no timed words, and a word longer than the limit is kept
/// whole rather than cut. The result carries the main text only; **a subtitle that begins a piece
/// keeps its start and one that ends it keeps its end**.
[[nodiscard]] std::vector<Subtitle> assembleSubtitles(std::span<const TranscribedSegment> segments,
                                                      const AssemblySettings& settings = {});

} // namespace subedit::core
