#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/translation_file.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <utility>
#include <variant>

namespace subedit::core {

std::expected<TranslationFile, TranslationError> openTranslation(const FileSystem& files,
                                                                 const Project& project,
                                                                 const std::filesystem::path& path,
                                                                 const ReadingChoices& choices) {
    // **However the path is spelled.** `film.srt`, `./film.srt` and
    // `../films/film.srt` name one file, and opening it twice would make the
    // translation a copy of the main document that saving would then write over
    // it. Compared without asking the disk, which is what the seam of the file
    // system does not offer — a link is not seen through.
    const std::optional<std::filesystem::path>& main = project.sourceFile().path;
    if (main.has_value() && main->lexically_normal() == path.lexically_normal())
        return std::unexpected(SameFileAsMain{});

    // A file counted in frames is read at the rate of the project it is laid
    // over, unless the caller says otherwise: the translation is read against
    // the same film.
    ReadingChoices reading = choices;
    if (!reading.frameRate.has_value())
        reading.frameRate = project.frameRate();

    std::expected<OpenedFile, OpenError> opened = openProject(files, path, reading);
    if (!opened.has_value())
        return std::unexpected(std::visit(
            [](auto& error) { return TranslationError{std::move(error)}; }, opened.error()));

    return TranslationFile{
        .lines = {opened->project.subtitles().begin(), opened->project.subtitles().end()},
        .source = opened->project.sourceFile(),
        .diagnostics = std::move(opened->diagnostics),
    };
}

} // namespace subedit::core
