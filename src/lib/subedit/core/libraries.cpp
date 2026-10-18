#include <subedit/core/i18n/language.hpp>
#include <subedit/core/libraries.hpp>

#include <unicode/uvernum.h>
#include <unicode/uversion.h>

#include <array>
#include <enchant.h>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The version of ICU loaded at run time, written as ICU writes it — `74.2`, trailing zeros left
/// out.
[[nodiscard]] std::string loadedIcuVersion() {
    UVersionInfo version{};
    u_getVersion(version);
    std::array<char, U_MAX_VERSION_STRING_LENGTH> written{};
    u_versionToString(version, written.data());
    return written.data();
}

} // namespace

std::vector<Library> coreLibraries() {
    return {
        {.name = "ICU", .loaded = loadedIcuVersion(), .built = U_ICU_VERSION},
        // SUBEDIT_ENCHANT_VERSION is set on this file alone, from `pkg-config`
        // (core/CMakeLists.txt).
        {.name = "Enchant", .loaded = enchant_get_version(), .built = SUBEDIT_ENCHANT_VERSION},
    };
}

std::vector<std::string> installedLanguageCodes(const std::filesystem::path& localeDirectory) {
    std::vector<std::string> codes{"en"};
    for (std::string& language : availableLanguages(localeDirectory))
        codes.push_back(std::move(language));
    return codes;
}

} // namespace subedit::core
