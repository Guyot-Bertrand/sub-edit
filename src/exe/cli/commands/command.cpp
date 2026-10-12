#include "command.hpp"

#include <subedit/cli/frame_rate_grammar.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

namespace subedit::cli {

void describeDestination(CLI::App* command, DestinationOptions& options) {
    command->add_option("--output",
                        options.output,
                        std::string{core::translate("File to write, for a single input")});
    command->add_option(
        "--output-dir", options.outputDir, std::string{core::translate("Directory to write into")});
    command->add_flag(
        "--in-place", options.inPlace, std::string{core::translate("Write back over the inputs")});
    command->add_flag(
        "--dry-run",
        options.dryRun,
        std::string{core::translate("Work out and say what would be written, and write nothing")});
}

void describeRecursive(CLI::App* command, bool& recursive) {
    command->add_flag("-r,--recursive",
                      recursive,
                      std::string{core::translate(
                          "Take directories as inputs, and every subtitle file in them")});
}

void describeRange(CLI::App* command, std::string& range) {
    command
        ->add_option("--range",
                     range,
                     std::string{core::translate("Act only on subtitles N to M, or N to the end")})
        ->option_text("N-M|N-");
}

void describeTranslation(CLI::App* command, TranslationOptions& options) {
    command
        ->add_option("-t,--translation-file",
                     options.file,
                     std::string{core::translate(
                         "Translation file to lay over the subtitle file, for a single input")})
        ->option_text("FILE");
    command
        ->add_option(
            "--align-method",
            options.alignMethod,
            std::string{core::translate("How the lines of the translation find their subtitles")})
        ->check(CLI::IsMember({"position", "number"}))
        ->option_text("position|number")
        ->default_str("position");
}

void describeDocument(CLI::App* command, TranslationOptions& options) {
    command
        ->add_option("--document",
                     options.document,
                     std::string{core::translate(
                         "The document to change: the main one, or the translation given by -t")})
        ->check(CLI::IsMember({"main", "translation"}))
        ->option_text("main|translation")
        ->default_str("main");
    describeTranslation(command, options);
}

ExitCode refuse(std::string_view why) {
    std::cerr << why << '\n';
    return ExitCode::Usage;
}

} // namespace subedit::cli
