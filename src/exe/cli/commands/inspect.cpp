#include "inspect.hpp"

#include <subedit/cli/inspection.hpp>

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `inspect` was asked for.
struct InspectOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string frameRate;
    TranslationOptions translation;
};

CLI::App* describeInspect(CLI::App& app, std::string_view name, InspectOptions& options) {
    CLI::App* inspect =
        app.add_subcommand(std::string{name}, "Report what a subtitle file is made of");
    inspect->add_option("files", options.files, "Subtitle files to report on")->required();
    describeRecursive(inspect, options.recursive);
    // **Reading, here, and writing too on `convert`** — one rate, one option,
    // spelled the same on both. It is not global as `--encoding` is, because a
    // global option has to come before the subcommand: `--frame-rate` would
    // then be one word on `inspect` and another place entirely on `convert`,
    // where it also governs what is written.
    inspect
        ->add_option(
            "--frame-rate", options.frameRate, "Frame rate of a file counted in frames: 25, 23.976")
        ->option_text("RATE");

    describeTranslation(inspect, options.translation);

    // `--order-report` lived here, offering both readings of disorder while
    // real files settled the question. They did not — none of the corpus is out
    // of order — so it was settled by reasoning, and the option is gone: the
    // report names what breaks the order, because that is the subtitle there is
    // something to do about.
    return inspect;
}

ExitCode runInspect(const InspectOptions& options,
                    const core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    const std::expected<core::ReadingChoices, std::string> choices =
        readingWith(reading, options.frameRate);
    if (!choices) {
        return refuse(choices.error());
    }

    const std::expected<Inputs, std::string> inputs =
        expandInputs(files, options.files, options.recursive, "", reporter);
    if (!inputs) {
        return refuse(inputs.error());
    }

    const std::expected<std::optional<Pairing>, std::string> pairing =
        pairingOf(options.translation, false, *inputs);
    if (!pairing) {
        return refuse(pairing.error());
    }

    return inspectAll(files, inputs->paths, *choices, std::cout, reporter, *pairing);
}

} // namespace

Declared declareInspect(CLI::App& app, std::string_view name) {
    return declareWith<InspectOptions>(app, name, describeInspect, runInspect);
}

} // namespace subedit::cli
