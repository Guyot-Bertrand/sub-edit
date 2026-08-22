#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <system_error>

namespace subedit::core {

namespace {

/// How much a single read asks for.
///
/// Large enough that a subtitle file of a few hundred kilobytes takes a handful
/// of reads, small enough to stay off the stack's limits — it is a member of an
/// automatic array.
constexpr std::size_t kReadBlockSize = std::size_t{64} * 1024;

/// Builds the refusal, with the reason as free context.
///
/// **The reason is written in English even though nobody reads it yet.** Every
/// caller reports `reasonOf(kind)` and drops the detail, so surfacing it one
/// day should be a decision — not the day the program starts answering in two
/// languages at once. Three of them were French until the phase 6 toolchain
/// went past.
[[nodiscard]] std::unexpected<FileError>
failure(FileErrorKind kind, const std::filesystem::path& path, std::string_view reason) {
    return std::unexpected(FileError{
        .kind = kind,
        .detail = path.string() + " : " + std::string{reason},
    });
}

[[nodiscard]] std::unexpected<FileError> failure(const std::error_code& code,
                                                 const std::filesystem::path& path) {
    return failure(fileErrorKindOf(code), path, code.message());
}

} // namespace

bool RealFileSystem::exists(const std::filesystem::path& path) const {
    std::error_code code;
    // The non-throwing overload: the absence of a file is an answer, not an
    // incident.
    return std::filesystem::exists(path, code) && !code;
}

// `status` rather than `symlink_status`: it follows symbolic links, and
// `/usr/bin/ffprobe` is one on most distributions.
bool RealFileSystem::isExecutable(const std::filesystem::path& path) const {
    std::error_code code;
    const std::filesystem::file_status status = std::filesystem::status(path, code);
    if (code || !std::filesystem::is_regular_file(status))
        return false;

    constexpr std::filesystem::perms kAnyExecuteBit = std::filesystem::perms::owner_exec |
                                                      std::filesystem::perms::group_exec |
                                                      std::filesystem::perms::others_exec;
    return (status.permissions() & kAnyExecuteBit) != std::filesystem::perms::none;
}

std::expected<std::string, FileError>
RealFileSystem::readFile(const std::filesystem::path& path) const {
    std::ifstream file{path, std::ios::binary};
    if (!file)
        return failure(exists(path) ? FileErrorKind::Io : FileErrorKind::NotFound,
                       path,
                       "could not be opened for reading");

    // Read in blocks rather than through `istreambuf_iterator`, which walks the
    // stream one character at a time. The size is never asked for: `tellg`
    // answers -1 on anything that is not a plain seekable file, and a size read
    // before the content is a size that may already be wrong.
    std::string content;
    std::array<char, kReadBlockSize> block{};
    while (file.read(block.data(), block.size()) || file.gcount() > 0)
        content.append(block.data(), static_cast<std::size_t>(file.gcount()));

    if (file.bad())
        return failure(FileErrorKind::Io, path, "reading was interrupted");

    return content;
}

std::expected<void, FileError> RealFileSystem::writeFile(const std::filesystem::path& path,
                                                         std::string_view content) {
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file)
        return failure(FileErrorKind::Io, path, "could not be opened for writing");

    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    file.close();
    if (!file)
        return failure(FileErrorKind::Io, path, "writing was interrupted");

    return {};
}

std::expected<void, FileError> RealFileSystem::rename(const std::filesystem::path& from,
                                                      const std::filesystem::path& to) {
    std::error_code code;
    std::filesystem::rename(from, to, code);
    if (code)
        return failure(code, from);
    return {};
}

std::expected<void, FileError> RealFileSystem::remove(const std::filesystem::path& path) {
    std::error_code code;
    if (!std::filesystem::remove(path, code)) {
        if (code)
            return failure(code, path);
        return failure(FileErrorKind::NotFound, path, "no such file");
    }
    return {};
}

} // namespace subedit::core
