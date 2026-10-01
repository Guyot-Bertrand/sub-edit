#include <subedit/cli/destination.hpp>
#include <subedit/cli/expansion.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <string>

namespace subedit::cli {

namespace {

/// The extensions of the formats a walk keeps — **not `.txt`**, see `expandInputs`.
constexpr std::array<std::string_view, 6> kWalkedExtensions = {
    ".srt", ".vtt", ".ssa", ".ass", ".lrc", ".sub"};

[[nodiscard]] bool isWalked(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return std::ranges::find(kWalkedExtensions, extension) != kWalkedExtensions.end();
}

/// What a walk carries from directory to directory.
struct Walk {
    const core::FileSystem& files;
    std::string_view outputDir;
    Inputs& found;
    std::size_t keptHere = 0;
    std::size_t leftAside = 0;
};

[[nodiscard]] std::expected<void, std::string> walk(Walk& state, const std::filesystem::path& dir) {
    const std::expected<std::vector<core::DirectoryEntry>, core::FileError> entries =
        state.files.entriesIn(dir);
    if (!entries) {
        return std::unexpected{dir.string() + ": " +
                               std::string{core::reasonOf(entries.error().kind)}};
    }

    for (const core::DirectoryEntry& entry : *entries) {
        const std::string name = entry.path.filename().string();
        if (name.starts_with('.')) {
            continue;
        }
        switch (entry.kind) {
        case core::EntryKind::Directory: {
            if (!state.outputDir.empty() && sameFile(state.files, entry.path, state.outputDir)) {
                break;
            }
            if (const std::expected<void, std::string> inside = walk(state, entry.path); !inside) {
                return inside;
            }
            break;
        }
        case core::EntryKind::File:
            if (isWalked(entry.path)) {
                state.found.paths.push_back(entry.path.string());
                ++state.keptHere;
            } else {
                ++state.leftAside;
            }
            break;
        case core::EntryKind::Link:
        case core::EntryKind::Other:
            break;
        }
    }
    return {};
}

} // namespace

std::expected<Inputs, std::string> expandInputs(const core::FileSystem& files,
                                                const std::vector<std::string>& given,
                                                bool recursive,
                                                std::string_view outputDir,
                                                const Reporter& reporter) {
    Inputs inputs;
    for (const std::string& path : given) {
        if (!files.isDirectory(path)) {
            inputs.paths.push_back(path);
            continue;
        }
        if (!recursive) {
            return std::unexpected{path + ": is a directory: use --recursive"};
        }

        Walk state{.files = files, .outputDir = outputDir, .found = inputs};
        if (const std::expected<void, std::string> walked = walk(state, path); !walked) {
            return std::unexpected{walked.error()};
        }
        inputs.roots.emplace_back(path);

        if (state.keptHere == 0) {
            reporter.say(1, path + ": holds no file in a format this tool walks for");
        }
        if (state.leftAside > 0) {
            reporter.say(2,
                         path + ": " + core::countOf(state.leftAside, "file") +
                             " left aside, with no extension of a known format");
        }
    }
    return inputs;
}

} // namespace subedit::cli
