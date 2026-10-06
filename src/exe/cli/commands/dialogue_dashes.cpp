#include "dialogue_dashes.hpp"

#include <subedit/cli/text_rewriting.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `dialogue-dashes` was asked for: which way, and where.
struct DialogueDashesOptions {
    std::vector<std::string> files;
    bool recursive = false;
    bool add = false;
    bool remove = false;
    std::string range;
    TranslationOptions translation;
    DestinationOptions destination;
};

CLI::App*
describeDialogueDashes(CLI::App& app, std::string_view name, DialogueDashesOptions& options) {
    CLI::App* dashes =
        app.add_subcommand(std::string{name}, "Put dialogue dashes on the lines, or take them off");
    dashes->add_option("files", options.files, "Subtitle files to change")->required();
    describeRecursive(dashes, options.recursive);
    dashes->add_flag("--add", options.add, "Put a dialogue dash at the head of the lines");
    dashes->add_flag("--remove", options.remove, "Take the dialogue dashes off");
    describeRange(dashes, options.range);
    describeDocument(dashes, options.translation);

    describeDestination(dashes, options.destination);
    return dashes;
}

ExitCode runDialogueDashes(const DialogueDashesOptions& options,
                           core::FileSystem& files,
                           const std::optional<core::Encoding>& reading,
                           const Reporter& reporter) {
    if (options.add == options.remove) {
        return refuse(options.add ? "--add and --remove say opposite things; give one of them"
                                  : "dialogue-dashes needs --add to put dashes on, or --remove");
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

    return dialogueDashesIn(files,
                            prepared->inputs.paths,
                            reading,
                            options.add,
                            prepared->range,
                            prepared->destination,
                            reporter,
                            prepared->pairing);
}

} // namespace

Declared declareDialogueDashes(CLI::App& app, std::string_view name) {
    return declareWith<DialogueDashesOptions>(app, name, describeDialogueDashes, runDialogueDashes);
}

} // namespace subedit::cli
