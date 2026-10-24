#pragma once

#include <subedit/core/io/file_system.hpp>

#include <expected>
#include <filesystem>
#include <string_view>

namespace subedit::core {

/// Writes `content` to `path` without ever leaving it half written.
///
/// Writes to a temporary next to the target, then renames it over. A save
/// interrupted at any point leaves the previous version of the file exactly as
/// it was — losing an hour of work to a full disk is the failure this exists to
/// prevent, and it is the one Gaupol guards against the same way.
///
/// The temporary is removed if the rename fails, so that a failed save leaves
/// nothing lying next to the file either.
[[nodiscard]] std::expected<void, FileError>
writeAtomically(FileSystem& files, const std::filesystem::path& path, std::string_view content);

/// Makes `directory` and every missing parent, and says whether it is there afterwards.
///
/// For a caller that makes room before writing and has nothing to do about a refusal: the
/// write that follows says why it could not.
bool ensureDirectory(FileSystem& files, const std::filesystem::path& directory);

} // namespace subedit::core
