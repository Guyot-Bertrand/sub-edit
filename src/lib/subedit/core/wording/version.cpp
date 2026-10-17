#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/libraries.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/version.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace subedit::core {

std::string libraryVersions(const Library& library) {
    // TRANSLATORS: %1 is the version of a library the program runs against, %2 the version it
    // was compiled with — « 74.2 (built with 74.2) ».
    return substitute(translate("%1 (built with %2)"), {library.loaded, library.built});
}

std::string languagesLine(const std::vector<std::string>& languages) {
    std::string codes;
    for (const std::string& language : languages) {
        if (!codes.empty())
            codes += ", ";
        codes += language;
    }
    // TRANSLATORS: %1 is the list of the interface languages installed, by code — « en, fr ».
    return substitute(translate("Languages: %1"), {codes});
}

std::string versionReport(const std::vector<Library>& libraries,
                          const std::vector<std::string>& languages) {
    // The width of the longest name, by bytes: the names are the libraries' own, in ASCII.
    std::size_t width = 0;
    for (const Library& library : libraries)
        width = std::max(width, library.name.size());

    std::string report = "subedit " + versionString() + "\n";
    for (const Library& library : libraries) {
        report += "  " + library.name + std::string(width - library.name.size() + 2, ' ') +
                  libraryVersions(library) + "\n";
    }
    report += "  " + languagesLine(languages) + "\n";
    return report;
}

} // namespace subedit::core
