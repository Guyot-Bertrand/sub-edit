#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/source_file.hpp>

#include <expected>
#include <string>
#include <utility>

namespace subedit::core {

std::expected<OpenedFile, OpenError> openProject(const FileSystem& files,
                                                 const std::filesystem::path& path) {
    std::expected<std::string, FileError> content = files.readFile(path);
    if (!content)
        return std::unexpected(OpenError{std::move(content.error())});

    std::expected<ReadResult, ReadError> read = readSubtitles(*content);
    if (!read)
        return std::unexpected(OpenError{std::move(read.error())});

    Project project;
    project.setSubtitles(std::move(read->subtitles));
    project.setSourceFile(sourceFileOf(*read, path));

    return OpenedFile{.project = std::move(project),
                      .diagnostics = std::move(read->diagnostics),
                      .bytes = content->size()};
}

std::expected<void, FileError> saveProject(FileSystem& files,
                                           const Project& project,
                                           const std::filesystem::path& path,
                                           SubtitleFormat format) {
    const SourceFile& source = project.sourceFile();

    const WriteRequest request{
        .subtitles = project.subtitles(),
        .document = Document::Main,
        .newline = source.newline,
        .encoding = source.encoding,
        .header = source.header,
    };

    const std::string written = writeSubtitles(format, request);

    return writeAtomically(files, path, written);
}

} // namespace subedit::core
