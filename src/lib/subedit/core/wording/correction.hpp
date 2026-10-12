#pragma once

// What the tool says of a correction that could not be played in full: a pattern
// that gave up, a pattern file that did not read. Shared wording of
// `core/wording/`; see `formats.hpp` for why it lives in the core.
//
// The window says them in the confirmation of the assistant and the command line
// on its error stream, **in these words** — the second half of the sentence the
// surface builds around them.
//
// And how a pattern is named when it is shown (spec 15, D6; issue #666).

#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

/// A pattern's name as it is shown: the installed catalogue's translation of the
/// English name its file holds, or that name as written when the catalogue does
/// not know it — a user's own pattern. The files are Gaupol's and are never
/// rewritten (ADR 0037); `src/scripts/update-pot.sh` puts the names of the
/// shipped ones in the template.
///
/// **For a reader only.** The English name stays the identifier: a setting, the
/// name `--enable` takes and every value of `--format json` keep it.
[[nodiscard]] std::string_view patternName(std::string_view englishName);
[[nodiscard]] std::string_view patternName(const CorrectionPattern& pattern);

/// A pattern's description as it is shown, by the same rule as its name.
[[nodiscard]] std::string_view patternDescription(const CorrectionPattern& pattern);

/// Why a pattern was not applied: « will not compile », « timed out »…
[[nodiscard]] std::string_view reasonOf(FailureKind kind);

/// Why a record or a file of patterns was not read: « malformed line »…
[[nodiscard]] std::string_view reasonOf(PatternProblem problem);

/// What a reading of the patterns ran into, in one phrase: the file by its own
/// name, the line when there is one, and the reason, with the detail the
/// reading gave when it gave one — `Zyyy.common-error, line 4 (malformed line:
/// detail)`.
[[nodiscard]] std::string describe(const PatternDiagnostic& diagnostic);

} // namespace subedit::core
