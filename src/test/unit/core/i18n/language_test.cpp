// Which language was asked for — issue #658.

#include <subedit/core/i18n/language.hpp>
#include <subedit/core/libraries.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

using subedit::core::availableLanguages;
using subedit::core::cataloguePath;
using subedit::core::chosenLanguage;
using subedit::core::endonymOf;
using subedit::core::EnvironmentLookup;
using subedit::core::installedCatalogue;
using subedit::core::installedLanguage;
using subedit::core::installedLanguageCodes;
using subedit::core::isLanguageName;
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

/// A locale directory of the test's own, laid out as an installation is, removed
/// when the test ends.
class LocaleDirectory {
public:
    LocaleDirectory()
        : m_path{std::filesystem::temp_directory_path() /
                 ("subedit-locale-test-" + std::to_string(::getpid()))} {
        std::filesystem::remove_all(m_path);
        std::filesystem::create_directories(m_path);
    }

    LocaleDirectory(const LocaleDirectory&) = delete;
    LocaleDirectory& operator=(const LocaleDirectory&) = delete;
    LocaleDirectory(LocaleDirectory&&) = delete;
    LocaleDirectory& operator=(LocaleDirectory&&) = delete;

    ~LocaleDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    /// Puts the compiled fixture `fixture` (`ru.mo`) where `language` is looked for.
    void install(const std::string& language, const std::string& fixture) const {
        const std::filesystem::path target = cataloguePath(m_path, language);
        std::filesystem::create_directories(target.parent_path());
        std::filesystem::copy_file(std::filesystem::path{SUBEDIT_COMPILED_I18N_DIR} / fixture,
                                   target);
    }

    /// Puts bytes that are no catalogue where `language` is looked for.
    void damage(const std::string& language) const {
        const std::filesystem::path target = cataloguePath(m_path, language);
        std::filesystem::create_directories(target.parent_path());
        std::ofstream{target} << "not a catalogue";
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

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

TEST_CASE("a catalogue is looked for where gettext puts it", "[i18n][language]") {
    CHECK(cataloguePath("/usr/share/subedit/locale", "pt_BR") ==
          std::filesystem::path{"/usr/share/subedit/locale/pt_BR/LC_MESSAGES/subedit.mo"});
}

TEST_CASE("the first candidate that is installed is the one read", "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");
    locale.install("fr", "fr.mo");

    // `de` is not installed, and is passed over; `ru` comes before `fr`.
    const auto catalogue = installedCatalogue({"de_DE", "de", "ru", "fr"}, locale.path());
    REQUIRE(catalogue != nullptr);
    CHECK(catalogue->findPlural("%1 subtitle", 5) == "%1 субтитров");
}

TEST_CASE("the language found says which candidate it was found under", "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");

    const auto found = installedLanguage({"de_DE", "de", "ru"}, locale.path());
    // Read through `if`, which is what the optional-access check understands.
    std::string name;
    std::shared_ptr<const subedit::core::Catalogue> catalogue;
    if (found) {
        name = found->name;
        catalogue = found->catalogue;
    }
    CHECK(name == "ru");
    REQUIRE(catalogue != nullptr);
    CHECK(catalogue->findPlural("%1 subtitle", 5) == "%1 субтитров");

    CHECK_FALSE(installedLanguage({"de"}, locale.path()).has_value());
}

TEST_CASE("no installed candidate, or none at all, is English and says nothing",
          "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");

    CHECK(installedCatalogue({"de", "xx"}, locale.path()) == nullptr);
    CHECK(installedCatalogue({}, locale.path()) == nullptr);
    CHECK(installedCatalogue({"ru"}, locale.path() / "absent") == nullptr);
}

TEST_CASE("a catalogue that does not read is passed over like a missing one", "[i18n][language]") {
    const LocaleDirectory locale;
    locale.damage("fr");
    locale.install("ru", "ru.mo");

    CHECK(installedCatalogue({"fr"}, locale.path()) == nullptr);
    const auto catalogue = installedCatalogue({"fr", "ru"}, locale.path());
    REQUIRE(catalogue != nullptr);
    CHECK(catalogue->findPlural("%1 subtitle", 1) == "%1 субтитр");
}

// ## The setting `interface.language` — issue #669

namespace {

/// An environment of the test's own: `variables`, and nothing else.
EnvironmentLookup environmentOf(std::map<std::string, std::string, std::less<>> variables) {
    return [variables = std::move(variables)](std::string_view name) -> std::optional<std::string> {
        const auto found = variables.find(name);
        if (found == variables.end())
            return std::nullopt;
        return found->second;
    };
}

/// The name of the language `chosenLanguage` settles on, empty for English.
std::string chosenName(const std::optional<std::string>& preferred,
                       const EnvironmentLookup& environment,
                       const std::filesystem::path& locale) {
    const auto found = chosenLanguage(preferred, environment, locale);
    return found ? found->name : std::string{};
}

} // namespace

TEST_CASE("a language's name is two or three letters, and a region of two", "[i18n][language]") {
    for (const char* name : {"fr", "ast", "pt_BR", "en", "zh_CN"})
        CHECK(isLanguageName(name));
    for (const char* name :
         {"", "f", "fren", "FR", "fr_", "fr_br", "fr_BRA", "fr-FR", "fr_FR.UTF-8", "system", "é"})
        CHECK_FALSE(isLanguageName(name));
}

TEST_CASE("the setting wins over the environment when its language is installed",
          "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");
    locale.install("fr", "fr.mo");

    const EnvironmentLookup french = environmentOf({{"LANG", "fr_FR.UTF-8"}});
    CHECK(chosenName("ru", french, locale.path()) == "ru");
    // A region falls back on its language, as a locale of the environment does.
    CHECK(chosenName("ru_RU", french, locale.path()) == "ru");
    // `LC_ALL=C` is the environment's way of asking for English; a setting is asked for too.
    CHECK(chosenName("ru", environmentOf({{"LC_ALL", "C"}}), locale.path()) == "ru");
    // English, named, is English whatever the environment says.
    CHECK(chosenName("en", french, locale.path()).empty());
    CHECK(chosenName("en_GB", french, locale.path()).empty());
}

TEST_CASE("no setting, or one that is not installed, leaves the environment to decide",
          "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");

    const EnvironmentLookup russian = environmentOf({{"LANG", "ru_RU.UTF-8"}});
    CHECK(chosenName(std::nullopt, russian, locale.path()) == "ru");
    CHECK(chosenName("de", russian, locale.path()) == "ru");
    CHECK(chosenName("de", environmentOf({}), locale.path()).empty());
}

TEST_CASE("the languages offered are those whose catalogue reads", "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");
    locale.install("fr", "fr.mo");
    locale.damage("de");

    CHECK(availableLanguages(locale.path()) == Names{"fr", "ru"});
    CHECK(availableLanguages(locale.path() / "absent").empty());
}

TEST_CASE("the languages reported are English and those whose catalogue reads",
          "[i18n][language]") {
    const LocaleDirectory locale;
    locale.install("ru", "ru.mo");
    locale.install("fr", "fr.mo");
    locale.damage("de");

    // English first: the source, which needs no catalogue and is always there.
    CHECK(installedLanguageCodes(locale.path()) == Names{"en", "fr", "ru"});
    CHECK(installedLanguageCodes(locale.path() / "absent") == Names{"en"});
}

TEST_CASE("a language is named in its own language", "[i18n][language]") {
    CHECK(endonymOf("en") == "English");
    CHECK(endonymOf("fr") == "Français");
    CHECK(endonymOf("ru") == "Русский");
    CHECK(endonymOf("pt_BR") == "Português do Brasil");
    // A region with no name of its own takes its language's.
    CHECK(endonymOf("fr_CA") == "Français");
    CHECK_FALSE(endonymOf("xx").has_value());
    CHECK_FALSE(endonymOf("xx_YY").has_value());
}
