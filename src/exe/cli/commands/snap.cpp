#include "snap.hpp"

#include <subedit/cli/aligning.hpp>
#include <subedit/cli/frame_rate_grammar.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `snap` was asked for.
struct SnapOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string rate;
    DestinationOptions destination;
};

CLI::App* describeSnap(CLI::App& app, std::string_view name, SnapOptions& options) {
    CLI::App* snap = app.add_subcommand(
        std::string{name},
        "Move every position onto the nearest frame of a frame rate (see framerate)");
    snap->add_option("files", options.files, "Subtitle files to align")->required();
    describeRecursive(snap, options.recursive);
    snap->add_option("--rate", options.rate, "Frame rate to align on: 25, 23.976")->required();

    describeDestination(snap, options.destination);
    return snap;
}

ExitCode runSnap(const SnapOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter) {
    const std::expected<core::FrameRate, std::string> rate = parseFrameRate(options.rate);
    if (!rate) {
        return refuse(rate.error());
    }

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

    return alignAll(files, inputs->paths, reading, *rate, *destination, reporter);
}

} // namespace

Declared declareSnap(CLI::App& app, std::string_view name) {
    return declareWith<SnapOptions>(app, name, describeSnap, runSnap);
}

} // namespace subedit::cli
