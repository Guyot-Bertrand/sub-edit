// Which language was asked for — issue #658.

#include <subedit/core/i18n/language.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

using subedit::core::EnvironmentLookup;
using subedit::core::languageCandidates;

namespace {

using Names = std::vector<std::string>;

Names candidatesFor(const std::map<std::string, std::string, std::less<>>& variables) {
    const EnvironmentLookup environment =
        [&variables](std::string_view name) -> std::optional<std::string> {
        const auto found = variables.find(name);
        if (found == variables.end()) {
            return std::nullopt;
        }
        return found->second;
    };
    return languageCandidates(environment);
}

} // namespace

TEST_CASE("an unset environment, C and POSIX are English", "[i18n][language]") {
    CHECK(candidatesFor({}).empty());
    CHECK(candidatesFor({{"LANG", "C"}}).empty());
    CHECK(candidatesFor({{"LANG", "POSIX"}}).empty());
    CHECK(candidatesFor({{"LANG", ""}}).empty());
}

TEST_CASE("a locale gives its language and its territory", "[i18n][language]") {
    CHECK(candidatesFor({{"LANG", "fr_FR.UTF-8"}}) == Names{"fr_FR", "fr"});
    CHECK(candidatesFor({{"LANG", "fr_FR.UTF-8@euro"}}) == Names{"fr_FR", "fr"});
    CHECK(candidatesFor({{"LANG", "sr@latin"}}) == Names{"sr"});
    CHECK(candidatesFor({{"LANG", "ru"}}) == Names{"ru"});
}

TEST_CASE("the variables are read in the standard order", "[i18n][language]") {
    CHECK(candidatesFor({{"LANG", "de"}, {"LC_MESSAGES", "fr"}}) == Names{"fr"});
    CHECK(candidatesFor({{"LANG", "de"}, {"LC_MESSAGES", "fr"}, {"LC_ALL", "ru"}}) == Names{"ru"});
    CHECK(candidatesFor({{"LC_ALL", ""}, {"LANG", "de"}}) == Names{"de"});
}

TEST_CASE("LANGUAGE lists the preferences and overrides the locale", "[i18n][language]") {
    CHECK(candidatesFor({{"LANG", "de_DE.UTF-8"}, {"LANGUAGE", "fr:ru"}}) == Names{"fr", "ru"});
    CHECK(candidatesFor({{"LANG", "de_DE.UTF-8"}, {"LANGUAGE", "pt_BR:pt:fr"}}) ==
          Names{"pt_BR", "pt", "fr"});
    CHECK(candidatesFor({{"LANG", "de"}, {"LANGUAGE", "fr:fr_FR"}}) == Names{"fr", "fr_FR"});
}

TEST_CASE("LANGUAGE does nothing under the C locale, which is how English is asked for",
          "[i18n][language]") {
    CHECK(candidatesFor({{"LC_ALL", "C"}, {"LANGUAGE", "fr"}}).empty());
    CHECK(candidatesFor({{"LANGUAGE", "fr"}}).empty());
}

TEST_CASE("English ends the list: what follows it never applies", "[i18n][language]") {
    CHECK(candidatesFor({{"LANG", "fr"}, {"LANGUAGE", "en:fr"}}).empty());
    CHECK(candidatesFor({{"LANG", "fr"}, {"LANGUAGE", "ru:en_GB:fr"}}) == Names{"ru"});
    CHECK(candidatesFor({{"LANG", "en_US.UTF-8"}}).empty());
}

TEST_CASE("empty entries are skipped", "[i18n][language]") {
    CHECK(candidatesFor({{"LANG", "de"}, {"LANGUAGE", ":fr::ru:"}}) == Names{"fr", "ru"});
}

TEST_CASE("the process's environment is what the lookup reads", "[i18n][language]") {
    const auto environment = subedit::core::processEnvironment();
    REQUIRE(setenv("SUBEDIT_TEST_LANGUAGE_PROBE", "fr_FR", 1) == 0);
    CHECK(environment("SUBEDIT_TEST_LANGUAGE_PROBE") == "fr_FR");
    REQUIRE(unsetenv("SUBEDIT_TEST_LANGUAGE_PROBE") == 0);
    CHECK_FALSE(environment("SUBEDIT_TEST_LANGUAGE_PROBE").has_value());
}
