#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

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

    /// Tells whether `path` is a file the system would agree to run.
    ///
    /// Here rather than beside the caller because looking for an external
    /// program — a video player, `ffprobe` — is a question about the file
    /// system, and asking it through this interface is what lets a test answer
    /// « the program is not installed » without uninstalling anything.
    [[nodiscard]] virtual bool isExecutable(const std::filesystem::path& path) const = 0;

    /// Returns the files lying directly in `directory`, sorted by path.
    ///
    /// **Files only, and one level only.** A caller looking for a neighbour of
    /// a file wants neighbours, and telling files from directories is a second
    /// question every one of them would have to ask.
    ///
    /// **Sorted**, because the order a device hands a directory back in is not
    /// stable — not between machines, not between two runs on one. A rule
    /// resting on « the first one » would then rest on nothing, and a test
    /// reading the list would pass or fail by luck.
    ///
    /// An empty `directory` names the current one. On a device that is where
    /// the process stands; in memory, where there is no such thing, it is the
    /// files added under no directory at all — which is the same sentence.
    ///
    /// A symbolic link to a file is a file, as everywhere else here.
    [[nodiscard]] virtual std::expected<std::vector<std::filesystem::path>, FileError>
    filesIn(const std::filesystem::path& directory) const = 0;

    [[nodiscard]] virtual std::expected<std::string, FileError>
    readFile(const std::filesystem::path& path) const = 0;

    [[nodiscard]] virtual std::expected<void, FileError>
    writeFile(const std::filesystem::path& path, std::string_view content) = 0;

    /// Makes `directory` and every missing parent above it.
    ///
    /// **Already there is a success**, and that is the point: this is asked
    /// before writing, and the directory existing is the ordinary case. A
    /// version that failed on it would make every caller write the same
    /// « unless it exists » around it.
    ///
    /// Here rather than in a caller because writing where nothing has ever
    /// been written is part of writing: the settings of a machine that has
    /// never run the program land in a directory nobody has made yet.
    [[nodiscard]] virtual std::expected<void, FileError>
    createDirectories(const std::filesystem::path& directory) = 0;

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
