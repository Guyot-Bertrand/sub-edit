#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/i18n/messages.hpp>
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
            Failure{idOf(written.error().kind), [&] { return reasonOf(written.error().kind); }});

    if (dryRun)
        return written->size();

    if (const std::expected<void, core::FileError> saved =
            core::writeAtomically(files, out, *written);
        !saved)
        return std::unexpected(Failure{
            idOf(saved.error().kind), [&] {
                return reasonOfWriting(saved.error().kind);
            }}.about(out.string()));

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

void narrateRead(const Reporter& reporter,
                 std::size_t bytesRead,
                 const core::SourceFile& source,
                 std::span<const core::Diagnostic> diagnostics) {
    reporter.tell(2, Key::Format, nameOf(source.format));
    reporter.tell(2, Key::Encoding, source.encoding.charset());
    // The words of the report of `inspect`, as names: a script reads them in every language.
    reporter.tell(2,
                  Key::Bom,
                  source.encoding.byteOrderMark() == core::ByteOrderMark::Present ? "present"
                                                                                  : "absent");
    reporter.tell(2, Key::LineEndings, nameOf(source.newline));
    reporter.tell(3, Key::BytesRead, std::to_string(bytesRead));
    // The path is the element's `source` line: a diagnostic of an element does not repeat it.
    sayDiagnostics(reporter, diagnostics);
}

} // namespace subedit::cli
