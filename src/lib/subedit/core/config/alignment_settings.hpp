#pragma once

#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/transcript_matching_aligner.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <cstdint>
#include <string>

namespace subedit::core {

inline constexpr int kDefaultMinimumMatchPercent = 50;
inline constexpr int kMostMinimumMatchPercent = 100;
inline constexpr double kPercent = 100.0;

/// What the automatic alignment retains — phase 23, D7 of its spec.
///
/// The model, the voice-activity model and the engine that **listens** are those of
/// `TranscriptionSettings`: there is no second set. These are the choices of the alignment itself.
/// They are read from the settings file where the first surface that shows them is written (the
/// `sync` command, #813); until then they are the defaults.
struct AlignmentSettings {
    /// The alignment engine, as `makeAlignmentEngine` knows it.
    std::string engine{kDefaultAlignmentEngine};

    /// The share of a line's words that must be heard for the line to be found, in percent.
    int minimumMatchPercent = kDefaultMinimumMatchPercent;

    /// How far a shift may depart from its neighbours', in milliseconds.
    std::int64_t doubtGapMilliseconds = kDefaultDoubtGapMilliseconds;

    /// The same share, as the matching takes it.
    [[nodiscard]] double minimumMatch() const { return minimumMatchPercent / kPercent; }

    friend bool operator==(const AlignmentSettings&, const AlignmentSettings&) = default;
};

} // namespace subedit::core
