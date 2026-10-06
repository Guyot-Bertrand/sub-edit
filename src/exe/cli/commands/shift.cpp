#include "shift.hpp"

#include <subedit/cli/shifting.hpp>
#include <subedit/cli/time_grammar.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `shift` was asked for.
///
/// The amount is given by `--by`, or measured by `--to-grid`. Exactly one of
/// the two, and neither is `required()` on its own: CLI11 would then insist on
/// both, and the check below can say why in a sentence rather than in the
/// usage.
struct ShiftOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string by;
    bool toGrid = false;
    DestinationOptions destination;
};

CLI::App* describeShift(CLI::App& app, std::string_view name, ShiftOptions& options) {
    CLI::App* shift =
        app.add_subcommand(std::string{name}, "Move every position of a file by a fixed amount");
    shift->add_option("files", options.files, "Subtitle files to shift")->required();
    describeRecursive(shift, options.recursive);
    shift->add_option("--by", options.by, "Amount to move by: 2.999, -7.001, or 00:00:07.001");
    shift->add_flag("--to-grid",
                    options.toGrid,
                    "Move by the amount that puts the positions back on their frame grid");

    describeDestination(shift, options.destination);
    return shift;
}

ExitCode runShift(const ShiftOptions& options,
                  core::FileSystem& files,
                  const std::optional<core::Encoding>& reading,
                  const Reporter& reporter) {
    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files, .recursive = options.recursive},
                       options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    const std::expected<std::optional<core::Duration>, std::string> by =
        shiftAmountOf(options.toGrid, options.by);
    if (!by) {
        return refuse(by.error());
    }
    if (!by->has_value()) {
        return shiftOntoGridAll(
            files, prepared->inputs.paths, reading, prepared->destination, reporter);
    }

    return shiftAll(files, prepared->inputs.paths, reading, **by, prepared->destination, reporter);
}

} // namespace

Declared declareShift(CLI::App& app, std::string_view name) {
    return declareWith<ShiftOptions>(app, name, describeShift, runShift);
}

} // namespace subedit::cli
