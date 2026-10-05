#pragma once

// What the tool says of a correction that could not be played in full: a pattern
// that gave up, a pattern file that did not read. Shared wording of
// `core/wording/`; see `formats.hpp` for why it lives in the core.
//
// The window says them in the confirmation of the assistant and the command line
// on its error stream, **in these words** — the second half of the sentence the
// surface builds around them.

#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

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
