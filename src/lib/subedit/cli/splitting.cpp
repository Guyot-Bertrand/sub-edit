#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/opening.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/cli/splitting.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

/// The bytes of one half, or the failure that says why they cannot be made.
/// With `dryRun` the system is not asked to take them.
[[nodiscard]] std::expected<std::size_t, Failure> writeHalf(core::FileSystem& files,
                                                            const std::filesystem::path& out,
                                                            const core::Project& half,
                                                            bool dryRun) {
    const core::SourceFile& source = half.sourceFile();
    const core::WriteRequest request =
        writeRequestOf(half.subtitles(), core::Document::Main, source);
    return writeSubtitlesTo(files, out, source.format, request, dryRun);
}

} // namespace

ExitCode splitFile(core::FileSystem& files,
                   const SplitRequest& request,
                   const std::optional<core::Encoding>& reading,
                   const Reporter& reporter) {
    const std::string& path = request.input;
    const bool dryRun = request.dryRun;
    const std::filesystem::path head = request.head;
    const std::filesystem::path tail = request.tail;

    // **Before any file is read**: what the command line says is judged on the
    // command line alone.
    if (!head.empty() && sameFile(files, head, tail)) {
        reporter.failed(head.string() + ": named as both the head and the tail");
        return ExitCode::Usage;
    }
    for (const std::filesystem::path& out : std::array{head, tail}) {
        if (!out.empty()) {
            if (const std::optional<std::string> refused =
                    overwrittenInput(files, out, std::span{&path, 1})) {
                reporter.failed(*refused);
                return ExitCode::Usage;
            }
        }
    }

    std::optional<core::OpenedFile> opened = openReporting(files, path, reading, reporter);
    if (!opened) {
        return ExitCode::AllFailed;
    }

    // **A cut needs a subtitle on each side**: from the second to the last, as
    // the box of the window offers them.
    core::Session session{std::move(opened->project)};
    // **Before the cut is judged**: `--at` counts in the order that is cut.
    std::size_t reordered = 0;
    if (request.sort) {
        reordered = sortedIn(session);
        reporter.say(1, narrationOfSort(path, reordered));
    }
    const std::size_t total = session.project().count();
    if (request.at < 2 || request.at > total) {
        reportFailure(
            reporter,
            path,
            Failure{"at-out-of-bounds",
                    total < 2 ? "a file of " + core::countOf(total, "subtitle") +
                                    " cannot be split: a cut needs a subtitle on each side"
                              : "--at " + std::to_string(request.at) + ": the file holds " +
                                    core::countOf(total, "subtitle") +
                                    ", and the tail can start from 2 to " + std::to_string(total)});
        return ExitCode::AllFailed;
    }

    const core::SubtitleIndex from = core::SubtitleIndex::fromValue(request.at - 1);
    std::expected<core::SplitProject, core::SplitRefusal> split =
        core::splitProject(session.project(), from);
    if (!split) {
        reportFailure(reporter,
                      path,
                      Failure{"split-overlap",
                              core::refusalOfSplit(request.at, split.error().before.value() + 1)});
        return ExitCode::AllFailed;
    }
    (void)session.apply(std::move(split->command));
    const core::Project& stays = session.project();

    // One loop for the two halves: the head first, and a disk that refuses the
    // tail leaves the head written.
    struct Half {
        const std::filesystem::path& out;
        const core::Project& project;
        std::size_t bytes = 0;
    };

    std::array<Half, 2> halves{Half{.out = head, .project = stays},
                               Half{.out = tail, .project = split->tail}};
    // Both directories are made before either file is written.
    if (!dryRun) {
        const std::array<std::filesystem::path, 2> outputs{head, tail};
        if (const std::optional<ExitCode> failed = createDirectoriesFor(files, outputs, reporter)) {
            return *failed;
        }
    }
    for (Half& half : halves) {
        const std::expected<std::size_t, Failure> written =
            writeHalf(files, half.out, half.project, dryRun);
        if (!written) {
            reportFailure(reporter, path, written.error());
            return ExitCode::AllFailed;
        }
        half.bytes = *written;
    }

    const core::SourceFile& source = stays.sourceFile();
    narrateKept(reporter,
                path,
                opened->bytes,
                std::to_string(halves[0].bytes) + " and " + std::to_string(halves[1].bytes),
                dryRun,
                source,
                opened->diagnostics);

    const std::size_t kept = stays.count();
    const std::size_t moved = split->tail.count();
    const std::string made = path + ": split at subtitle " + std::to_string(request.at) + ": " +
                             core::countOf(kept, "subtitle") + " in the head, " +
                             core::countOf(moved, "subtitle") + " in the tail";
    std::vector<Count> counts{{"subtitles", static_cast<std::int64_t>(total)},
                              {"head", static_cast<std::int64_t>(kept)},
                              {"tail", static_cast<std::int64_t>(moved)}};
    // Only with `--sort`: a field nobody asked for would change the record.
    if (request.sort)
        counts.emplace_back("moved", static_cast<std::int64_t>(reordered));
    if (dryRun) {
        reporter.say(1, made + " (dry run, nothing written)");
        reporter.record(dryRunRecord(reporter.command(),
                                     path,
                                     counts,
                                     warningsOf(opened->diagnostics),
                                     std::nullopt,
                                     {{"tail", Json{nullptr}}}));
    } else {
        reporter.say(1, made + " -> " + head.string() + ", " + tail.string());
        reporter.record(writtenRecord(reporter.command(),
                                      path,
                                      head,
                                      counts,
                                      warningsOf(opened->diagnostics),
                                      std::nullopt,
                                      {{"tail", Json{tail.string()}}}));
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
