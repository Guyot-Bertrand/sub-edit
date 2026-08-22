#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

[[nodiscard]] std::unexpected<FileError> failure(FileErrorKind kind,
                                                 const std::filesystem::path& path) {
    return std::unexpected(FileError{.kind = kind, .detail = path.string()});
}

/// Consumes a pending failure, if one was armed.
[[nodiscard]] std::optional<FileErrorKind> take(std::optional<FileErrorKind>& pending) {
    const std::optional<FileErrorKind> kind = pending;
    pending.reset();
    return kind;
}

} // namespace

void InMemoryFileSystem::addFile(const std::filesystem::path& path, std::string content) {
    m_files[path] = std::move(content);
}

void InMemoryFileSystem::addExecutable(const std::filesystem::path& path) {
    // A program is a file like any other, and one a test never reads: what is
    // asked of it is that it be there and be runnable.
    m_files.try_emplace(path);
    m_executables.insert(path);
}

std::optional<std::string> InMemoryFileSystem::contentOf(const std::filesystem::path& path) const {
    const auto found = m_files.find(path);
    if (found == m_files.end())
        return std::nullopt;
    return found->second;
}

void InMemoryFileSystem::failNextRead(FileErrorKind kind) {
    m_pendingReadFailure = kind;
}

void InMemoryFileSystem::failNextWrite(FileErrorKind kind) {
    m_pendingWriteFailure = kind;
}

void InMemoryFileSystem::failNextRename(FileErrorKind kind) {
    m_pendingRenameFailure = kind;
}

bool InMemoryFileSystem::exists(const std::filesystem::path& path) const {
    return m_files.contains(path);
}

bool InMemoryFileSystem::isExecutable(const std::filesystem::path& path) const {
    return m_executables.contains(path);
}

std::expected<std::string, FileError>
InMemoryFileSystem::readFile(const std::filesystem::path& path) const {
    if (const std::optional<FileErrorKind> kind = take(m_pendingReadFailure); kind.has_value())
        return failure(*kind, path);

    const auto found = m_files.find(path);
    if (found == m_files.end())
        return failure(FileErrorKind::NotFound, path);
    return found->second;
}

std::expected<void, FileError> InMemoryFileSystem::writeFile(const std::filesystem::path& path,
                                                             std::string_view content) {
    if (const std::optional<FileErrorKind> kind = take(m_pendingWriteFailure); kind.has_value())
        return failure(*kind, path);

    m_files[path] = std::string{content};
    return {};
}

std::expected<void, FileError> InMemoryFileSystem::rename(const std::filesystem::path& from,
                                                          const std::filesystem::path& to) {
    if (const std::optional<FileErrorKind> kind = take(m_pendingRenameFailure); kind.has_value())
        return failure(*kind, from);

    const auto found = m_files.find(from);
    if (found == m_files.end())
        return failure(FileErrorKind::NotFound, from);

    m_files[to] = std::move(found->second);
    m_files.erase(found);

    // The mode travels with the file, as it does on a real system. Leaving it
    // behind would make the old name still look runnable, which is exactly the
    // lie a fake is there not to tell.
    if (m_executables.erase(from) != 0)
        m_executables.insert(to);
    return {};
}

std::expected<void, FileError> InMemoryFileSystem::remove(const std::filesystem::path& path) {
    if (m_files.erase(path) == 0)
        return failure(FileErrorKind::NotFound, path);
    m_executables.erase(path);
    return {};
}

} // namespace subedit::core
