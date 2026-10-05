#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
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
    const core::WriteRequest request{
        .subtitles = half.subtitles(),
        .document = core::Document::Main,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = source.header,
    };
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
        if (!out.empty() && sameFile(files, out, path)) {
            reporter.failed(out.string() + ": would be written over the input " + path);
            return ExitCode::Usage;
        }
    }

    std::expected<core::OpenedFile, core::OpenError> opened =
        reading ? core::openProject(files, path, *reading) : core::openProject(files, path);
    if (!opened) {
        reportFailure(
            reporter, path, Failure{idOf(opened.error()), std::string{reasonOf(opened.error())}});
        return ExitCode::AllFailed;
    }

    // **A cut needs a subtitle on each side**: from the second to the last, as
    // the box of the window offers them.
    core::Session session{std::move(opened->project)};
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
    for (Half& half : halves) {
        const std::filesystem::path directory = half.out.parent_path();
        if (!dryRun && !directory.empty()) {
            if (const std::expected<void, core::FileError> made =
                    files.createDirectories(directory);
                !made) {
                reporter.failed(directory.string() + ": " +
                                std::string{core::reasonOfCreating(made.error().kind)});
                return ExitCode::AllFailed;
            }
        }
        const std::expected<std::size_t, Failure> written =
            writeHalf(files, half.out, half.project, dryRun);
        if (!written) {
            reportFailure(reporter, path, written.error());
            return ExitCode::AllFailed;
        }
        half.bytes = *written;
    }

    const core::SourceFile& source = stays.sourceFile();
    reporter.say(3,
                 path + ": " + std::to_string(opened->bytes) + " bytes read, " +
                     std::to_string(halves[0].bytes) + " and " + std::to_string(halves[1].bytes) +
                     (dryRun ? " would be written" : " written"));
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(2,
                 path + ": " + std::string{nameOf(source.format)} + ", " + nameOf(source.encoding) +
                     ", " + std::string{nameOf(source.newline)} + " line endings kept");

    const std::size_t kept = stays.count();
    const std::size_t moved = split->tail.count();
    const std::string made = path + ": split at subtitle " + std::to_string(request.at) + ": " +
                             core::countOf(kept, "subtitle") + " in the head, " +
                             core::countOf(moved, "subtitle") + " in the tail";
    const std::vector<Count> counts{{"subtitles", static_cast<std::int64_t>(total)},
                                    {"head", static_cast<std::int64_t>(kept)},
                                    {"tail", static_cast<std::int64_t>(moved)}};
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
