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
    std::string by;
    bool toGrid = false;
    DestinationOptions destination;
};

CLI::App* describeShift(CLI::App& app, std::string_view name, ShiftOptions& options) {
    CLI::App* shift =
        app.add_subcommand(std::string{name}, "Move every position of a file by a fixed amount");
    shift->add_option("files", options.files, "Subtitle files to shift")->required();
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
    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    if (options.toGrid && !options.by.empty()) {
        return refuse("--by and --to-grid both say by how much to move; give one or the other");
    }

    // Measured rather than given, and file by file: two files shifted off the
    // same grid by different amounts come back by different amounts.
    if (options.toGrid) {
        return shiftOntoGridAll(files, options.files, reading, *destination, reporter);
    }

    if (options.by.empty()) {
        return refuse("shift needs --by, or --to-grid to work the amount out from the positions");
    }

    const std::expected<core::Duration, std::string> by = parseTime(options.by);
    if (!by) {
        return refuse(by.error());
    }

    return shiftAll(files, options.files, reading, *by, *destination, reporter);
}

} // namespace

Declared declareShift(CLI::App& app, std::string_view name) {
    return declareWith<ShiftOptions>(app, name, describeShift, runShift);
}

} // namespace subedit::cli
