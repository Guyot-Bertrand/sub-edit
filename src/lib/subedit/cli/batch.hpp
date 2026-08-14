#pragma once

// Turning a run over several files into a summary and an exit code.

#include <subedit/cli/exit_code.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::cli {

class Reporter;

/// How a run over `total` files ended, `done` of them having succeeded.
///
/// `AllFailed` and `SomeFailed` are told apart on purpose: a script must be
/// able to act on "nothing worked" and on "one is missing" without reading the
/// output back.
[[nodiscard]] ExitCode outcomeOf(std::size_t done, std::size_t total);

/// The summary line, `verb` being what was done to the files — "inspected".
///
/// Empty below two files: on a single input, "1 of 1 files inspected" repeats
/// the line just above it.
[[nodiscard]] std::string summaryOf(std::string_view verb, std::size_t done, std::size_t total);

/// Writes the summary through `reporter` and returns the exit code.
[[nodiscard]] ExitCode
tally(const Reporter& reporter, std::string_view verb, std::size_t done, std::size_t total);

} // namespace subedit::cli
