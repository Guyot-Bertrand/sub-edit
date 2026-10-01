#include "transform.hpp"

#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/transforming.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `transform` was asked for.
struct TransformOptions {
    std::vector<std::string> files;
    std::string first;
    std::string last;
    DestinationOptions destination;
};

CLI::App* describeTransform(CLI::App& app, std::string_view name, TransformOptions& options) {
    CLI::App* transform = app.add_subcommand(
        std::string{name}, "Correct every position from two points known to be right");
    transform->add_option("files", options.files, "Subtitle files to transform")->required();
    transform
        ->add_option(
            "--first", options.first, "Earlier reference, as <index>=<time>: 1=00:00:01.000")
        ->required();
    transform
        ->add_option("--last", options.last, "Later reference, as <index>=<time>: 3=00:00:10.000")
        ->required();

    describeDestination(transform, options.destination);
    return transform;
}

ExitCode runTransform(const TransformOptions& options,
                      core::FileSystem& files,
                      const std::optional<core::Encoding>& reading,
                      const Reporter& reporter) {
    const std::expected<Reference, std::string> first = parseReference(options.first);
    if (!first) {
        return refuse(first.error());
    }

    const std::expected<Reference, std::string> last = parseReference(options.last);
    if (!last) {
        return refuse(last.error());
    }

    const std::expected<Transform, std::string> transform = Transform::between(*first, *last);
    if (!transform) {
        return refuse(transform.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return transformAll(files, options.files, reading, *transform, *destination, reporter);
}

} // namespace

Declared declareTransform(CLI::App& app, std::string_view name) {
    return declareWith<TransformOptions>(app, name, describeTransform, runTransform);
}

} // namespace subedit::cli
