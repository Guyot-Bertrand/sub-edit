#include "repair.hpp"

#include <subedit/cli/disjoining.hpp>
#include <subedit/cli/joining.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `repair` was asked for: files, a destination, and the settings of the
/// cut it ends with.
struct RepairOptions {
    std::vector<std::string> files;
    bool recursive = false;
    DisjointOptions settings;
    DestinationOptions destination;
};

CLI::App* describeRepair(CLI::App& app, std::string_view name, RepairOptions& options) {
    CLI::App* repair = app.add_subcommand(
        std::string{name},
        "Join the repeated subtitles, put them in order, then make the ranges disjoint");
    repair->add_option("files", options.files, "Subtitle files to repair")->required();
    describeRecursive(repair, options.recursive);
    repair
        ->add_option("--minimum",
                     options.settings.minimum,
                     "Shortest interval kept; a shorter one is taken over by a neighbour "
                     "(default 0.08)")
        ->option_text("TIME");
    repair
        ->add_option("--max-lines",
                     options.settings.maxLines,
                     "Most lines of a merged subtitle; more are shown in windows, 0 for no limit "
                     "(default 2)")
        ->option_text("N");
    repair->add_flag(
        "--dashes", options.settings.dashes, "Put a dash before the text of each merged subtitle");

    describeDestination(repair, options.destination);
    return repair;
}

ExitCode runRepair(const RepairOptions& options,
                   core::FileSystem& files,
                   const std::optional<core::Encoding>& reading,
                   const Reporter& reporter) {
    const std::expected<core::DisjointSettings, std::string> settings =
        settingsOf(options.settings);
    if (!settings) {
        return refuse(settings.error());
    }

    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files, .recursive = options.recursive},
                       options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    return repairAll(
        files, prepared->inputs.paths, reading, *settings, prepared->destination, reporter);
}

} // namespace

Declared declareRepair(CLI::App& app, std::string_view name) {
    return declareWith<RepairOptions>(app, name, describeRepair, runRepair);
}

} // namespace subedit::cli
