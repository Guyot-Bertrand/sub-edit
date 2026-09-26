#include <subedit/core/text/pattern_paths.hpp>

#include <filesystem>
#include <string>
#include <string_view>

namespace subedit::core {

std::filesystem::path shippedPatternsPath(const std::filesystem::path& executableDirectory) {
    // `lexically_normal` so that the `..` leaves the path rather than being
    // carried all the way into the diagnostics.
    return (executableDirectory / ".." / "share" / "subedit" / "patterns").lexically_normal();
}

std::filesystem::path userPatternsPath(std::string_view xdgDataHome, std::string_view home) {
    const std::filesystem::path explicitBase{std::string{xdgDataHome}};
    if (!xdgDataHome.empty() && explicitBase.is_absolute())
        return explicitBase / "subedit" / "patterns";
    if (home.empty())
        return {};
    return std::filesystem::path{std::string{home}} / ".local" / "share" / "subedit" / "patterns";
}

} // namespace subedit::core
