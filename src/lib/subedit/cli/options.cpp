#include <subedit/cli/frame_rate_grammar.hpp>
#include <subedit/cli/options.hpp>

#include <utility>

namespace subedit::cli {

std::expected<Destination, std::string> destinationOf(const DestinationOptions& options,
                                                      const Inputs& inputs) {
    const std::expected<Destination, std::string> destination = Destination::from(
        options.output, options.outputDir, options.inPlace, inputs.paths.size(), options.dryRun);
    if (!destination) {
        return destination;
    }
    return destination->withRoots(inputs.roots);
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
