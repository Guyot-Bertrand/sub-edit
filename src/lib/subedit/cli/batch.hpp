#pragma once

// Turning a run over several files into a summary and an exit code.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/exit_code.hpp>

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// Everything a batch that writes settles before it touches a file: where each
/// input goes, and that the directories to write into are there.
///
/// **In this order, and before the first file is read** — a refusal of the
/// whole is the same sentence as a refusal of one input, with a different cost:
/// it must never leave a batch half written.
///
/// 1. `Destination::plan` judges the destinations; a collision, or an input
///    that would be written over, is said once and gives `Usage`;
/// 2. the directory each destination lies in is created, parents included —
///    unless the batch is `--in-place`, where they all exist, or `--dry-run`,
///    which creates nothing (CLI-DRYRUN-01). If that fails it
///    is said once, no file is read, and the code is `AllFailed`.
///
/// On success, the jobs, in the order the inputs were given.
[[nodiscard]] std::expected<std::vector<Job>, ExitCode>
arrange(core::FileSystem& files,
        const Destination& destination,
        const std::vector<std::string>& inputs,
        std::string_view extension,
        const Reporter& reporter);

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
