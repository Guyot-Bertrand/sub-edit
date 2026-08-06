#pragma once

#include <subedit/core/format/file_system.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>

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

    /// Returns the content of `path`, or nothing if it is not there.
    [[nodiscard]] std::optional<std::string> contentOf(const std::filesystem::path& path) const;

    /// Returns how many files exist — enough to notice a temporary left behind.
    [[nodiscard]] std::size_t fileCount() const { return m_files.size(); }

    /// Makes the next write fail, and only the next one.
    void failNextWrite(FileErrorKind kind);

    /// Makes the next rename fail, and only the next one.
    void failNextRename(FileErrorKind kind);

    [[nodiscard]] bool exists(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<std::string, FileError>
    readFile(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<void, FileError> writeFile(const std::filesystem::path& path,
                                                           std::string_view content) override;

    [[nodiscard]] std::expected<void, FileError> rename(const std::filesystem::path& from,
                                                        const std::filesystem::path& to) override;

    [[nodiscard]] std::expected<void, FileError> remove(const std::filesystem::path& path) override;

private:
    std::map<std::filesystem::path, std::string> m_files;

    std::optional<FileErrorKind> m_pendingWriteFailure;
    std::optional<FileErrorKind> m_pendingRenameFailure;
};

} // namespace subedit::core
