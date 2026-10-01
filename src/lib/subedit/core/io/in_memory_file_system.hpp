#pragma once

#include <subedit/core/io/file_system.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A file system that lives entirely in memory.
///
/// Ships with the library rather than with the tests: every module that
/// touches files needs it, and a failure it can be told to produce on demand
/// is the only honest way to test what happens when a save is interrupted.
/// Waiting for a real full disk is not a test.
class InMemoryFileSystem final : public FileSystem {

public:
    /// Puts a file there, as if it had always been.
    void addFile(const std::filesystem::path& path, std::string content);

    /// Puts an executable file there, with no content worth reading.
    ///
    /// What a fake `PATH` is made of: a test looking for an external program
    /// lays out the directories it wants, puts the program in one of them —
    /// or in none, which is the interesting case — and never touches the
    /// machine it runs on.
    void addExecutable(const std::filesystem::path& path);

    /// Returns the content of `path`, or nothing if it is not there.
    [[nodiscard]] std::optional<std::string> contentOf(const std::filesystem::path& path) const;

    /// Returns how many files exist — enough to notice a temporary left behind.
    [[nodiscard]] std::size_t fileCount() const { return m_files.size(); }

    /// Makes the next read fail, and only the next one.
    ///
    /// Same reason as the two below: a caller that reports "permission
    /// denied" differently from "not found" can only be tested against a
    /// device that refuses on demand, and waiting for a real one is not a test.
    void failNextRead(FileErrorKind kind);

    /// Makes the next write fail, and only the next one.
    void failNextWrite(FileErrorKind kind);

    /// Makes the next rename fail, and only the next one.
    void failNextRename(FileErrorKind kind);

    /// Makes the next creation of a directory fail, and only the next one.
    void failNextCreateDirectories(FileErrorKind kind);

    /// Puts a symbolic link there, to nothing worth following: it is listed as a
    /// `Link` by `entriesIn`, which is all a walk needs to be told to skip it.
    void addLink(const std::filesystem::path& path);

    /// Makes listing `directory` fail, every time.
    void failEntriesOf(const std::filesystem::path& directory, FileErrorKind kind);

    /// The directories asked for, in order — this file system makes none, so
    /// that a test can still see that one was requested.
    [[nodiscard]] const std::vector<std::filesystem::path>& directoriesAsked() const {
        return m_directoriesAsked;
    }

    [[nodiscard]] bool exists(const std::filesystem::path& path) const override;

    /// Two spellings of one path, both present: this file system has no links
    /// and no case folding, so a normalised path is a file's whole identity.
    [[nodiscard]] bool equivalent(const std::filesystem::path& first,
                                  const std::filesystem::path& second) const override;

    [[nodiscard]] bool isExecutable(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<std::vector<std::filesystem::path>, FileError>
    filesIn(const std::filesystem::path& directory) const override;

    /// Files and the directories the files imply; no links, no other kinds.
    [[nodiscard]] std::expected<std::vector<DirectoryEntry>, FileError>
    entriesIn(const std::filesystem::path& directory) const override;

    [[nodiscard]] bool isDirectory(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<std::string, FileError>
    readFile(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<void, FileError> writeFile(const std::filesystem::path& path,
                                                           std::string_view content) override;

    /// **A success unless told otherwise, and it creates nothing.** This file system has no
    /// directories: a path is a key, and its parent exists as soon as a file
    /// names it. Refusing here would force every test that writes to build a
    /// tree that does not exist.
    [[nodiscard]] std::expected<void, FileError>
    createDirectories(const std::filesystem::path& directory) override;

    [[nodiscard]] std::expected<void, FileError> rename(const std::filesystem::path& from,
                                                        const std::filesystem::path& to) override;

    [[nodiscard]] std::expected<void, FileError> remove(const std::filesystem::path& path) override;

private:
    std::map<std::filesystem::path, std::string> m_files;

    /// Which of them the system would agree to run. A second container rather
    /// than a field on the entry: nothing but the search for a program reads
    /// it, and every other test would have to name a mode it does not care
    /// about.
    std::set<std::filesystem::path> m_executables;

    mutable std::optional<FileErrorKind> m_pendingReadFailure;
    std::optional<FileErrorKind> m_pendingWriteFailure;
    std::optional<FileErrorKind> m_pendingRenameFailure;
    std::optional<FileErrorKind> m_pendingCreateFailure;
    std::vector<std::filesystem::path> m_directoriesAsked;
    std::set<std::filesystem::path> m_links;
    std::map<std::filesystem::path, FileErrorKind> m_entriesFailures;
};

} // namespace subedit::core
