#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/formats.hpp>

#include <expected>
#include <string>
#include <utility>

namespace subedit::cli {

std::expected<std::size_t, Failure> writeSubtitlesTo(core::FileSystem& files,
                                                     const std::filesystem::path& out,
                                                     core::SubtitleFormat format,
                                                     const core::WriteRequest& request,
                                                     bool dryRun) {
    const std::expected<std::string, core::WriteError> written =
        core::writeSubtitles(format, request);
    if (!written)
        return std::unexpected(
            Failure{idOf(written.error().kind), std::string{reasonOf(written.error().kind)}});

    if (dryRun)
        return written->size();

    if (const std::expected<void, core::FileError> saved =
            core::writeAtomically(files, out, *written);
        !saved)
        return std::unexpected(
            Failure{idOf(saved.error().kind),
                    out.string() + ": " + std::string{reasonOfWriting(saved.error().kind)}});

    return written->size();
}

core::WriteRequest writeRequestOf(std::span<const core::Subtitle> subtitles,
                                  core::Document document,
                                  const core::SourceFile& source) {
    return core::WriteRequest{
        .subtitles = subtitles,
        .document = document,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = source.header,
    };
}

void narrateKept(const Reporter& reporter,
                 std::string_view path,
                 std::size_t bytesRead,
                 const std::string& written,
                 bool dryRun,
                 const core::SourceFile& source,
                 std::span<const core::Diagnostic> diagnostics) {
    const std::string name{path};
    reporter.say(3,
                 name + ": " + std::to_string(bytesRead) + " bytes read, " + written +
                     (dryRun ? " would be written" : " written"));
    sayDiagnostics(reporter, path, diagnostics);
    reporter.say(2,
                 name + ": " + std::string{nameOf(source.format)} + ", " + nameOf(source.encoding) +
                     ", " + std::string{nameOf(source.newline)} + " line endings kept");
}

} // namespace subedit::cli
