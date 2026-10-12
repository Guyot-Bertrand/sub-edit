#include "split_file.hpp"

#include <subedit/cli/splitting.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <CLI/CLI.hpp>
#include <optional>
#include <string>

namespace subedit::cli {

namespace {

/// What `split-file` was asked for: one file, a cut, and two outputs.
///
/// **No `--output`, `--output-dir`, `--in-place` or `--recursive`**: one input
/// makes two outputs, each named, so there is no directory to spread them in and
/// no input to write them over.
struct SplitFileOptions {
    std::string file;
    std::size_t at = 0;
    std::string head;
    std::string tail;
    bool dryRun = false;
    bool sort = false;
    bool disjoint = false;
};

CLI::App* describeSplitFile(CLI::App& app, std::string_view name, SplitFileOptions& options) {
    CLI::App* split = app.add_subcommand(
        std::string{name},
        std::string{core::translate("Cut a subtitle file in two, a head and a tail")});
    split->add_option("file", options.file, std::string{core::translate("Subtitle file to cut")})
        ->required();
    split
        ->add_option("--at",
                     options.at,
                     std::string{core::translate(
                         "Number of the first subtitle of the tail, as the table numbers them")})
        ->required()
        ->option_text("N");
    split
        ->add_option("--head",
                     options.head,
                     std::string{core::translate("File to write the subtitles before the cut to")})
        ->option_text("FILE");
    split
        ->add_option("--tail",
                     options.tail,
                     std::string{core::translate("File to write the subtitles from the cut on to")})
        ->option_text("FILE");
    split->add_flag(
        "--dry-run",
        options.dryRun,
        std::string{core::translate("Work out and say what would be written, and write nothing")});
    split->add_flag("--sort",
                    options.sort,
                    std::string{core::translate(
                        "Put the subtitles in order of their start before cutting; --at counts in "
                        "that order")});
    split->add_flag(
        "--disjoint",
        options.disjoint,
        std::string{core::translate(
            "Cut overlapping subtitles so that no range overlaps another, before cutting")});
    return split;
}

ExitCode runSplitFile(const SplitFileOptions& options,
                      core::FileSystem& files,
                      const std::optional<core::Encoding>& reading,
                      const Reporter& reporter) {
    if (!options.dryRun && (options.head.empty() || options.tail.empty())) {
        return refuse(core::translate(
            "no destination given: split-file writes two files, use --head and --tail"));
    }
    if (options.dryRun && options.head.empty() != options.tail.empty()) {
        return refuse(
            core::translate("--head and --tail go together: give both, or neither with --dry-run"));
    }

    return splitFile(files,
                     SplitRequest{.input = options.file,
                                  .at = options.at,
                                  .head = options.head,
                                  .tail = options.tail,
                                  .dryRun = options.dryRun,
                                  .sort = options.sort,
                                  .disjoint = options.disjoint},
                     reading,
                     reporter);
}

} // namespace

Declared declareSplitFile(CLI::App& app, std::string_view name) {
    return declareWith<SplitFileOptions>(app, name, describeSplitFile, runSplitFile);
}

} // namespace subedit::cli
