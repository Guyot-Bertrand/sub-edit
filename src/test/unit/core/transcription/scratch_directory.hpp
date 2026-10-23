#pragma once

// A directory that removes itself, and the programs a test puts in it.

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace subedit::test {

class ScratchDirectory {

public:
    ScratchDirectory() {
        static int serial = 0;
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-scratch-" + std::to_string(stamp) + "-" + std::to_string(++serial));
        std::filesystem::create_directories(m_path);
    }

    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory(ScratchDirectory&&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(ScratchDirectory&&) = delete;

    ~ScratchDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

    [[nodiscard]] std::filesystem::path file(const std::string& name) const {
        return m_path / name;
    }

    /// Writes `content` to `name`.
    [[nodiscard]] std::filesystem::path write(const std::string& name,
                                              const std::string& content) const {
        const std::filesystem::path target = file(name);
        std::ofstream{target, std::ios::binary} << content;
        return target;
    }

    /// Writes an executable called `name` that runs the fake transcriber with
    /// `knobs` followed by the arguments it was given — a stand-in for
    /// `whisper-cli`, `ffprobe` or `ffmpeg`, which a test finds on a search path
    /// it controls.
    [[nodiscard]] std::filesystem::path standIn(const std::string& name,
                                                const std::filesystem::path& fake,
                                                const std::string& knobs) const {
        const std::filesystem::path script =
            write(name, "#!/bin/sh\nexec '" + fake.string() + "' " + knobs + " \"$@\"\n");
        std::filesystem::permissions(
            script, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
        return script;
    }

private:
    std::filesystem::path m_path;
};

} // namespace subedit::test
