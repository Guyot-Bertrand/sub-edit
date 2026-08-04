#include <subedit/core/version.hpp>

#include <string>

namespace subedit::core {

Version version() {
    return Version{SUBEDIT_VERSION_MAJOR, SUBEDIT_VERSION_MINOR, SUBEDIT_VERSION_PATCH};
}

std::string versionString() {
    const Version current = version();
    std::string text;
    // Three numbers and two separators: reserving avoids reallocating twice.
    text.reserve(16);
    text += std::to_string(current.majorNumber);
    text += '.';
    text += std::to_string(current.minorNumber);
    text += '.';
    text += std::to_string(current.patchNumber);
    return text;
}

} // namespace subedit::core
