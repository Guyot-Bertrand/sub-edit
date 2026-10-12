#include "disjoint.hpp"

#include <subedit/cli/disjoining.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `disjoint` was asked for: files, a destination and the three settings.
struct DisjointCommandOptions {
    std::vector<std::string> files;
    bool recursive = false;
    DisjointOptions settings;
    DestinationOptions destination;
};

CLI::App* describeDisjoint(CLI::App& app, std::string_view name, DisjointCommandOptions& options) {
    CLI::App* disjoint = app.add_subcommand(
        std::string{name},
        std::string{core::translate(
            "Cut overlapping subtitles so that no range overlaps another, merging what overlaps")});
    disjoint
        ->add_option(
            "files", options.files, std::string{core::translate("Subtitle files to make disjoint")})
        ->required();
    describeRecursive(disjoint, options.recursive);
    disjoint
        ->add_option("--minimum",
                     options.settings.minimum,
                     std::string{core::translate(
                         "Shortest interval kept; a shorter one is taken over by a neighbour "
                         "(default 0.08)")})
        ->option_text("TIME");
    disjoint
        ->add_option(
            "--max-lines",
            options.settings.maxLines,
            std::string{core::translate(
                "Most lines of a merged subtitle; more are shown in windows, 0 for no limit "
                "(default 2)")})
        ->option_text("N");
    disjoint->add_flag(
        "--dashes",
        options.settings.dashes,
        std::string{core::translate("Put a dash before the text of each merged subtitle")});
    disjoint->add_flag(
        "--sort",
        options.settings.sort,
        std::string{core::translate(
            "Put the subtitles in order of their start first; without it, a file out of "
            "order is refused")});

    describeDestination(disjoint, options.destination);
    return disjoint;
}

ExitCode runDisjoint(const DisjointCommandOptions& options,
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

    return disjointAll(files,
                       prepared->inputs.paths,
                       reading,
                       *settings,
                       options.settings.sort,
                       prepared->destination,
                       reporter);
}

} // namespace

Declared declareDisjoint(CLI::App& app, std::string_view name) {
    return declareWith<DisjointCommandOptions>(app, name, describeDisjoint, runDisjoint);
}

} // namespace subedit::cli
