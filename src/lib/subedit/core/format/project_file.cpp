#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/save_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

/// The recipe itself, once the bytes are in hand and the reading has happened.
///
/// The two `openProject` differ by one call — one proposes an encoding, the
/// other is given one — and everything after it is the same work.
[[nodiscard]] std::expected<OpenedFile, OpenError>
projectOf(std::expected<ReadResult, ReadError> read,
          const std::filesystem::path& path,
          std::size_t bytes) {
    if (!read)
        return std::unexpected(OpenError{std::move(read.error())});

    Project project;
    project.setSubtitles(std::move(read->subtitles));
    project.setSourceFile(sourceFileOf(*read, path));

    // **A document counted in frames works against the rate it was read at.**
    // Its positions were computed from its frames with that number, so it is
    // the one a conversion opens on and the one writing it back uses. The eight
    // other formats count in time and leave the project on its default: what a
    // time-based file was timed against is something no reader can know.
    if (const auto* frames = std::get_if<MicroDvdFile>(&project.sourceFile().extras))
        project.setFrameRate(frames->rate);

    return OpenedFile{
        .project = std::move(project), .diagnostics = std::move(read->diagnostics), .bytes = bytes};
}

} // namespace

std::expected<OpenedFile, OpenError>
openProject(const FileSystem& files, const std::filesystem::path& path, const Encoding& encoding) {
    std::expected<std::string, FileError> content = files.readFile(path);
    if (!content)
        return std::unexpected(OpenError{std::move(content.error())});

    return projectOf(readSubtitles(*content, encoding), path, content->size());
}

std::expected<OpenedFile, OpenError> openProject(const FileSystem& files,
                                                 const std::filesystem::path& path) {
    std::expected<std::string, FileError> content = files.readFile(path);
    if (!content)
        return std::unexpected(OpenError{std::move(content.error())});

    return projectOf(readSubtitles(*content), path, content->size());
}

std::expected<OpenedFile, OpenError> openProject(const FileSystem& files,
                                                 const std::filesystem::path& path,
                                                 const ReadingChoices& choices) {
    const std::expected<std::string, FileError> content = files.readFile(path);
    if (!content.has_value())
        return std::unexpected(OpenError{content.error()});

    return projectOf(readSubtitles(*content, choices), path, content->size());
}

ConvertedProject convertProjectFor(const Project& project, SubtitleFormat target, FrameRate rate) {
    const SourceFile& source = project.sourceFile();
    const std::span<const Subtitle> held = project.subtitles();

    // **Filled field by field rather than in one aggregate, and the reason is
    // measured.** Building the header string inside the initialiser makes gcov
    // attribute that line the construction's exception exit, which it then
    // counts as never taken — for a line that runs on every call. Written as an
    // assignment, it counts.
    ConvertedProject converted;
    converted.subtitles.assign(held.begin(), held.end());
    converted.header = headerFor(source, target);
    converted.extras = extrasFor(source, target);

    // **A file written in frames needs a rate whatever it came from.** What the
    // document declared crosses only into its own format, so a MicroDVD arrived
    // at from anywhere else has nothing to say about frames until this line.
    if (target == SubtitleFormat::MicroDvd &&
        !std::holds_alternative<MicroDvdFile>(converted.extras)) {
        converted.extras = MicroDvdFile{.rate = rate};
    }

    converted.loss = convertFor(converted.subtitles, source, target, rate);
    return converted;
}

std::expected<void, SaveError> saveProject(FileSystem& files,
                                           const Project& project,
                                           const std::filesystem::path& path,
                                           SubtitleFormat format) {
    const SourceFile& source = project.sourceFile();
    const ConvertedProject converted = convertProjectFor(project, format, project.frameRate());

    const WriteRequest request{
        .subtitles = converted.subtitles,
        .document = Document::Main,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = converted.header,
        .extras = converted.extras,
    };

    const std::expected<std::string, WriteError> written = writeSubtitles(format, request);
    if (!written.has_value())
        return std::unexpected(SaveError{written.error()});

    if (const std::expected<void, FileError> saved = writeAtomically(files, path, *written);
        !saved.has_value())
        return std::unexpected(SaveError{saved.error()});

    return {};
}

} // namespace subedit::core
