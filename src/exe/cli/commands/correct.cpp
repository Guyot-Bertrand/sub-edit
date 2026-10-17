#include "correct.hpp"

#include <subedit/cli/correcting.hpp>
#include <subedit/cli/shared_settings.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/enchant_spell_provider.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/wording/correction.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/platform/locations.hpp>

#include <CLI/CLI.hpp>
#include <expected>
#include <optional>
#include <string>
#include <utility>
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
    CLI::App* correct =
        app.add_subcommand(std::string{name},
                           std::string{core::translate(
                               "Correct the texts with the patterns of the correction assistant")});
    correct
        ->add_option(
            "files", options.files, std::string{core::translate("Subtitle files to correct")})
        ->required();
    describeRecursive(correct, options.recursive);
    correct
        ->add_option(
            "--tasks",
            options.correction.tasks,
            std::string{core::translate("The tasks to run, in a list: none is chosen beforehand")})
        ->required()
        ->option_text("LIST");
    correct
        ->add_option(
            "--code",
            options.correction.code,
            std::string{core::translate(
                "The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US")})
        ->option_text("CODE");
    correct
        ->add_option(
            "--classes",
            options.correction.classes,
            std::string{core::translate("The classes of common errors to apply: both by default")})
        ->option_text("human|ocr|human,ocr");
    correct
        ->add_option("--enable",
                     options.correction.enable,
                     std::string{core::translate(
                         "Switch a pattern on, by its English name or type:name; repeatable")})
        ->option_text("NAME");
    correct
        ->add_option("--disable",
                     options.correction.disable,
                     std::string{core::translate(
                         "Switch a pattern off, by its English name or type:name; repeatable")})
        ->option_text("NAME");
    correct
        ->add_option("--language",
                     options.correction.language,
                     std::string{core::translate(
                         "The dictionary join-words and split-words read: fr_FR, en_US, en...")})
        ->option_text("CODE");
    correct
        ->add_option(
            "--max-length",
            options.correction.maxLength,
            std::string{core::translate(
                "Longest line of line-break, in characters; no default, Gaupol's is in ems")})
        ->option_text("N");
    correct
        ->add_option("--max-lines",
                     options.correction.maxLines,
                     std::string{core::translate("Most lines of line-break: 3, as Gaupol's")})
        ->option_text("N");
    correct
        ->add_option(
            "--skip-length",
            options.correction.skipLength,
            std::string{core::translate(
                "Leave alone a subtitle whose longest line is within this, or off: --max-length")})
        ->option_text("N|off");
    correct
        ->add_option(
            "--skip-lines",
            options.correction.skipLines,
            std::string{core::translate(
                "Leave alone a subtitle whose line count is within this, or off: --max-lines")})
        ->option_text("N|off");
    correct->add_flag(
        "--keep-blank-subtitles",
        options.correction.keepBlankSubtitles,
        std::string{core::translate(
            "Leave the subtitles the correction empties, empty, instead of removing them")});
    // The opposite, so that a settings file that keeps them can be undone on
    // the line (#705).
    correct->add_flag(
        "--remove-blank-subtitles",
        options.correction.removeBlankSubtitles,
        std::string{core::translate(
            "Remove the subtitles the correction empties, whatever the settings file says")});
    describeRange(correct, options.range);
    describeDocument(correct, options.translation);

    describeDestination(correct, options.destination);
    return correct;
}

ExitCode runCorrect(const CorrectOptions& options,
                    core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter,
                    const SharedSettings& shared) {
    // A file named and not there is a mistake in the command line, not a
    // request for the defaults.
    if (const std::optional<std::string> missing = missingNamedFile(shared)) {
        return refuse(*missing);
    }
    announceSettings(reporter, shared);
    // Everything that can be refused is, before a file is read. The file gives
    // three things only; an option beats it (spec 15, S8).
    std::expected<CorrectionRun, std::string> run = prepareCorrection(
        files,
        reporter,
        options.correction,
        platform::installedPatternsPath(),
        platform::resolvedUserPatternsPath(),
        [] { return std::make_unique<core::EnchantSpellProvider>(); },
        shared.values.correction);
    if (!run) {
        return refuse(run.error());
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

    return correctIn(files,
                     prepared->inputs.paths,
                     reading,
                     run->catalogue,
                     run->settings,
                     prepared->range,
                     prepared->destination,
                     reporter,
                     prepared->pairing,
                     run->spellChecker ? &*run->spellChecker : nullptr,
                     settingsRecord(shared));
}

} // namespace

Declared declareCorrect(CLI::App& app, std::string_view name) {
    return declareWith<CorrectOptions>(app, name, describeCorrect, runCorrect);
}

} // namespace subedit::cli
