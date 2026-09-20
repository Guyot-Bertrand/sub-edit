#pragma once

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <expected>
#include <filesystem>
#include <variant>
#include <vector>

namespace subedit::core {

/// The file asked for is the one already open as the main document.
struct SameFileAsMain {
    friend bool operator==(const SameFileAsMain&, const SameFileAsMain&) = default;
};

using TranslationError = std::variant<FileError, ReadError, SameFileAsMain>;

/// A translation file, read: the lines it holds, and what it says about itself.
struct TranslationFile {
    /// The subtitles the file holds, positions and text, in the order of the
    /// file. The text of each is in `mainText`, which is what a file is read
    /// as; attaching moves it to the translation of a subtitle.
    std::vector<Subtitle> lines{};

    /// What the file looked like — its path, format, encoding, line endings —
    /// which is what a later `Save Translation` writes back.
    SourceFile source{};

    std::vector<Diagnostic> diagnostics{};
};

/// Reads the translation file at `path`, to be laid over the main document of
/// `project` — decision D4 of the phase-11 spec.
///
/// The file is read like any other, in its own format and its own encoding.
/// **Refused when it is the main file itself**, however its path is spelled.
/// A format counted in frames is read at the frame rate of the project, unless
/// `choices` gives another.
[[nodiscard]] std::expected<TranslationFile, TranslationError>
openTranslation(const FileSystem& files,
                const Project& project,
                const std::filesystem::path& path,
                const ReadingChoices& choices = {});

} // namespace subedit::core
