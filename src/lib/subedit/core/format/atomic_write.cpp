#include <subedit/core/format/atomic_write.hpp>
#include <subedit/core/format/file_system.hpp>

#include <expected>
#include <filesystem>
#include <string_view>

namespace subedit::core {

namespace {

/// Suffix of the temporary, next to the target so that the rename stays on the
/// same file system — a rename across devices is not atomic, and on some it is
/// not even possible.
constexpr std::string_view kTemporarySuffix = ".subedit-tmp";

[[nodiscard]] std::filesystem::path temporaryFor(const std::filesystem::path& path) {
    std::filesystem::path temporary = path;
    temporary += kTemporarySuffix;
    return temporary;
}

} // namespace

std::expected<void, FileError>
writeAtomically(FileSystem& files, const std::filesystem::path& path, std::string_view content) {
    const std::filesystem::path temporary = temporaryFor(path);

    if (const std::expected<void, FileError> written = files.writeFile(temporary, content);
        !written.has_value()) {
        // Nothing was touched but the temporary, which may not even exist.
        return written;
    }

    if (const std::expected<void, FileError> renamed = files.rename(temporary, path);
        !renamed.has_value()) {
        // The target is still the previous version. Take the temporary away so
        // that a failed save leaves nothing lying next to the file — best
        // effort: if even that fails, the rename error is the one worth
        // reporting, and the file itself is intact either way.
        [[maybe_unused]] const std::expected<void, FileError> cleaned = files.remove(temporary);
        return renamed;
    }

    return {};
}

} // namespace subedit::core
