#include "framerate.hpp"

#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/frame_rate_grammar.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `framerate` was asked for.
struct FrameRateOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string from;
    std::string to;
    DestinationOptions destination;
};

CLI::App* describeFrameRate(CLI::App& app, std::string_view name, FrameRateOptions& options) {
    CLI::App* framerate = app.add_subcommand(
        std::string{name}, "Re-time a file mastered at one frame rate for another");
    framerate->add_option("files", options.files, "Subtitle files to re-time")->required();
    describeRecursive(framerate, options.recursive);
    framerate->add_option("--from", options.from, "Frame rate the file is timed at: 25, 23.976")
        ->required();
    framerate->add_option("--to", options.to, "Frame rate to time it for: 24, 29.97")->required();

    describeDestination(framerate, options.destination);
    return framerate;
}

ExitCode runFrameRate(const FrameRateOptions& options,
                      core::FileSystem& files,
                      const std::optional<core::Encoding>& reading,
                      const Reporter& reporter) {
    const std::expected<core::FrameRate, std::string> from = parseFrameRate(options.from);
    if (!from) {
        return refuse(from.error());
    }

    const std::expected<core::FrameRate, std::string> to = parseFrameRate(options.to);
    if (!to) {
        return refuse(to.error());
    }

    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files, .recursive = options.recursive},
                       options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    return convertFrameRateAll(
        files, prepared->inputs.paths, reading, *from, *to, prepared->destination, reporter);
}

} // namespace

Declared declareFrameRate(CLI::App& app, std::string_view name) {
    return declareWith<FrameRateOptions>(app, name, describeFrameRate, runFrameRate);
}

} // namespace subedit::cli
