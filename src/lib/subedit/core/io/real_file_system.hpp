#pragma once

#include <subedit/core/io/file_system.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace subedit::core {

/// The file system of the machine.
///
/// Files are opened in binary: a subtitle file carries its own line endings,
/// which are detected and put back as they were, and a stream translating them
/// would defeat that.
///
/// **Limit worth knowing.** `writeAtomically` guarantees that replacing a file
/// is atomic — the previous version survives an interrupted save — because a
/// rename within a directory is atomic. It does **not** guarantee durability
/// across a power cut: that would need the file and its directory to be
/// flushed to the device. Gaupol does not do it either, and the failure this
/// protects against is the common one, a save that fails halfway.
class RealFileSystem final : public FileSystem {

public:
    [[nodiscard]] bool exists(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<std::string, FileError>
    readFile(const std::filesystem::path& path) const override;

    [[nodiscard]] std::expected<void, FileError> writeFile(const std::filesystem::path& path,
                                                           std::string_view content) override;

    [[nodiscard]] std::expected<void, FileError> rename(const std::filesystem::path& from,
                                                        const std::filesystem::path& to) override;

    [[nodiscard]] std::expected<void, FileError> remove(const std::filesystem::path& path) override;
};

} // namespace subedit::core
