#include "pair.hpp"

#include <subedit/cli/translation_writing.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>

namespace subedit::cli {

namespace {

/// What `pair` was asked for: a main file, the translation to lay over it, and
/// where to write the result.
///
/// **One main file and no `--recursive`**: a pair is two files, and a tree
/// holds no way to say which translation goes with which.
struct PairOptions {
    std::string main;
    TranslationOptions translation;
    DestinationOptions destination;
};

CLI::App* describePair(CLI::App& app, std::string_view name, PairOptions& options) {
    CLI::App* pair = app.add_subcommand(
        std::string{name},
        "Write a translation at the positions of its main file, and say how it lined up");
    pair->add_option("main", options.main, "The main subtitle file")->required();
    describeTranslation(pair, options.translation);

    describeDestination(pair, options.destination);
    return pair;
}

ExitCode runPair(const PairOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter) {
    if (options.translation.file.empty()) {
        return refuse("pair needs the translation file to lay over the main one: use -t");
    }

    const std::expected<Inputs, std::string> inputs =
        expandInputs(files, {options.main}, false, options.destination.outputDir, reporter);
    if (!inputs) {
        return refuse(inputs.error());
    }

    // -t is given, so there is a pairing; `--document` does not exist here.
    const std::expected<std::optional<Pairing>, std::string> pairing =
        pairingOf(options.translation, false, *inputs);
    if (!pairing) {
        return refuse(pairing.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, *inputs);
    if (!destination) {
        return refuse(destination.error());
    }

    return writeTranslationAt(files,
                              options.main,
                              reading,
                              *destination,
                              reporter,
                              pairing->value_or(Pairing{.translation = options.translation.file}));
}

} // namespace

Declared declarePair(CLI::App& app, std::string_view name) {
    return declareWith<PairOptions>(app, name, describePair, runPair);
}

} // namespace subedit::cli
