#pragma once

#include <string>
#include <string_view>

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

/// Returns the date of the commit the library was built from, `YYYY-MM-DD`.
///
/// Empty when the build had no repository to ask — a source archive, for
/// instance. Empty rather than a word, so that the caller decides what to say
/// about it and in which language.
///
/// The date of the commit, not of the build: two builds of one state of the
/// source then produce the same binary.
std::string_view buildDate();

} // namespace subedit::core
