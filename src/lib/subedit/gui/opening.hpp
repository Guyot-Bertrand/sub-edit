#pragma once

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/model/project.hpp>

#include <expected>
#include <filesystem>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::gui {

/// A file that was read: what it holds, and what was wrong with it.
struct OpenedFile {
    core::Project project{};

    /// What the reading recovered from, in the order it was met.
    ///
    /// **Carried out rather than dropped.** Issue #128 left them behind, on the
    /// grounds that showing them was the window's business — which is true, and
    /// which is exactly why the window has to be handed them. They are the
    /// first thing of a reading a user sees other than through `-vvv`.
    std::vector<core::Diagnostic> diagnostics{};
};

/// Reads `path` and builds what a window opens on.
///
/// Ties together what the core already does separately: the bytes are read, the
/// format is recognised, and the project keeps what it takes to write the file
/// back as it was — its format among the rest, since ADR 0018.
///
/// Fails rather than opening something wrong: a file that is not readable, not
/// UTF-8, or not a subtitle file at all gives a `ReadError` and nothing opened.
[[nodiscard]] std::expected<OpenedFile, core::ReadError>
openProject(const core::FileSystem& files, const std::filesystem::path& path);

} // namespace subedit::gui
