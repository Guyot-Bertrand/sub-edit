#include "adjust.hpp"

#include <subedit/cli/adjusting.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `adjust` was asked for.
struct AdjustCommandOptions {
    std::vector<std::string> files;
    bool recursive = false;
    AdjustOptions constraints;
    std::string range;
    DestinationOptions destination;
};

CLI::App* describeAdjust(CLI::App& app, std::string_view name, AdjustCommandOptions& options) {
    CLI::App* adjust = app.add_subcommand(
        std::string{name},
        "Bring the duration of every subtitle within a reading speed and limits");
    adjust->add_option("files", options.files, "Subtitle files to adjust")->required();
    describeRecursive(adjust, options.recursive);
    adjust
        ->add_option("--speed",
                     options.constraints.speed,
                     "Reading speed in characters per second, or off (default 15)")
        ->option_text("CPS|off");
    adjust->add_flag(
        "--shorten", options.constraints.shorten, "Let the reading speed bring an end earlier");
    adjust->add_flag("--no-lengthen",
                     options.constraints.noLengthen,
                     "Do not let the reading speed move an end later");
    adjust
        ->add_option(
            "--minimum", options.constraints.minimum, "Shortest duration, or off (default 1.5)")
        ->option_text("TIME|off");
    adjust->add_option("--maximum", options.constraints.maximum, "Longest duration (default: none)")
        ->option_text("TIME");
    adjust
        ->add_option("--gap",
                     options.constraints.gap,
                     "Least time left before the next subtitle, or off (default 0)")
        ->option_text("TIME|off");
    describeRange(adjust, options.range);

    describeDestination(adjust, options.destination);
    return adjust;
}

ExitCode runAdjust(const AdjustCommandOptions& options,
                   core::FileSystem& files,
                   const std::optional<core::Encoding>& reading,
                   const Reporter& reporter) {
    const std::expected<core::DurationConstraints, std::string> constraints =
        constraintsOf(options.constraints);
    if (!constraints) {
        return refuse(constraints.error());
    }

    const std::expected<PreparedWriting, std::string> prepared = prepareWriting(
        files,
        reporter,
        {.files = options.files, .recursive = options.recursive, .range = &options.range},
        options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    return adjustAll(files,
                     prepared->inputs.paths,
                     reading,
                     *constraints,
                     prepared->range,
                     prepared->destination,
                     reporter);
}

} // namespace

Declared declareAdjust(CLI::App& app, std::string_view name) {
    return declareWith<AdjustCommandOptions>(app, name, describeAdjust, runAdjust);
}

} // namespace subedit::cli
