#pragma once

// What every subcommand has in common: the shape of its registration, and the
// few pieces more than one of them uses.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/expansion.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/options.hpp>
#include <subedit/cli/pairing.hpp>
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

/// Declares the options on `command`.
///
/// Called last by every `describe…`, so that a subcommand lists its own options
/// before those it shares — the order the help shows and the manual quotes.
void describeDestination(CLI::App* command, DestinationOptions& options);

/// Declares `--recursive` / `-r` on `command`: directories as inputs.
///
/// On every subcommand that takes files — `inspect` included, which reads a
/// tree as readily as it writes one.
void describeRecursive(CLI::App* command, bool& recursive);

/// Declares `--range` on `command`, for the subcommands that act on a selection.
void describeRange(CLI::App* command, std::string& range);

/// Declares `-t/--translation-file` and `--align-method` on `command`: what
/// `inspect` takes to report how a translation lines up.
void describeTranslation(CLI::App* command, TranslationOptions& options);

/// Declares those two and `--document main|translation` too, for the
/// subcommands that change the texts of one of the two documents.
void describeDocument(CLI::App* command, TranslationOptions& options);

/// Writes a refusal and gives the code that goes with it.
///
/// Every value the subcommands read can be refused, and each refusal was written
/// out in four lines, nine times over. Named once, a usage error stops being a
/// shape one has to recognise.
ExitCode refuse(std::string_view why);

} // namespace subedit::cli
