#include "adjust.hpp"

#include <subedit/cli/adjusting.hpp>
#include <subedit/cli/shared_settings.hpp>
#include <subedit/core/i18n/messages.hpp>

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
        std::string{core::translate(
            "Bring the duration of every subtitle within a reading speed and limits")});
    adjust
        ->add_option(
            "files", options.files, std::string{core::translate("Subtitle files to adjust")})
        ->required();
    describeRecursive(adjust, options.recursive);
    adjust
        ->add_option("--speed",
                     options.constraints.speed,
                     std::string{core::translate(
                         "Reading speed in characters per second, or off (default 15)")})
        ->option_text("CPS|off");
    adjust->add_flag("--shorten",
                     options.constraints.shorten,
                     std::string{core::translate("Let the reading speed bring an end earlier")});
    // The opposites, so that every direction the settings file sets can be
    // undone on the line (#705).
    adjust->add_flag(
        "--no-shorten",
        options.constraints.noShorten,
        std::string{core::translate("Do not let the reading speed bring an end earlier")});
    adjust->add_flag("--lengthen",
                     options.constraints.lengthen,
                     std::string{core::translate("Let the reading speed move an end later")});
    adjust->add_flag(
        "--no-lengthen",
        options.constraints.noLengthen,
        std::string{core::translate("Do not let the reading speed move an end later")});
    adjust
        ->add_option("--minimum",
                     options.constraints.minimum,
                     std::string{core::translate("Shortest duration, or off (default 1.5)")})
        ->option_text("TIME|off");
    adjust
        ->add_option("--maximum",
                     options.constraints.maximum,
                     std::string{core::translate("Longest duration, or off (default: none)")})
        ->option_text("TIME|off");
    adjust
        ->add_option("--gap",
                     options.constraints.gap,
                     std::string{core::translate(
                         "Least time left before the next subtitle, or off (default 0)")})
        ->option_text("TIME|off");
    describeRange(adjust, options.range);

    describeDestination(adjust, options.destination);
    return adjust;
}

ExitCode runAdjust(const AdjustCommandOptions& options,
                   core::FileSystem& files,
                   const std::optional<core::Encoding>& reading,
                   const Reporter& reporter,
                   const SharedSettings& shared) {
    // A file named and not there is a mistake in the command line, not a
    // request for the defaults.
    if (const std::optional<std::string> missing = missingNamedFile(shared)) {
        return refuse(*missing);
    }
    announceSettings(reporter, shared);
    // An option beats the file, which beats the core default (spec 15, S8).
    const std::expected<core::DurationConstraints, std::string> constraints =
        constraintsOf(options.constraints, shared.values.durationAdjustment);
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
                     settingsRecord(shared),
                     prepared->range,
                     prepared->destination,
                     reporter);
}

} // namespace

Declared declareAdjust(CLI::App& app, std::string_view name) {
    return declareWith<AdjustCommandOptions>(app, name, describeAdjust, runAdjust);
}

} // namespace subedit::cli
