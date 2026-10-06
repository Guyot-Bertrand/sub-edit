#include "command.hpp"

#include <subedit/cli/frame_rate_grammar.hpp>

#include <CLI/CLI.hpp>
#include <iostream>

namespace subedit::cli {

void describeDestination(CLI::App* command, DestinationOptions& options) {
    command->add_option("--output", options.output, "File to write, for a single input");
    command->add_option("--output-dir", options.outputDir, "Directory to write into");
    command->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    command->add_flag(
        "--dry-run", options.dryRun, "Work out and say what would be written, and write nothing");
}

std::expected<Destination, std::string> destinationOf(const DestinationOptions& options,
                                                      const Inputs& inputs) {
    const std::expected<Destination, std::string> destination = Destination::from(
        options.output, options.outputDir, options.inPlace, inputs.paths.size(), options.dryRun);
    if (!destination) {
        return destination;
    }
    return destination->withRoots(inputs.roots);
}

void describeRecursive(CLI::App* command, bool& recursive) {
    command->add_flag(
        "-r,--recursive", recursive, "Take directories as inputs, and every subtitle file in them");
}

void describeRange(CLI::App* command, std::string& range) {
    command->add_option("--range", range, "Act only on subtitles N to M, or N to the end")
        ->option_text("N-M|N-");
}

std::expected<std::optional<Range>, std::string> rangeOf(const std::string& range) {
    if (range.empty()) {
        return std::optional<Range>{};
    }
    const std::expected<Range, std::string> read = parseRange(range);
    if (!read) {
        return std::unexpected{"--range: " + read.error()};
    }
    return std::optional{*read};
}

void describeTranslation(CLI::App* command, TranslationOptions& options) {
    command
        ->add_option("-t,--translation-file",
                     options.file,
                     "Translation file to lay over the subtitle file, for a single input")
        ->option_text("FILE");
    command
        ->add_option("--align-method",
                     options.alignMethod,
                     "How the lines of the translation find their subtitles")
        ->check(CLI::IsMember({"position", "number"}))
        ->option_text("position|number")
        ->default_str("position");
}

void describeDocument(CLI::App* command, TranslationOptions& options) {
    command
        ->add_option("--document",
                     options.document,
                     "The document to change: the main one, or the translation given by -t")
        ->check(CLI::IsMember({"main", "translation"}))
        ->option_text("main|translation")
        ->default_str("main");
    describeTranslation(command, options);
}

std::expected<std::optional<Pairing>, std::string>
pairingOf(const TranslationOptions& options, bool withDocument, const Inputs& inputs) {
    const bool translation = options.document == "translation";
    const bool given = !options.file.empty();

    if (withDocument && translation && !given) {
        return std::unexpected{"--document translation needs the translation file: use -t"};
    }
    if (withDocument && given && !translation) {
        return std::unexpected{"-t names a translation to change: use --document translation"};
    }
    if (!given) {
        if (!options.alignMethod.empty()) {
            return std::unexpected{"--align-method needs a translation file: use -t"};
        }
        return std::optional<Pairing>{};
    }
    if (inputs.paths.size() > 1 || !inputs.roots.empty()) {
        return std::unexpected{
            "-t names one file but several inputs were given: use one invocation per pair"};
    }

    return std::optional{Pairing{.translation = options.file,
                                 .method = options.alignMethod == "number"
                                               ? core::TranslationMethod::Number
                                               : core::TranslationMethod::Position}};
}

namespace {

/// The common part of `prepare`, expanding the inputs against `outputDir`.
[[nodiscard]] std::expected<Prepared, std::string> read(const core::FileSystem& files,
                                                        const Reporter& reporter,
                                                        const Preparation& wanted,
                                                        const std::string& outputDir) {
    Prepared prepared;

    if (wanted.range != nullptr) {
        std::expected<std::optional<Range>, std::string> range = rangeOf(*wanted.range);
        if (!range) {
            return std::unexpected{range.error()};
        }
        prepared.range = *std::move(range);
    }

    std::expected<Inputs, std::string> inputs =
        expandInputs(files, wanted.files, wanted.recursive, outputDir, reporter);
    if (!inputs) {
        return std::unexpected{inputs.error()};
    }
    prepared.inputs = *std::move(inputs);

    if (wanted.translation != nullptr) {
        std::expected<std::optional<Pairing>, std::string> pairing =
            pairingOf(*wanted.translation, wanted.withDocument, prepared.inputs);
        if (!pairing) {
            return std::unexpected{pairing.error()};
        }
        prepared.pairing = *std::move(pairing);
    }
    return prepared;
}

} // namespace

std::expected<Prepared, std::string>
prepare(const core::FileSystem& files, const Reporter& reporter, const Preparation& wanted) {
    return read(files, reporter, wanted, "");
}

std::expected<PreparedWriting, std::string> prepareWriting(const core::FileSystem& files,
                                                           const Reporter& reporter,
                                                           const Preparation& wanted,
                                                           const DestinationOptions& options) {
    std::expected<Prepared, std::string> prepared =
        read(files, reporter, wanted, options.outputDir);
    if (!prepared) {
        return std::unexpected{prepared.error()};
    }

    std::expected<Destination, std::string> destination = destinationOf(options, prepared->inputs);
    if (!destination) {
        return std::unexpected{destination.error()};
    }
    return PreparedWriting{.inputs = std::move(prepared->inputs),
                           .range = prepared->range,
                           .pairing = std::move(prepared->pairing),
                           .destination = *std::move(destination)};
}

ExitCode refuse(std::string_view why) {
    std::cerr << why << '\n';
    return ExitCode::Usage;
}

std::expected<core::ReadingChoices, std::string>
readingWith(const std::optional<core::Encoding>& encoding, const std::string& frameRate) {
    core::ReadingChoices choices{.encoding = encoding};
    if (frameRate.empty())
        return choices;

    const std::expected<core::FrameRate, std::string> rate = parseFrameRate(frameRate);
    if (!rate.has_value())
        return std::unexpected("--frame-rate: " + rate.error());
    choices.frameRate = *rate;
    return choices;
}

} // namespace subedit::cli
