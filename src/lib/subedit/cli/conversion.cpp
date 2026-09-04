#include <subedit/cli/batch.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>

namespace subedit::cli {

namespace {

using core::SubtitleFormat;

/// Converts one file. Returns true when it was written.
bool convertFile(core::FileSystem& files,
                 const std::string& path,
                 const std::optional<core::Encoding>& reading,
                 SubtitleFormat target,
                 const WriteShape& shape,
                 const Destination& destination,
                 const Reporter& reporter) {
    const std::expected<core::OpenedFile, core::OpenError> opened =
        reading ? core::openProject(files, path, *reading) : core::openProject(files, path);
    if (!opened) {
        reporter.failed(path + ": " + std::string{reasonOf(opened.error())});
        return false;
    }

    const core::SourceFile& source = opened->project.sourceFile();

    // Empty means "as the source had it": the model kept both so that a
    // conversion would not throw them away.
    const core::Newline newline = shape.newline.value_or(source.newline);
    // The source's unless another is asked for, exactly as the line ending and
    // the mark are — and the mark asked for is put on whichever of the two.
    const core::Encoding asked = shape.encoding.value_or(source.encoding);
    const core::Encoding encoding = shape.bom ? asked.withByteOrderMark(*shape.bom) : asked;

    // **`--bom` on an encoding that has none is refused, not ignored.** A byte
    // order mark exists for the Unicode encodings and for no other, so asking
    // for one on a Windows-1252 file asks for something that does not exist.
    // Writing the file without it would answer a question the user did ask.
    if (encoding.byteOrderMark() == core::ByteOrderMark::Present &&
        encoding.byteOrderMarkBytes().empty()) {
        reporter.failed(path + ": " + std::string{encoding.charset()} +
                        " has no byte order mark to write");
        return false;
    }

    const core::WriteRequest request{
        .subtitles = opened->project.subtitles(),
        .document = core::Document::Main,
        .newline = newline,
        .encoding = encoding,
        .header = source.header,
    };
    const std::filesystem::path out = destination.pathFor(path, extensionOf(target));
    const std::expected<std::size_t, std::string> written =
        writeSubtitlesTo(files, out, target, request);
    if (!written) {
        reporter.failed(path + ": " + written.error());
        return false;
    }

    reporter.say(3,
                 path + ": " + std::to_string(opened->bytes) + " bytes read, " +
                     std::to_string(*written) + " written");
    sayDiagnostics(reporter, path, opened->diagnostics);
    reporter.say(2,
                 path + ": " + std::string{nameOf(source.format)} + " -> " +
                     std::string{nameOf(target)} + ", " + std::string{encoding.charset()} + ", " +
                     (encoding.byteOrderMark() == core::ByteOrderMark::Present ? "BOM" : "no BOM") +
                     ", " + std::string{nameOf(newline)} + " line endings");
    reporter.say(1,
                 path + ": " + core::countOf(opened->project.subtitles().size(), "subtitle") +
                     " written as " + std::string{nameOf(target)} + " -> " + out.string());
    return true;
}

} // namespace

bool wouldMisname(const std::vector<std::string>& paths, SubtitleFormat target) {
    const std::string_view wanted = extensionOf(target);
    return std::ranges::any_of(paths, [wanted](const std::string& path) {
        std::string extension = std::filesystem::path{path}.extension().string();
        std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return extension != wanted;
    });
}

ExitCode convertAll(core::FileSystem& files,
                    const std::vector<std::string>& paths,
                    const std::optional<core::Encoding>& reading,
                    SubtitleFormat target,
                    const WriteShape& shape,
                    const Destination& destination,
                    const Reporter& reporter) {
    std::size_t done = 0;
    for (const std::string& path : paths) {
        if (convertFile(files, path, reading, target, shape, destination, reporter)) {
            ++done;
        }
    }
    return tally(reporter, "converted", done, paths.size());
}

} // namespace subedit::cli
