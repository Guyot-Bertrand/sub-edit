#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/model/project.hpp>

#include <expected>
#include <filesystem>

namespace subedit::core {
class FileSystem;
}

namespace subedit::gui {

/// Reads `path` and builds the project a window opens on.
///
/// Ties together what the core already does separately: the bytes are read, the
/// format is recognised, and the project keeps what it takes to write the file
/// back as it was — its format among the rest, since ADR 0018.
///
/// Fails rather than opening something wrong: a file that is not readable, not
/// UTF-8, or not a subtitle file at all gives a `ReadError` and no project. The
/// diagnostics a successful reading collected are **not** returned here; they
/// belong to the reading, and showing them is the window's business.
[[nodiscard]] std::expected<core::Project, core::ReadError>
openProject(const core::FileSystem& files, const std::filesystem::path& path);

} // namespace subedit::gui
