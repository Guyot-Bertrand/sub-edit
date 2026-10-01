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

/// What an entry of a directory is, **without following a link**.
enum class EntryKind {
    File,
    Directory,
    Link,  ///< a symbolic link, whatever it points to
    Other, ///< a socket, a device, a pipe
};

/// One entry of a directory, with what it is.
struct DirectoryEntry {
    std::filesystem::path path;
    EntryKind kind;
};

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

    /// Tells whether `first` and `second` name the same file.
    ///
    /// **Not the same question as comparing two spellings**: `out/../a.srt` and
    /// `a.srt`, a symbolic link and its target, and on a case-insensitive
    /// system `A.SRT` and `a.srt` are one file under several names. A caller
    /// about to write over something asks this before it does.
    ///
    /// False when either does not exist — nothing can be the same file as a
    /// file that is not there.
    [[nodiscard]] virtual bool equivalent(const std::filesystem::path& first,
                                          const std::filesystem::path& second) const = 0;

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

    /// Returns everything lying directly in `directory`, sorted by name.
    ///
    /// **What `filesIn` does not say, and a walk needs**: directories, and links
    /// told from what they point to. A link is reported as `Link` and never
    /// followed — a tree that contains a link to one of its own parents has no
    /// end, and a walk that follows it is a defect found in production.
    ///
    /// **Sorted by the bytes of the name**, not by the locale: the order a walk
    /// writes in is then the same on every machine.
    [[nodiscard]] virtual std::expected<std::vector<DirectoryEntry>, FileError>
    entriesIn(const std::filesystem::path& directory) const = 0;

    /// Tells whether `path` is a directory, following links, as a name given
    /// on a command line is understood.
    [[nodiscard]] virtual bool isDirectory(const std::filesystem::path& path) const = 0;

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
