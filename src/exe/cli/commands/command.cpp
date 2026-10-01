#include "command.hpp"

#include <subedit/cli/frame_rate_grammar.hpp>

#include <CLI/CLI.hpp>
#include <iostream>

namespace subedit::cli {

void describeDestination(CLI::App* command, DestinationOptions& options) {
    command->add_option("--output", options.output, "File to write, for a single input");
    command->add_option("--output-dir", options.outputDir, "Directory to write into");
    command->add_flag("--in-place", options.inPlace, "Write back over the inputs");
}

std::expected<Destination, std::string> destinationOf(const DestinationOptions& options,
                                                      std::size_t inputCount) {
    return Destination::from(options.output, options.outputDir, options.inPlace, inputCount);
}

ExitCode refuse(std::string_view why) {
    std::cerr << why << '\n';
    return ExitCode::Usage;
}

std::expected<core::ReadingChoices, std::string>
readingWith(const std::optional<core::Encoding>& encoding, const std::string& frameRate) {
    core::ReadingChoices choices{.encoding = encoding};
    if (frameRate.empty())
        return choices;

    const std::expected<core::FrameRate, std::string> rate = parseFrameRate(frameRate);
    if (!rate.has_value())
        return std::unexpected("--frame-rate: " + rate.error());
    choices.frameRate = *rate;
    return choices;
}

} // namespace subedit::cli
