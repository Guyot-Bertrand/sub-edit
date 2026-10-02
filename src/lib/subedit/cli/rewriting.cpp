#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording/formats.hpp>

#include <cstddef>
#include <filesystem>

namespace subedit::cli {

namespace {

/// Reads, operates, writes. Returns true when the file was written — or worked
/// out, on a dry run.
bool rewriteFile(core::FileSystem& files,
                 const Job& job,
                 const std::optional<core::Encoding>& reading,
                 bool dryRun,
                 const std::optional<Range>& range,
                 const Reporter& reporter,
                 const ChangingOperation& operation) {
    const std::string& path = job.input;
    std::expected<core::OpenedFile, core::OpenError> opened =
        reading ? core::openProject(files, path, *reading) : core::openProject(files, path);
    if (!opened) {
        reportFailure(
            reporter, path, Failure{idOf(opened.error()), std::string{reasonOf(opened.error())}});
        return false;
    }

    const core::SourceFile source = opened->project.sourceFile();
    core::Session session{std::move(opened->project)};

    // **Before the operation, and before anything is changed**: a range the file
    // does not hold is a mistake about the file, said while it is untouched.
    core::Selection selection = core::Selection::all(session.project());
    if (range) {
        std::expected<core::Selection, std::string> chosen =
            selectionOf(*range, session.project().count());
        if (!chosen) {
            reportFailure(reporter, path, Failure{"range-out-of-bounds", chosen.error()});
            return false;
        }
        selection = *std::move(chosen);
    }

    // The list is read by the dry run's text and by every JSON record.
    const OperationOutcome done = operation(
        session,
        Request{.changes = dryRun || reporter.recording(), .selection = std::move(selection)});
    if (!done) {
        reportFailure(reporter, path, done.error());
        return false;
    }

    const core::WriteRequest request{
        .subtitles = session.project().subtitles(),
        .document = core::Document::Main,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = source.header,
    };
    const std::filesystem::path& out = job.output;
    const std::expected<std::size_t, Failure> written =
        writeSubtitlesTo(files, out, source.format, request, dryRun);
    if (!written) {
        reportFailure(reporter, path, written.error());
        return false;
    }

    reporter.say(3,
                 path + ": " + std::to_string(opened->bytes) + " bytes read, " +
                     std::to_string(*written) + (dryRun ? " would be written" : " written"));
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(2,
                 path + ": " + std::string{nameOf(source.format)} + ", " + nameOf(source.encoding) +
                     ", " + std::string{nameOf(source.newline)} + " line endings kept");
    if (dryRun) {
        reporter.say(1, path + ": " + done->sentence + " (dry run, nothing written)");
        if (done->changes) {
            reporter.result(textOf(path, *done->changes));
        }
        reporter.record(dryRunRecord(reporter.command(),
                                     path,
                                     done->counts,
                                     warningsOf(opened->diagnostics),
                                     done->changes));
        return true;
    }

    reporter.say(1, path + ": " + done->sentence + " -> " + out.string());
    // The diagnostics are data: they go into the record at every level, where the
    // narration keeps them for the third.
    reporter.record(writtenRecord(reporter.command(),
                                  path,
                                  out,
                                  done->counts,
                                  warningsOf(opened->diagnostics),
                                  done->changes));
    return true;
}

} // namespace

ExitCode rewriteAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const std::optional<core::Encoding>& reading,
                    const Destination& destination,
                    const Reporter& reporter,
                    std::string_view verb,
                    const ChangingOperation& operation,
                    const std::optional<Range>& range) {
    // The extension is left alone: the format has not changed.
    const std::expected<std::vector<Job>, ExitCode> jobs =
        arrange(files, destination, paths, "", reporter);
    if (!jobs) {
        return jobs.error();
    }

    std::size_t done = 0;
    for (const Job& job : *jobs) {
        if (rewriteFile(files, job, reading, destination.isDryRun(), range, reporter, operation)) {
            ++done;
        }
    }
    return tally(reporter, verb, done, paths.size());
}

ExitCode rewriteAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const std::optional<core::Encoding>& reading,
                    const Destination& destination,
                    const Reporter& reporter,
                    std::string_view verb,
                    const Operation& operation) {
    // An operation that lists nothing is asked for nothing.
    const ChangingOperation changing = [&operation](core::Session& session, const Request&) {
        return operation(session);
    };
    return rewriteAll(files, paths, reading, destination, reporter, verb, changing);
}

} // namespace subedit::cli
