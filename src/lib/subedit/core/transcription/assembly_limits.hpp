#pragma once

#include <cstddef>
#include <cstdint>

namespace subedit::core {

/// The defaults of the post-processing's limits, starting points to be tuned on
/// the output of a model of real size. In a header of their own because both the
/// post-processing and the settings need them, and the settings are included
/// nearly everywhere.
inline constexpr std::int64_t kDefaultMergeGapMilliseconds = 400;
inline constexpr std::int64_t kDefaultMaxDurationMilliseconds = 7000;
inline constexpr std::size_t kDefaultMaxCharacters = 84;

/// The bounds a setting of the post-processing is held to, wherever it is typed: a second to an
/// hour, and a thousand characters.
inline constexpr int kLongestTranscribedSubtitleMilliseconds = 3'600'000;
inline constexpr int kMostTranscribedCharacters = 1000;

/// What is too small to stand as a subtitle of its own when a piece is cut: not settings, but
/// the care the cut takes (a one-word subtitle of a fifth of a second is not readable).
inline constexpr std::int64_t kMinSubtitleDurationMilliseconds = 700;
inline constexpr std::size_t kMinSubtitleCharacters = 12;

} // namespace subedit::core
