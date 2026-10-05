#include "correct.hpp"

#include <subedit/cli/correcting.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/wording/correction.hpp>
#include <subedit/platform/locations.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `correct` was asked for.
struct CorrectOptions {
    CorrectionOptions correction;
    std::vector<std::string> files;
    bool recursive = false;
    std::string range;
    TranslationOptions translation;
    DestinationOptions destination;
};

CLI::App* describeCorrect(CLI::App& app, std::string_view name, CorrectOptions& options) {
    CLI::App* correct = app.add_subcommand(
        std::string{name}, "Correct the texts with the patterns of the correction assistant");
    correct->add_option("files", options.files, "Subtitle files to correct")->required();
    describeRecursive(correct, options.recursive);
    correct
        ->add_option("--tasks",
                     options.correction.tasks,
                     "The tasks to run, in a list: none is chosen beforehand")
        ->required()
        ->option_text("LIST");
    correct
        ->add_option("--code",
                     options.correction.code,
                     "The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US")
        ->option_text("CODE");
    correct
        ->add_option("--classes",
                     options.correction.classes,
                     "The classes of common errors to apply: both by default")
        ->option_text("human|ocr|human,ocr");
    correct
        ->add_option("--enable",
                     options.correction.enable,
                     "Switch a pattern on, by its English name or type:name; repeatable")
        ->option_text("NAME");
    correct
        ->add_option("--disable",
                     options.correction.disable,
                     "Switch a pattern off, by its English name or type:name; repeatable")
        ->option_text("NAME");
    correct
        ->add_option("--max-length",
                     options.correction.maxLength,
                     "Longest line of line-break, in characters; no default, Gaupol's is in ems")
        ->option_text("N");
    correct
        ->add_option(
            "--max-lines", options.correction.maxLines, "Most lines of line-break: 3, as Gaupol's")
        ->option_text("N");
    correct
        ->add_option(
            "--skip-length",
            options.correction.skipLength,
            "Leave alone a subtitle whose longest line is within this, or off: --max-length")
        ->option_text("N|off");
    correct
        ->add_option("--skip-lines",
                     options.correction.skipLines,
                     "Leave alone a subtitle whose line count is within this, or off: --max-lines")
        ->option_text("N|off");
    correct->add_flag(
        "--keep-blank-subtitles",
        options.correction.keepBlankSubtitles,
        "Leave the subtitles the correction empties, empty, instead of removing them");
    describeRange(correct, options.range);
    describeDocument(correct, options.translation);

    describeDestination(correct, options.destination);
    return correct;
}

ExitCode runCorrect(const CorrectOptions& options,
                    core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    // Everything that can be refused is, before a file is read — and the patterns
    // are read once, from where the executable and the environment say they are
    // (ADR 0037): the shipped ones, and those a user dropped.
    const core::PatternCatalogue catalogue = core::readPatternCatalogue(
        files, platform::installedPatternsPath(), platform::resolvedUserPatternsPath());
    // An installation without its patterns says so, and the run that follows
    // cannot find a pattern to play and says that as well.
    for (const core::PatternDiagnostic& diagnostic : catalogue.diagnostics()) {
        reporter.say(1, "patterns: " + core::describe(diagnostic));
    }

    const std::expected<core::CorrectionSettings, std::string> settings =
        correctionSettingsOf(options.correction, catalogue);
    if (!settings) {
        return refuse(settings.error());
    }

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

    return correctIn(files,
                     inputs->paths,
                     reading,
                     catalogue,
                     *settings,
                     *range,
                     *destination,
                     reporter,
                     *pairing);
}

} // namespace

Declared declareCorrect(CLI::App& app, std::string_view name) {
    return declareWith<CorrectOptions>(app, name, describeCorrect, runCorrect);
}

} // namespace subedit::cli
