#include "hearing_impaired.hpp"

#include <subedit/cli/hearing_impaired.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `hearing-impaired` was asked for.
///
/// Nothing but files and a destination: the rule is decided, not configurable,
/// and it applies to the whole of the main text. What a phase 12 will make
/// adjustable is written in its spec, not guessed at here.
struct HearingImpairedOptions {
    std::vector<std::string> files;
    DestinationOptions destination;
};

CLI::App*
describeHearingImpaired(CLI::App& app, std::string_view name, HearingImpairedOptions& options) {
    CLI::App* hearing = app.add_subcommand(
        std::string{name}, "Remove the sounds described between brackets or parentheses");
    hearing->add_option("files", options.files, "Subtitle files to clean")->required();

    describeDestination(hearing, options.destination);
    return hearing;
}

ExitCode runHearingImpaired(const HearingImpairedOptions& options,
                            core::FileSystem& files,
                            const std::optional<core::Encoding>& reading,
                            const Reporter& reporter) {
    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return removeHearingImpairedIn(files, options.files, reading, *destination, reporter);
}

} // namespace

Declared declareHearingImpaired(CLI::App& app, std::string_view name) {
    return declareWith<HearingImpairedOptions>(
        app, name, describeHearingImpaired, runHearingImpaired);
}

} // namespace subedit::cli
