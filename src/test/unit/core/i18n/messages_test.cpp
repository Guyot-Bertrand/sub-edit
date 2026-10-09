// translate, translatePlural, translateIn and substitute — issue #658.

#include <subedit/core/i18n/messages.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

using namespace subedit::core;

namespace {

std::shared_ptr<const Catalogue> load(const char* name) {
    auto catalogue = Catalogue::fromFile(std::string(SUBEDIT_COMPILED_I18N_DIR) + "/" + name);
    REQUIRE(catalogue.has_value());
    return std::make_shared<const Catalogue>(std::move(*catalogue));
}

/// Puts English back when the test ends, whatever happened in it.
struct EnglishAfterwards {
    EnglishAfterwards() = default;
    EnglishAfterwards(const EnglishAfterwards&) = delete;
    EnglishAfterwards& operator=(const EnglishAfterwards&) = delete;
    EnglishAfterwards(EnglishAfterwards&&) = delete;
    EnglishAfterwards& operator=(EnglishAfterwards&&) = delete;

    ~EnglishAfterwards() { installCatalogue(nullptr); }
};

} // namespace

TEST_CASE("with no catalogue the text is its own translation", "[i18n][messages]") {
    CHECK(translate("Save") == "Save");
    CHECK(translateIn("verb", "Open") == "Open");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 1) == "%1 subtitle");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 0) == "%1 subtitles");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 2) == "%1 subtitles");
}

TEST_CASE("an installed catalogue translates what it knows", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(translate("Save") == "Enregistrer");
    CHECK(translate("Save As…") == "Enregistrer sous…");
}

TEST_CASE("what the catalogue lacks stays English", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(translate("Quit") == "Quit");
    CHECK(translate("Close") == "Close");
}

TEST_CASE("a context tells two identical words apart", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(translateIn("verb", "Open") == "Ouvrir");
    CHECK(translateIn("adjective", "Open") == "Ouvert");
    // Without its context a message is another message, and the catalogue does not have it.
    CHECK(translate("Open") == "Open");
    CHECK(translateIn("noun", "Open") == "Open");
}

TEST_CASE("the plural follows the catalogue's rule", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 0) == "%1 sous-titre");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 1) == "%1 sous-titre");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 2) == "%1 sous-titres");

    installCatalogue(load("ru.mo"));
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 1) == "%1 субтитр");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 3) == "%1 субтитра");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 5) == "%1 субтитров");
    CHECK(translatePlural("%1 subtitle", "%1 subtitles", 21) == "%1 субтитр");
}

TEST_CASE("a plural the catalogue lacks falls back to English's rule", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(translatePlural("%1 line", "%1 lines", 1) == "%1 line");
    CHECK(translatePlural("%1 line", "%1 lines", 0) == "%1 lines");
}

TEST_CASE("arguments are substituted by position", "[i18n][messages]") {
    CHECK(substitute("Moved %1 to %2", {"a", "b"}) == "Moved a to b");
    CHECK(substitute("%2 then %1", {"a", "b"}) == "b then a");
    CHECK(substitute("%1 and %1", {"a"}) == "a and a");
    CHECK(substitute("no argument", {}) == "no argument");
}

TEST_CASE("a translation may reorder the arguments", "[i18n][messages]") {
    const EnglishAfterwards guard;
    installCatalogue(load("fr.mo"));
    CHECK(substitute(translate("Moved %1 to %2"), {"A", "B"}) == "Déplacé vers B depuis A");
}

TEST_CASE("a placeholder with nothing to fill it stays as written", "[i18n][messages]") {
    CHECK(substitute("%3", {"a"}) == "%3");
    CHECK(substitute("100%", {"a"}) == "100%");
    CHECK(substitute("%0 %x %", {"a"}) == "%0 %x %");
    CHECK(substitute("%1%2", {"a"}) == "a%2");
}

TEST_CASE("an argument that looks like a placeholder is not expanded", "[i18n][messages]") {
    CHECK(substitute("%1 %2", {"%2", "b"}) == "%2 b");
}
