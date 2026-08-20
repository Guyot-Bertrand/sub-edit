#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/gui/opening.hpp>

#include <expected>
#include <string>
#include <utility>

namespace subedit::gui {

std::expected<core::Project, core::ReadError> openProject(const core::FileSystem& files,
                                                          const std::filesystem::path& path) {
    const std::expected<std::string, core::FileError> content = files.readFile(path.string());
    if (!content) {
        // A file that cannot be opened and a file that cannot be understood are
        // both « nothing to show », and the window says so the same way. What
        // went wrong precisely is the file system's answer, not a reader's.
        return std::unexpected(
            core::ReadError{.kind = core::ReadErrorKind::NoSubtitleFound, .detail = path.string()});
    }

    std::expected<core::ReadResult, core::ReadError> read = core::readSubtitles(*content);
    if (!read)
        return std::unexpected(read.error());

    core::Project project;
    project.setSubtitles(std::move(read->subtitles));
    project.setSourceFile(sourceFileOf(*read, path));

    return project;
}

} // namespace subedit::gui
