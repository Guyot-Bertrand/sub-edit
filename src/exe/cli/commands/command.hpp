#pragma once

// What every subcommand has in common: the shape of its registration, and the
// few pieces more than one of them uses.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>

#include <CLI/CLI.hpp>
#include <cstddef>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::cli {

/// What a subcommand leaves behind once it is declared: where to find it after
/// parsing, and what to run if it is the one that was named.
///
/// The options live in the closure, so that `run()` has no type to know.
struct Declared {
    const CLI::App* app = nullptr;
    std::function<ExitCode(core::FileSystem& files,
                           const std::optional<core::Encoding>& reading,
                           const Reporter& reporter)>
        run;
};

/// Declares one subcommand named `name` on `app`.
using Declare = Declared (*)(CLI::App& app, std::string_view name);

/// Declares a subcommand whose options are an `Options`.
///
/// The seven subcommands all had the same four lines: make the options, describe
/// them, keep them alive, run with them. Here once.
template<typename Options, typename Describe, typename Run>
[[nodiscard]] Declared
declareWith(CLI::App& app, std::string_view name, Describe describe, Run run) {
    auto options = std::make_shared<Options>();
    const CLI::App* command = describe(app, name, *options);
    return Declared{command,
                    [options, run](core::FileSystem& files,
                                   const std::optional<core::Encoding>& reading,
                                   const Reporter& reporter) {
                        return run(*options, files, reading, reporter);
                    }};
}

/// Where a subcommand writes, as the three options that say it.
///
/// A type of its own rather than three fields repeated in four structs: they
/// always travel together, they are always declared the same way, and they are
/// always turned into a `Destination` by the same call. A fifth subcommand that
/// writes now costs none of the three.
struct DestinationOptions {
    std::string output;
    std::string outputDir;
    bool inPlace = false;
};

/// Declares the three options on `command`.
///
/// Called last by every `describe…`, so that a subcommand lists its own options
/// before those it shares — the order the help shows and the manual quotes.
void describeDestination(CLI::App* command, DestinationOptions& options);

/// The destination those options describe, or why they cannot be honoured.
[[nodiscard]] std::expected<Destination, std::string>
destinationOf(const DestinationOptions& options, std::size_t inputCount);

/// Writes a refusal and gives the code that goes with it.
///
/// Every value the subcommands read can be refused, and each refusal was written
/// out in four lines, nine times over. Named once, a usage error stops being a
/// shape one has to recognise.
ExitCode refuse(std::string_view why);

/// The two things a reading can be told, once both have been checked.
///
/// **Named once because two subcommands say it**, and because the refusal has
/// to be the same sentence in both: a rate that names nothing is a usage error,
/// not a file that failed.
[[nodiscard]] std::expected<core::ReadingChoices, std::string>
readingWith(const std::optional<core::Encoding>& encoding, const std::string& frameRate);

} // namespace subedit::cli
