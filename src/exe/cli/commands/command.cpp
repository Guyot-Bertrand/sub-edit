#include "command.hpp"

#include <subedit/cli/frame_rate_grammar.hpp>

#include <CLI/CLI.hpp>
#include <iostream>

namespace subedit::cli {

void describeDestination(CLI::App* command, DestinationOptions& options) {
    command->add_option("--output", options.output, "File to write, for a single input");
    command->add_option("--output-dir", options.outputDir, "Directory to write into");
    command->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    command->add_flag(
        "--dry-run", options.dryRun, "Work out and say what would be written, and write nothing");
}

std::expected<Destination, std::string> destinationOf(const DestinationOptions& options,
                                                      const Inputs& inputs) {
    const std::expected<Destination, std::string> destination = Destination::from(
        options.output, options.outputDir, options.inPlace, inputs.paths.size(), options.dryRun);
    if (!destination) {
        return destination;
    }
    return destination->withRoots(inputs.roots);
}

void describeRecursive(CLI::App* command, bool& recursive) {
    command->add_flag(
        "-r,--recursive", recursive, "Take directories as inputs, and every subtitle file in them");
}

void describeRange(CLI::App* command, std::string& range) {
    command->add_option("--range", range, "Act only on subtitles N to M, or N to the end")
        ->option_text("N-M|N-");
}

std::expected<std::optional<Range>, std::string> rangeOf(const std::string& range) {
    if (range.empty()) {
        return std::optional<Range>{};
    }
    const std::expected<Range, std::string> read = parseRange(range);
    if (!read) {
        return std::unexpected{"--range: " + read.error()};
    }
    return std::optional{*read};
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
