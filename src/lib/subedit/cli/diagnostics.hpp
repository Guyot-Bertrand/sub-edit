#pragma once

// What a reader ran into, told to whoever asked for that much detail.

#include <subedit/core/format/diagnostic.hpp>

#include <span>
#include <string>
#include <string_view>

namespace subedit::cli {

class Reporter;

/// The level at which a read's diagnostics are named one by one.
///
/// The most detailed of the four. They are informative and never a failure —
/// the file was read, at best effort, and the operation went on — so putting
/// them lower would bury the line that says what actually happened under a
/// list nobody asked for.
inline constexpr int kDiagnosticLevel = 3;

/// One diagnostic, as a line of narration: where, what, and what was done.
///
/// ```
/// line 5: a timing line that could not be read ("00:00:01 -> 3"), left as it stands
/// ```
///
/// The detail comes from the file and is therefore **quoted and bounded**:
/// unquoted, a line ending in a comma would read as part of the sentence, and
/// unbounded, a single absurd line would push the rest of the narration off the
/// screen. Neither is ours to trust.
[[nodiscard]] std::string reportOf(const subedit::core::Diagnostic& diagnostic);

/// Says every diagnostic of a read, each on its own line, prefixed by `path`.
///
/// Shared by the five subcommands rather than written where each one reads a
/// file: ADR 0008 has the core read at best effort and report what it had to
/// decide, and a promise honoured by one caller out of five is not honoured.
void sayDiagnostics(const Reporter& reporter,
                    std::string_view path,
                    std::span<const subedit::core::Diagnostic> diagnostics);

} // namespace subedit::cli
