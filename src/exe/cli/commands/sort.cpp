#include "sort.hpp"

#include <subedit/cli/sorting.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `sort` was asked for: files and a destination, and nothing to choose —
/// the order is the order of the starts.
struct SortOptions {
    std::vector<std::string> files;
    bool recursive = false;
    DestinationOptions destination;
};

CLI::App* describeSort(CLI::App& app, std::string_view name, SortOptions& options) {
    CLI::App* sort = app.add_subcommand(
        std::string{name},
        "Put the subtitles in the order of their start, keeping ties as they are");
    sort->add_option("files", options.files, "Subtitle files to sort")->required();
    describeRecursive(sort, options.recursive);

    describeDestination(sort, options.destination);
    return sort;
}

ExitCode runSort(const SortOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter) {
    const std::expected<Inputs, std::string> inputs = expandInputs(
        files, options.files, options.recursive, options.destination.outputDir, reporter);
    if (!inputs) {
        return refuse(inputs.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, *inputs);
    if (!destination) {
        return refuse(destination.error());
    }

    return sortAll(files, inputs->paths, reading, *destination, reporter);
}

} // namespace

Declared declareSort(CLI::App& app, std::string_view name) {
    return declareWith<SortOptions>(app, name, describeSort, runSort);
}

} // namespace subedit::cli
