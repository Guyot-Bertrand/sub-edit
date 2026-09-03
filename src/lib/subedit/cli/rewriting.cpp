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
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording.hpp>

#include <cstddef>
#include <filesystem>

namespace subedit::cli {

namespace {

/// Reads, operates, writes. Returns true when the file was written.
bool rewriteFile(core::FileSystem& files,
                 const std::string& path,
                 const Destination& destination,
                 const Reporter& reporter,
                 const Operation& operation) {
    std::expected<core::OpenedFile, core::OpenError> opened = core::openProject(files, path);
    if (!opened) {
        reporter.failed(path + ": " + std::string{reasonOf(opened.error())});
        return false;
    }

    const core::SourceFile source = opened->project.sourceFile();
    core::Session session{std::move(opened->project)};

    const std::expected<std::string, std::string> done = operation(session);
    if (!done) {
        reporter.failed(path + ": " + done.error());
        return false;
    }

    const core::WriteRequest request{
        .subtitles = session.project().subtitles(),
        .document = core::Document::Main,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = source.header,
    };
    // The extension is left alone: the format has not changed.
    const std::filesystem::path out = destination.pathFor(path, "");
    const std::expected<std::size_t, std::string> written =
        writeSubtitlesTo(files, out, source.format, request);
    if (!written) {
        reporter.failed(path + ": " + written.error());
        return false;
    }

    reporter.say(3,
                 path + ": " + std::to_string(opened->bytes) + " bytes read, " +
                     std::to_string(*written) + " written");
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(
        2,
        path + ": " + std::string{nameOf(source.format)} + ", " +
            std::string{source.encoding.charset()} + ", " +
            (source.encoding.byteOrderMark() == core::ByteOrderMark::Present ? "BOM" : "no BOM") +
            ", " + std::string{nameOf(source.newline)} + " line endings kept");
    reporter.say(1, path + ": " + *done + " -> " + out.string());
    return true;
}

} // namespace

ExitCode rewriteAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const Destination& destination,
                    const Reporter& reporter,
                    std::string_view verb,
                    const Operation& operation) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (rewriteFile(files, path, destination, reporter, operation)) {
            ++done;
        }
    }
    return tally(reporter, verb, done, paths.size());
}

} // namespace subedit::cli
