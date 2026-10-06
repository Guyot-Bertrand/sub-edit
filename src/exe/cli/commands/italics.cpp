#include "italics.hpp"

#include <subedit/cli/text_rewriting.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `italics` was asked for: which way, and where.
///
/// **Two words and not a toggle**: the core takes a direction, and the
/// decision — put on, or take out — stays with whoever is typing.
struct ItalicsOptions {
    std::vector<std::string> files;
    bool recursive = false;
    bool on = false;
    bool off = false;
    std::string range;
    TranslationOptions translation;
    DestinationOptions destination;
};

CLI::App* describeItalics(CLI::App& app, std::string_view name, ItalicsOptions& options) {
    CLI::App* italics = app.add_subcommand(std::string{name},
                                           "Put the texts in italics, or take their italics out");
    italics->add_option("files", options.files, "Subtitle files to change")->required();
    describeRecursive(italics, options.recursive);
    italics->add_flag("--on", options.on, "Put the texts in italics");
    italics->add_flag("--off", options.off, "Take the italics out");
    describeRange(italics, options.range);
    describeDocument(italics, options.translation);

    describeDestination(italics, options.destination);
    return italics;
}

ExitCode runItalics(const ItalicsOptions& options,
                    core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    if (options.on == options.off) {
        return refuse(options.on ? "--on and --off say opposite things; give one of them"
                                 : "italics needs --on to put the texts in italics, or --off");
    }

    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files,
                        .recursive = options.recursive,
                        .range = &options.range,
                        .translation = &options.translation},
                       options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    return italicsIn(files,
                     prepared->inputs.paths,
                     reading,
                     options.on,
                     prepared->range,
                     prepared->destination,
                     reporter,
                     prepared->pairing);
}

} // namespace

Declared declareItalics(CLI::App& app, std::string_view name) {
    return declareWith<ItalicsOptions>(app, name, describeItalics, runItalics);
}

} // namespace subedit::cli
