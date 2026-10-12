// Which language was asked for — issue #658.

#include <subedit/core/i18n/language.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>

using subedit::core::cataloguePath;
using subedit::core::EnvironmentLookup;
using subedit::core::installedCatalogue;
using subedit::core::installedLanguage;
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
