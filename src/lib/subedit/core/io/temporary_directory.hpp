#pragma once

#include <filesystem>

namespace subedit::core {

/// A directory that goes away with the object, so that a failure leaves nothing.
///
/// **Made by `mkdtemp`**: created in one step, readable by the user alone, and
/// refused if it already exists. A name worked out from the clock would be
/// guessable, in a temporary directory that other users share, and a directory
/// already there could be somebody else's. `made` is false, and `path` empty,
/// when it could not be made.
class TemporaryDirectory {

public:
    /// `stem` begins the name: `subedit-whisper` makes `subedit-whisper-XXXXXX`.
    explicit TemporaryDirectory(const char* stem);

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory(TemporaryDirectory&&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(TemporaryDirectory&&) = delete;

    ~TemporaryDirectory();

    [[nodiscard]] bool made() const { return !m_path.empty(); }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

} // namespace subedit::core
