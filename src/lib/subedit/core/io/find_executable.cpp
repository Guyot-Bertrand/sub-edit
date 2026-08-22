#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_executable.hpp>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

namespace {

constexpr char kEntrySeparator = ':';

} // namespace

std::optional<std::filesystem::path>
findExecutable(const FileSystem& files, std::string_view name, std::string_view searchPath) {
    if (name.empty())
        return std::nullopt;

    const std::filesystem::path candidate{name};
    if (candidate.has_parent_path())
        return files.isExecutable(candidate) ? std::optional{candidate} : std::nullopt;

    for (std::size_t entryStart = 0; entryStart <= searchPath.size();) {
        const std::size_t separator = searchPath.find(kEntrySeparator, entryStart);
        const std::string_view entry = searchPath.substr(entryStart, separator - entryStart);

        if (!entry.empty()) {
            std::filesystem::path found = std::filesystem::path{entry} / candidate;
            if (files.isExecutable(found))
                return found;
        }

        if (separator == std::string_view::npos)
            break;
        entryStart = separator + 1;
    }

    return std::nullopt;
}

} // namespace subedit::core
