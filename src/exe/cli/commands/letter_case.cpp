#include "letter_case.hpp"

#include <subedit/cli/text_rewriting.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `case` was asked for.
struct CaseOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string to;
    std::string range;
    TranslationOptions translation;
    DestinationOptions destination;
};

CLI::App* describeCase(CLI::App& app, std::string_view name, CaseOptions& options) {
    CLI::App* recase = app.add_subcommand(
        std::string{name}, "Put the texts in title, sentence, upper or lower case, tags intact");
    recase->add_option("files", options.files, "Subtitle files to change")->required();
    describeRecursive(recase, options.recursive);
    recase->add_option("--to", options.to, "The case to put the texts in")
        ->required()
        ->check(CLI::IsMember({"title", "sentence", "upper", "lower"}))
        ->option_text("title|sentence|upper|lower");
    describeRange(recase, options.range);
    describeDocument(recase, options.translation);

    describeDestination(recase, options.destination);
    return recase;
}

ExitCode runCase(const CaseOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter) {
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

    return recaseIn(files,
                    prepared->inputs.paths,
                    reading,
                    letterCaseNamed(options.to),
                    prepared->range,
                    prepared->destination,
                    reporter,
                    prepared->pairing);
}

} // namespace

Declared declareCase(CLI::App& app, std::string_view name) {
    return declareWith<CaseOptions>(app, name, describeCase, runCase);
}

} // namespace subedit::cli
