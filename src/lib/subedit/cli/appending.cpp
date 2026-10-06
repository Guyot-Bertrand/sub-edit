#include <subedit/cli/appending.hpp>
#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/opening.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <iterator>
#include <span>
#include <utility>

namespace subedit::cli {

namespace {

/// The cost of every crossing, summed: a flag is raised if any file raised it, a
/// count adds up, and the furthest a position moved is the furthest of them all.
void accumulate(core::ConversionLoss& total, const core::ConversionLoss& loss) {
    total.ends = total.ends || loss.ends;
    total.joined += loss.joined;
    total.tags += loss.tags;
    total.header = total.header || loss.header;
    total.fields += loss.fields;
    total.precision = std::max(total.precision, loss.precision);
}

/// The posts of a loss, each as an integer — the ones `noticeOf` words, as
/// `convert` counts them, so that the sentence and the record share a source.
[[nodiscard]] std::vector<Count> countsOfLoss(const core::ConversionLoss& loss) {
    return {{"lost_ends", loss.ends ? 1 : 0},
            {"joined_lines", static_cast<std::int64_t>(loss.joined)},
            {"lost_tags", static_cast<std::int64_t>(loss.tags)},
            {"lost_header", loss.header ? 1 : 0},
            {"lost_fields", static_cast<std::int64_t>(loss.fields)},
            {"furthest_ms", loss.precision}};
}

} // namespace

ExitCode appendAll(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   const Destination& destination,
                   const Reporter& reporter) {
    const std::string& basePath = paths.front();
    const bool dryRun = destination.isDryRun();

    // **Before any file is read**: writing over one of the inputs is judged on
    // the command line alone, and `arrange` below only knows the base.
    const std::filesystem::path out = destination.pathFor(basePath, "");
    if (!out.empty()) {
        if (const std::optional<std::string> refused = overwrittenInput(files, out, paths)) {
            reporter.failed(*refused);
            return ExitCode::Usage;
        }
    }
    if (const std::expected<std::vector<Job>, ExitCode> jobs =
            arrange(files, destination, {basePath}, "", reporter);
        !jobs) {
        return jobs.error();
    }

    std::optional<core::OpenedFile> opened = openReporting(files, basePath, reading, reporter);
    if (!opened) {
        return ExitCode::AllFailed;
    }
    std::size_t bytesRead = opened->bytes;
    std::vector<core::Diagnostic> diagnostics = std::move(opened->diagnostics);
    core::Session session{std::move(opened->project)};
    const core::SourceFile source = session.project().sourceFile();

    std::size_t inserted = 0;
    core::ConversionLoss loss{};
    Json inputs = Json::array();
    for (const std::string& path : std::span{paths}.subspan(1)) {
        std::optional<core::OpenedFile> next = openReporting(files, path, reading, reporter);
        if (!next) {
            return ExitCode::AllFailed;
        }
        bytesRead += next->bytes;
        sayDiagnostics(reporter, path, next->diagnostics);
        diagnostics.insert(diagnostics.end(),
                           std::make_move_iterator(next->diagnostics.begin()),
                           std::make_move_iterator(next->diagnostics.end()));

        // Applied one by one, so that each file is shifted from the end of the
        // previous one and not from that of the base.
        core::AppendedFile appended = core::appendFile(session.project(), next->project);
        if (appended.command) {
            (void)session.apply(std::move(appended.command));
        }
        inserted += appended.inserted;
        accumulate(loss, appended.loss);

        // Never empty: a file that read holds at least one subtitle, the readers
        // refusing the rest, so there is always "appended N subtitles" to say.
        reporter.say(1,
                     path + ": " +
                         core::noticeOfAppend(appended.inserted,
                                              appended.loss,
                                              next->project.sourceFile().format,
                                              source.format));

        std::vector<Count> counts = countsOfLoss(appended.loss);
        counts.insert(counts.begin(),
                      Count{"appended", static_cast<std::int64_t>(appended.inserted)});
        inputs.push(Json::object().set("file", path).set("counts", countsOf(counts)));
    }

    const core::WriteRequest request =
        writeRequestOf(session.project().subtitles(), core::Document::Main, source);
    const std::expected<std::size_t, Failure> written =
        writeSubtitlesTo(files, out, source.format, request, dryRun);
    if (!written) {
        reportFailure(reporter, basePath, written.error());
        return ExitCode::AllFailed;
    }

    narrateKept(
        reporter, basePath, bytesRead, std::to_string(*written), dryRun, source, diagnostics);

    const std::size_t total = session.project().count();
    const std::string made = basePath + ": " + core::countOf(total, "subtitle") + " from " +
                             core::countOf(paths.size(), "file");
    std::vector<Count> counts{{"files", static_cast<std::int64_t>(paths.size())},
                              {"subtitles", static_cast<std::int64_t>(total)},
                              {"appended", static_cast<std::int64_t>(inserted)}};
    for (Count& count : countsOfLoss(loss)) {
        counts.push_back(std::move(count));
    }
    Fields fields;
    fields.emplace_back("inputs", std::move(inputs));

    if (dryRun) {
        reporter.say(1, made + " (dry run, nothing written)");
        reporter.record(dryRunRecord(
            reporter.command(), basePath, counts, warningsOf(diagnostics), std::nullopt, fields));
    } else {
        reporter.say(1, made + " -> " + out.string());
        reporter.record(writtenRecord(reporter.command(),
                                      basePath,
                                      out,
                                      counts,
                                      warningsOf(diagnostics),
                                      std::nullopt,
                                      fields));
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
