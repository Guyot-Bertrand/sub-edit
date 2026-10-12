#pragma once

// Turning a run over several files into a summary and an exit code.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/exit_code.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
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

/// Makes the directories `outputs` are to be written into, parents included, each
/// once. **Nothing when all of them are there**, and when one cannot be made: it
/// is said once, nothing is written, and the code is `AllFailed`.
///
/// The step `arrange` takes for a batch, for the subcommands that name their own
/// outputs (`split-file`).
[[nodiscard]] std::optional<ExitCode>
createDirectoriesFor(subedit::core::FileSystem& files,
                     std::span<const std::filesystem::path> outputs,
                     const Reporter& reporter);

/// How a run over `total` files ended, `done` of them having succeeded.
///
/// `AllFailed` and `SomeFailed` are told apart on purpose: a script must be
/// able to act on "nothing worked" and on "one is missing" without reading the
/// output back.
[[nodiscard]] ExitCode outcomeOf(std::size_t done, std::size_t total);

/// What a batch did to its files, as its summary says it — "3 of 5 files shifted".
///
/// **A closed list, not a string**, for the reason `core::Counted` is one: each
/// verb has its own sentence in the catalogue, so that a language may agree the
/// participle with the files and choose its plural by their number (spec 15, D2).
enum class Verb {
    Adjusted,
    Aligned,
    Changed,
    Cleaned,
    Converted,
    Corrected,
    Inspected,
    Joined,
    MadeDisjoint,
    Paired,
    Recased,
    Repaired,
    Replaced,
    Retimed,
    Shifted,
    Sorted,
    Transformed,
};

/// The summary line, `verb` being what was done to the files.
///
/// Empty below two files: on a single input, "1 of 1 files inspected" repeats
/// the line just above it.
[[nodiscard]] std::string summaryOf(Verb verb, std::size_t done, std::size_t total);

/// Writes the summary through `reporter` and returns the exit code.
[[nodiscard]] ExitCode
tally(const Reporter& reporter, Verb verb, std::size_t done, std::size_t total);

} // namespace subedit::cli
