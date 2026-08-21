#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/saving.hpp>

#include <expected>
#include <string>

namespace subedit::gui {

std::expected<void, core::FileError> saveProject(core::FileSystem& files,
                                                 const core::Project& project,
                                                 const std::filesystem::path& path,
                                                 core::SubtitleFormat format) {
    const core::SourceFile& source = project.sourceFile();

    const core::WriteRequest request{
        .subtitles = project.subtitles(),
        .document = core::Document::Main,
        .newline = source.newline,
        .header = source.header,
    };

    const std::string written = core::writeSubtitles(
        format, request, source.hadUtf8Bom ? core::Utf8Bom::Present : core::Utf8Bom::Absent);

    return core::writeAtomically(files, path, written);
}

} // namespace subedit::gui
