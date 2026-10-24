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

} // namespace subedit::core
