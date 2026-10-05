#include <subedit/core/text/pattern_paths.hpp>
#include <subedit/platform/locations.hpp>

#include <cstdlib>
#include <string_view>
#include <system_error>

namespace subedit::platform {

namespace {

[[nodiscard]] std::string_view variable(const char* name) {
    const char* const value = std::getenv(name);
    return value != nullptr ? std::string_view{value} : std::string_view{};
}

} // namespace

std::filesystem::path executableDirectory() {
    // Empty when the system does not say, which makes the directory empty too:
    // `read_symlink` returns an empty path on failure, and the parent of nothing
    // is nothing.
    std::filesystem::path executable;
#if defined(__linux__)
    std::error_code ignored;
    executable = std::filesystem::read_symlink("/proc/self/exe", ignored);
#endif
    return executable.parent_path();
}

std::filesystem::path installedPatternsPath() {
    return core::shippedPatternsPath(executableDirectory());
}

std::filesystem::path resolvedUserPatternsPath() {
    return core::userPatternsPath(variable("XDG_DATA_HOME"), variable("HOME"));
}

} // namespace subedit::platform
