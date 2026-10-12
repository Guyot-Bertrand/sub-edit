#include "join_repeats.hpp"

#include <subedit/cli/joining.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `join-repeats` was asked for: files and a destination, and nothing to
/// choose.
struct JoinOptions {
    std::vector<std::string> files;
    bool recursive = false;
    DestinationOptions destination;
};

CLI::App* describeJoin(CLI::App& app, std::string_view name, JoinOptions& options) {
    CLI::App* join = app.add_subcommand(
        std::string{name},
        std::string{core::translate(
            "Join the subtitles that repeat each other, one after the other, into one")});
    join->add_option("files",
                     options.files,
                     std::string{core::translate("Subtitle files to join the repeats of")})
        ->required();
    describeRecursive(join, options.recursive);

    describeDestination(join, options.destination);
    return join;
}

ExitCode runJoin(const JoinOptions& options,
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

    return joinAll(files, prepared->inputs.paths, reading, prepared->destination, reporter);
}

} // namespace

Declared declareJoinRepeats(CLI::App& app, std::string_view name) {
    return declareWith<JoinOptions>(app, name, describeJoin, runJoin);
}

} // namespace subedit::cli
