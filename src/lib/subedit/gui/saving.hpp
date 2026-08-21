#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <expected>
#include <filesystem>

namespace subedit::core {
class Project;
} // namespace subedit::core

namespace subedit::gui {

/// Writes `project` to `path`, as a file of `format`.
///
/// **Puts the file back as it was found**: the line endings, the byte order
/// mark and the header are those the project came with, so that a file opened
/// and saved without a change is identical byte for byte. Anything else would
/// show a diff on every line where the user expected one corrected subtitle.
///
/// `format` is given rather than read from the project because « save as » may
/// change it; saving plainly passes the one the project already carries.
///
/// The writing is atomic — a save interrupted at any point leaves the previous
/// version exactly as it was, which is `writeAtomically`'s whole purpose.
///
/// **Neither the project nor the session is touched.** Recording that a
/// document was saved is `Session::markSaved`, and moving it is
/// `Session::setSourceFile`; both belong to whoever holds the session.
[[nodiscard]] std::expected<void, core::FileError> saveProject(core::FileSystem& files,
                                                               const core::Project& project,
                                                               const std::filesystem::path& path,
                                                               core::SubtitleFormat format);

} // namespace subedit::gui
