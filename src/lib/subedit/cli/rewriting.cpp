#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/opening.hpp>
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
#include <subedit/core/wording/translation.hpp>

#include <cstddef>
#include <filesystem>
#include <iterator>
#include <optional>
#include <vector>

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
                 const ChangingOperation& operation,
                 const std::string& mainPath,
                 const std::optional<Pairing>& pairing) {
    // What is reported is the file that is written: the translation when there
    // is one, which is also the input the destination was planned from.
    const std::string& path = job.input;
    std::optional<core::OpenedFile> opened = openReporting(files, mainPath, reading, reporter);
    if (!opened) {
        return false;
    }

    core::Session session{std::move(opened->project)};

    const core::Document document = targetOf(pairing);
    std::vector<core::Diagnostic> diagnostics = std::move(opened->diagnostics);
    std::optional<core::TranslationOutcome> alignment;
    if (pairing) {
        core::ReadingChoices choices;
        if (reading) {
            choices.encoding = *reading;
        }
        std::expected<Paired, Failure> paired = pair(files, session, *pairing, choices);
        if (!paired) {
            reportFailure(reporter, path, paired.error());
            return false;
        }
        alignment = paired->outcome;
        diagnostics.insert(diagnostics.end(),
                           std::make_move_iterator(paired->diagnostics.begin()),
                           std::make_move_iterator(paired->diagnostics.end()));
    }
    // **Of the document that is written**, after the pairing: the translation's
    // own file is only known once it is attached.
    const core::SourceFile source = session.project().sourceFile(document);

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
    OperationOutcome done = operation(session,
                                      Request{.changes = dryRun || reporter.recording(),
                                              .selection = std::move(selection),
                                              .document = document,
                                              .alignment = alignment});
    if (done && pairing && alignment) {
        done->fields.emplace_back("alignment", alignmentOf(*pairing, *alignment));
    }
    if (!done) {
        reportFailure(reporter, path, done.error());
        return false;
    }

    const core::WriteRequest request =
        writeRequestOf(session.project().subtitles(), document, source);
    const std::filesystem::path& out = job.output;
    const std::expected<std::size_t, Failure> written =
        writeSubtitlesTo(files, out, source.format, request, dryRun);
    if (!written) {
        reportFailure(reporter, path, written.error());
        return false;
    }

    narrateKept(
        reporter, path, opened->bytes, std::to_string(*written), dryRun, source, diagnostics);
    // Said once: an operation whose sentence already is the alignment (`pair`)
    // has said it at level one.
    if (alignment && done->sentence != core::noticeOf(*alignment)) {
        reporter.say(2, path + ": " + core::noticeOf(*alignment));
    }
    // What the operation went on without is data: said at level 1 whatever the
    // rest, and in every record.
    Json warnings = warningsOf(diagnostics);
    for (const Warning& warning : done->warnings) {
        reporter.say(1, path + ": " + warning.detail);
        warnings.push(Json::object().set("kind", warning.kind).set("detail", warning.detail));
    }
    if (dryRun) {
        reporter.say(1, path + ": " + done->sentence + " (dry run, nothing written)");
        if (done->changes) {
            reporter.result(textOf(path, *done->changes));
        }
        reporter.record(dryRunRecord(reporter.command(),
                                     path,
                                     done->counts,
                                     std::move(warnings),
                                     done->changes,
                                     done->fields));
        return true;
    }

    reporter.say(1, path + ": " + done->sentence + " -> " + out.string());
    // The diagnostics are data: they go into the record at every level, where the
    // narration keeps them for the third.
    reporter.record(writtenRecord(reporter.command(),
                                  path,
                                  out,
                                  done->counts,
                                  std::move(warnings),
                                  done->changes,
                                  done->fields));
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
                    const std::optional<Range>& range,
                    const std::optional<Pairing>& pairing) {
    // The extension is left alone: the format has not changed. A paired run
    // writes the translation, so it is the translation the destination is
    // planned for.
    const std::vector<std::string> written =
        pairing ? std::vector<std::string>{pairing->translation} : paths;
    const std::expected<std::vector<Job>, ExitCode> jobs =
        arrange(files, destination, written, "", reporter);
    if (!jobs) {
        return jobs.error();
    }

    std::size_t done = 0;
    for (std::size_t at = 0; at < jobs->size(); ++at) {
        const Job& job = (*jobs)[at];
        if (rewriteFile(files,
                        job,
                        reading,
                        destination.isDryRun(),
                        range,
                        reporter,
                        operation,
                        paths[at],
                        pairing)) {
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
