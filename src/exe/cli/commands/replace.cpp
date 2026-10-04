#include "replace.hpp"

#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/replacing.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `replace` was asked for.
struct ReplaceOptions {
    std::string pattern;
    std::string replacement;
    std::vector<std::string> files;
    bool recursive = false;
    bool regex = false;
    bool caseSensitive = false;
    std::string range;
    DestinationOptions destination;
};

CLI::App* describeReplace(CLI::App& app, std::string_view name, ReplaceOptions& options) {
    CLI::App* replace = app.add_subcommand(
        std::string{name}, "Replace a text in the subtitles, without breaking a tag");
    replace->add_option("pattern", options.pattern, "Text to look for in what is shown")
        ->required();
    replace->add_option("replacement", options.replacement, "What replaces it, tags allowed")
        ->required();
    replace->add_option("files", options.files, "Subtitle files to change")->required();
    describeRecursive(replace, options.recursive);
    replace->add_flag(
        "--regex", options.regex, "Read the text to look for as a regular expression");
    replace->add_flag(
        "--case-sensitive", options.caseSensitive, "Tell capitals from small letters");
    replace->add_option("--range", options.range, "Change only subtitles N to M, or N to the end")
        ->option_text("N-M|N-");

    describeDestination(replace, options.destination);
    return replace;
}

ExitCode runReplace(const ReplaceOptions& options,
                    core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    // The pattern first: what cannot be read is refused before a file is.
    const std::expected<core::SearchPattern, std::string> pattern = compilePattern(
        options.pattern,
        core::SearchOptions{.regex = options.regex, .ignoreCase = !options.caseSensitive});
    if (!pattern) {
        return refuse(pattern.error());
    }

    std::optional<Range> range;
    if (!options.range.empty()) {
        const std::expected<Range, std::string> read = parseRange(options.range);
        if (!read) {
            return refuse("--range: " + read.error());
        }
        range = *read;
    }

    const std::expected<Inputs, std::string> inputs = expandInputs(
        files, options.files, options.recursive, options.destination.outputDir, reporter);
    if (!inputs) {
        return refuse(inputs.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, *inputs);
    if (!destination) {
        return refuse(destination.error());
    }

    return replaceIn(files,
                     inputs->paths,
                     reading,
                     *pattern,
                     options.pattern,
                     options.replacement,
                     range,
                     *destination,
                     reporter);
}

} // namespace

Declared declareReplace(CLI::App& app, std::string_view name) {
    return declareWith<ReplaceOptions>(app, name, describeReplace, runReplace);
}

} // namespace subedit::cli
