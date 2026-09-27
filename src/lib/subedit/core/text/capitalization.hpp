#pragma once

// Capitalizing texts as Gaupol's `capitalize` does it — decision D7 of the
// spec of phase 12.
//
// A stateful walk of texts already in order: whether one text ends a
// sentence decides whether the next opens with a capital, so this function
// receives consecutive subtitles, the first of them read as the very first
// of the document — the one Gaupol capitalizes whatever it holds. Where a
// selection has a gap is its caller's to know, not this function's: it walks
// only the run it is given, start to end.

#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <span>
#include <string>

namespace subedit::core {

/// Capitalizes `texts`, `texts.front()` read as the very first subtitle of
/// the document — always capitalized, like every text a previous one leaves
/// wanting a capital next.
///
/// Only the patterns of kind `Capitalization` are applied; the others are not
/// this function's, the same rule `correctCommonErrors` follows.
///
/// **A pattern that cannot be applied to a text leaves it as it was before
/// it**, the same rule `correctCommonErrors` follows, and is reported.
[[nodiscard]] CorrectedTexts
correctCapitalization(const PatternEngine& engine,
                      std::span<const CorrectionPattern* const> patterns,
                      std::span<const std::string> texts);

} // namespace subedit::core
