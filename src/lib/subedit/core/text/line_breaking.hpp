#pragma once

// Breaking lines to fit a length and a line count, as Gaupol's `Liner` does it
// — decision D5 of the spec of phase 12, issue #502.
//
// A function of texts, like `common_errors.cpp`: it takes the patterns whose
// penalties steer the choice of a break, the measure to break by, and the
// texts, and gives back one text per input, broken where breaking helps.

#include <subedit/core/text/common_errors.hpp> // PatternFailure, FailureKind
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

struct BrokenTexts {
    /// One per text given, in the same order, unbroken where breaking did not
    /// apply — skipped, or nothing found worth cutting.
    std::vector<std::string> texts;
    std::vector<PatternFailure> failures;
};

/// The thresholds of Gaupol's skip gate — `max_skip_length` and
/// `max_skip_lines`, each independent of the limits being broken to. A
/// threshold left at its default never holds a text back: Gaupol's own 32768
/// for a check the user turned off.
struct SkipLimits {
    double maxLength = std::numeric_limits<double>::infinity();
    int maxLines = std::numeric_limits<int>::max();
};

/// Breaks each of `texts` to `maxLength` (in `measure`'s unit) and `maxLines`
/// lines, the patterns of kind `LineBreak` placing the penalties that steer
/// which space a break lands on.
///
/// **The patterns are applied once per text**, to place their penalties — not
/// once per candidate break, the way a naive search would. A pattern that
/// cannot be applied to a text is named and contributes no penalty for that
/// text; the others still do.
///
/// **`maxLines` may be exceeded** when no combination of breaks up to it keeps
/// every line within `maxLength` — up to ten lines are tried before a text is
/// given up on and returned whole. **Nothing is cut below `maxLines`**: a text
/// short enough in boxes to never reach it is returned as it was, however long
/// its one line runs.
///
/// **`skip` is Gaupol's own gate**, for a caller breaking a whole document: a
/// text already within `skip->maxLength` and `skip->maxLines` is left alone,
/// and so is one breaking would not bring closer to either threshold it
/// exceeds — the length down, the line count down, or both. The thresholds
/// are the caller's, not `maxLength` and `maxLines`: Gaupol's defaults happen
/// to make them equal. Absent, the default, every text is broken and returned
/// broken, whether that changed anything or not.
[[nodiscard]] BrokenTexts breakLines(const PatternEngine& engine,
                                     std::span<const CorrectionPattern* const> patterns,
                                     std::span<const std::string> texts,
                                     const LineMeasure& measure,
                                     double maxLength,
                                     int maxLines,
                                     std::optional<SkipLimits> skip = std::nullopt);

} // namespace subedit::core
