#include "append.hpp"

#include <subedit/cli/appending.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `append` was asked for: the base, what follows it, and one output.
///
/// **No `--output-dir`, no `--in-place`, no `--recursive`**: N inputs make one
/// output, so there is no directory to put it in, no input to write it over, and
/// the order of the files is the order they are given — a walked tree has none.
struct AppendOptions {
    std::vector<std::string> files;
    std::string output;
    bool dryRun = false;
    bool sort = false;
};

CLI::App* describeAppend(CLI::App& app, std::string_view name, AppendOptions& options) {
    CLI::App* append = app.add_subcommand(
        std::string{name}, "Put subtitle files one after another into a single file");
    append
        ->add_option(
            "files", options.files, "The base file, then the files to put after it, in order")
        ->required()
        ->option_text("BASE FILE...");
    append->add_option("--output", options.output, "File to write")->option_text("FILE");
    append->add_flag(
        "--dry-run", options.dryRun, "Work out and say what would be written, and write nothing");
    append->add_flag(
        "--sort", options.sort, "Put the subtitles in order of their start before writing");
    return append;
}

ExitCode runAppend(const AppendOptions& options,
                   core::FileSystem& files,
                   const std::optional<core::Encoding>& reading,
                   const Reporter& reporter) {
    if (options.files.size() < 2) {
        return refuse("append needs a base file and at least one file to put after it");
    }
    if (options.output.empty() && !options.dryRun) {
        return refuse("no destination given: append writes one file, use --output");
    }

    const std::expected<Destination, std::string> destination =
        Destination::from(options.output, "", false, 1, options.dryRun);
    if (!destination) {
        return refuse(destination.error());
    }
    return appendAll(files, options.files, reading, *destination, reporter, options.sort);
}

} // namespace

Declared declareAppend(CLI::App& app, std::string_view name) {
    return declareWith<AppendOptions>(app, name, describeAppend, runAppend);
}

} // namespace subedit::cli
