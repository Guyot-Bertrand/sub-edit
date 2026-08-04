#include <subedit/core/version.hpp>

#include <string>

namespace subedit::core {

Version version() {
    return Version{SUBEDIT_VERSION_MAJOR, SUBEDIT_VERSION_MINOR, SUBEDIT_VERSION_PATCH};
}

std::string versionString() {
    const Version current = version();
    // No reserve() here: a version string fits in the small-string buffer, so
    // reserving would force the very heap allocation it looks like it avoids.
    return std::to_string(current.majorNumber) + '.' + std::to_string(current.minorNumber) + '.' +
           std::to_string(current.patchNumber);
}

} // namespace subedit::core
