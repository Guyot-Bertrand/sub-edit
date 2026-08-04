#pragma once

#include <string>

namespace subedit::core {

/// Semantic version number of the library.
struct Version {
    int majorNumber;
    int minorNumber;
    int patchNumber;

    friend bool operator==(const Version&, const Version&) = default;
};

/// Returns the version the library was built with.
Version version();

/// Returns the version formatted as `major.minor.patch`.
std::string versionString();

} // namespace subedit::core
