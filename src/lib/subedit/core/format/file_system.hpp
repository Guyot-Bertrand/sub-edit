#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace subedit::core {

/// Why an operation on a file did not happen.
enum class FileErrorKind {
    NotFound,
    PermissionDenied,
    Io, ///< anything the system refused for another reason
};

struct FileError {
    FileErrorKind kind;

    /// Free context. May be empty.
    std::string detail;

    friend bool operator==(const FileError&, const FileError&) = default;
};

/// Maps what the system said onto the three cases a caller can act upon.
///
/// Shared rather than repeated: every implementation talking to a real device
/// has the same mapping to make, and doing it once means it can be tested
/// against the error codes themselves instead of against a device that has to
/// be made to fail.
[[nodiscard]] constexpr FileErrorKind fileErrorKindOf(const std::error_code& code) {
    if (code == std::errc::no_such_file_or_directory)
        return FileErrorKind::NotFound;
    if (code == std::errc::permission_denied)
        return FileErrorKind::PermissionDenied;
    return FileErrorKind::Io;
}

/// The primitive operations on files, and nothing more.
///
/// One of the five points where the project knows the variation is real: a
/// test needs to read and write without a disk, without a temporary directory,
/// and without anything left behind when it fails halfway.
///
/// Deliberately primitive. Writing safely is a **policy**, not a primitive:
/// `writeAtomically` builds it on top of these operations so that the policy
/// itself is written once and tested, instead of being reimplemented — and
/// possibly faked — by each implementation.
class FileSystem {

public:
    virtual ~FileSystem() = default;

    [[nodiscard]] virtual bool exists(const std::filesystem::path& path) const = 0;

    [[nodiscard]] virtual std::expected<std::string, FileError>
    readFile(const std::filesystem::path& path) const = 0;

    [[nodiscard]] virtual std::expected<void, FileError>
    writeFile(const std::filesystem::path& path, std::string_view content) = 0;

    /// Moves `from` onto `to`, replacing `to` if it exists.
    [[nodiscard]] virtual std::expected<void, FileError>
    rename(const std::filesystem::path& from, const std::filesystem::path& to) = 0;

    [[nodiscard]] virtual std::expected<void, FileError>
    remove(const std::filesystem::path& path) = 0;

protected:
    FileSystem() = default;
    FileSystem(const FileSystem&) = default;
    FileSystem(FileSystem&&) = default;
    FileSystem& operator=(const FileSystem&) = default;
    FileSystem& operator=(FileSystem&&) = default;
};

} // namespace subedit::core
