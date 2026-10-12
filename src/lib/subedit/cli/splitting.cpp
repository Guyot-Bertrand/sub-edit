#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/disjoining.hpp>
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
#include <subedit/core/i18n/messages.hpp>
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

/// What the options that come before the cut made of the project.
struct Arranged {
    std::size_t reordered = 0;
    std::optional<core::DisjointReport> cut;
};

/// Puts the project in order when asked, then makes its ranges disjoint when
/// asked, saying so. Nothing when the ranges were asked for on a project out of
/// order: the refusal has been reported.
[[nodiscard]] std::optional<Arranged> arrangedBy(core::Session& session,
                                                 const SplitRequest& request,
                                                 const std::string& path,
                                                 const Reporter& reporter) {
    Arranged arranged;
    if (request.sort) {
        arranged.reordered = sortedIn(session);
        reporter.say(1, narrationOfSort(path, arranged.reordered));
    }
    if (request.disjoint) {
        const std::expected<core::DisjointReport, Failure> made = disjointedIn(session);
        if (!made.has_value()) {
            reportFailure(reporter, path, made.error());
            return std::nullopt;
        }
        arranged.cut = *made;
        reporter.say(1, narrationOfDisjoint(path, *made));
    }
    return arranged;
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
        reporter.failed(
            // TRANSLATORS: %1 is a file the user named.
            core::substitute(core::translate("%1: named as both the head and the tail"),
                             {head.string()}));
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
    const std::optional<Arranged> arranged = arrangedBy(session, request, path, reporter);
    if (!arranged.has_value()) {
        return ExitCode::AllFailed;
    }
    const std::size_t reordered = arranged->reordered;
    const std::optional<core::DisjointReport>& cut = arranged->cut;
    const std::size_t total = session.project().count();
    if (request.at < 2 || request.at > total) {
        reportFailure(
            reporter,
            path,
            Failure{
                "at-out-of-bounds", [&] {
                    return total < 2
                               ? core::substitute(
                                     // TRANSLATORS: %1 is a number of subtitles.
                                     core::translatePlural(
                                         "a file of %1 subtitle cannot be split: a cut needs a "
                                         "subtitle on each side",
                                         "a file of %1 subtitles cannot be split: a cut needs a "
                                         "subtitle on each side",
                                         total),
                                     {std::to_string(total)})
                               : core::substitute(
                                     // TRANSLATORS: %1 is the number given to --at, %2 how many
                                     // subtitles the file holds.
                                     core::translatePlural(
                                         "--at %1: the file holds %2 subtitle, and the tail can "
                                         "start from 2 to %2",
                                         "--at %1: the file holds %2 subtitles, and the tail can "
                                         "start from 2 to %2",
                                         total),
                                     {std::to_string(request.at), std::to_string(total)});
                }});
        return ExitCode::AllFailed;
    }

    const core::SubtitleIndex from = core::SubtitleIndex::fromValue(request.at - 1);
    std::expected<core::SplitProject, core::SplitRefusal> split =
        core::splitProject(session.project(), from);
    if (!split) {
        reportFailure(reporter, path, Failure{"split-overlap", [&] {
                                                  return core::refusalOfSplit(
                                                      request.at, split.error().before.value() + 1);
                                              }});
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
    narrateKept(
        reporter,
        path,
        opened->bytes,
        // TRANSLATORS: %1 and %2 are the numbers of bytes of the head and the tail.
        core::substitute(core::translate("%1 and %2"),
                         {std::to_string(halves[0].bytes), std::to_string(halves[1].bytes)}),
        dryRun,
        source,
        opened->diagnostics);

    const std::size_t kept = stays.count();
    const std::size_t moved = split->tail.count();
    const std::string made =
        // TRANSLATORS: %1 is a file, %2 the subtitle the tail starts at, %3 and %4 numbers of
        // subtitles (« 12 subtitles »).
        core::substitute(
            core::translate("%1: split at subtitle %2: %3 in the head, %4 in the tail"),
            {path,
             std::to_string(request.at),
             core::countOf(kept, core::Counted::Subtitle),
             core::countOf(moved, core::Counted::Subtitle)});
    std::vector<Count> counts{{"subtitles", static_cast<std::int64_t>(total)},
                              {"head", static_cast<std::int64_t>(kept)},
                              {"tail", static_cast<std::int64_t>(moved)}};
    // Only with `--sort`: a field nobody asked for would change the record.
    if (request.sort)
        counts.emplace_back("moved", static_cast<std::int64_t>(reordered));
    addCountsOfDisjoint(counts, cut);
    if (dryRun) {
        reporter.say(1, core::substitute(core::translate("%1 (dry run, nothing written)"), {made}));
        reporter.record(dryRunRecord(reporter.command(),
                                     path,
                                     counts,
                                     warningsOf(opened->diagnostics),
                                     std::nullopt,
                                     {{"tail", Json{nullptr}}}));
    } else {
        reporter.say(1,
                     // TRANSLATORS: %1 is what was done, %2 and %3 the two files written.
                     core::substitute(core::translate("%1 -> %2, %3"),
                                      {made, head.string(), tail.string()}));
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
