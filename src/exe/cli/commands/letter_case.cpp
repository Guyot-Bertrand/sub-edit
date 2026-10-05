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

/// The case a word names. The four are the closed set `--to` accepts, checked
/// before this is called.
[[nodiscard]] core::LetterCase caseNamed(const std::string& name) {
    if (name == "title") {
        return core::LetterCase::Title;
    }
    if (name == "sentence") {
        return core::LetterCase::Sentence;
    }
    if (name == "upper") {
        return core::LetterCase::Upper;
    }
    return core::LetterCase::Lower;
}

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
    const std::expected<std::optional<Range>, std::string> range = rangeOf(options.range);
    if (!range) {
        return refuse(range.error());
    }

    const std::expected<Inputs, std::string> inputs = expandInputs(
        files, options.files, options.recursive, options.destination.outputDir, reporter);
    if (!inputs) {
        return refuse(inputs.error());
    }

    const std::expected<std::optional<Pairing>, std::string> pairing =
        pairingOf(options.translation, true, *inputs);
    if (!pairing) {
        return refuse(pairing.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, *inputs);
    if (!destination) {
        return refuse(destination.error());
    }

    return recaseIn(files,
                    inputs->paths,
                    reading,
                    caseNamed(options.to),
                    *range,
                    *destination,
                    reporter,
                    *pairing);
}

} // namespace

Declared declareCase(CLI::App& app, std::string_view name) {
    return declareWith<CaseOptions>(app, name, describeCase, runCase);
}

} // namespace subedit::cli
