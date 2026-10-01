// The join-and-split page of the correction assistant — issue #508, D6/D8.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/join_split_page.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::CorrectionSettings;
using subedit::core::noDictionaryFor;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::JoinSplitPage;
using subedit::gui::spellLanguageFor;

WordListSpellProvider providerOf(std::initializer_list<const char*> languages) {
    WordListSpellProvider provider;
    for (const char* code : languages)
        provider.add(code, WordList{});
    return provider;
}

CorrectionSettings settingsFor(const char* language) {
    CorrectionSettings settings;
    settings.spellLanguage = language;
    return settings;
}

QCheckBox* boxNamed(const JoinSplitPage& page, const QString& text) {
    for (QCheckBox* box : page.findChildren<QCheckBox*>()) {
        if (box->text() == text)
            return box;
    }
    return nullptr;
}

} // namespace

TEST_CASE("a page with a dictionary opens on Gaupol's defaults: join on, split off",
          "[gui][join-split-page]") {
    const WordListSpellProvider provider = providerOf({"fr"});
    JoinSplitPage page{&provider};

    page.applySettings(settingsFor("fr"));

    CHECK(page.available());
    CHECK(page.language() == "fr");
    CHECK(page.join());
    CHECK_FALSE(page.split());
    CHECK(boxNamed(page, QStringLiteral("Join words"))->isEnabled());
    CHECK(page.unavailableReason().isEmpty());
}

TEST_CASE("GUI-SPELL-02: without a dictionary the page is greyed and says why",
          "[gui][join-split-page][GUI-SPELL-02]") {
    const WordListSpellProvider provider = providerOf({"en"});
    JoinSplitPage page{&provider};

    page.applySettings(settingsFor("fr"));

    CHECK_FALSE(page.available());
    CHECK_FALSE(boxNamed(page, QStringLiteral("Join words"))->isEnabled());
    CHECK_FALSE(boxNamed(page, QStringLiteral("Split words"))->isEnabled());
    CHECK(page.unavailableReason().toStdString() == noDictionaryFor("fr"));
}

TEST_CASE("GUI-SPELL-02: a window with no spell-checking says the same",
          "[gui][join-split-page][GUI-SPELL-02]") {
    JoinSplitPage page{nullptr};

    page.applySettings(settingsFor("fr"));

    CHECK_FALSE(page.available());
    CHECK(page.unavailableReason().toStdString() == "no dictionary for fr");
}

TEST_CASE("the languages offered are the ones with a dictionary, and the language chosen too",
          "[gui][join-split-page]") {
    const WordListSpellProvider provider = providerOf({"fr_FR", "en", "en-variant_0", "de_DE"});
    JoinSplitPage page{&provider};

    page.applySettings(settingsFor("es"));

    const auto* combo = page.findChild<QComboBox*>();
    REQUIRE(combo != nullptr);
    QStringList shown;
    for (int row = 0; row < combo->count(); ++row)
        shown << combo->itemText(row);
    CHECK(shown == QStringList{"de_DE", "en", "fr_FR", "es"}); // the odd code is filtered out
    CHECK(page.language() == "es");
    CHECK_FALSE(page.available()); // offered, so it can be seen to be missing
}

TEST_CASE("choosing another language greys or ungreys the boxes", "[gui][join-split-page]") {
    const WordListSpellProvider provider = providerOf({"fr", "en"});
    JoinSplitPage page{&provider};
    page.applySettings(settingsFor("fr"));
    auto* combo = page.findChild<QComboBox*>();
    REQUIRE(combo != nullptr);

    combo->setCurrentText(QStringLiteral("en"));
    CHECK(page.available());
    CHECK(page.language() == "en");

    combo->addItem(QStringLiteral("zz"));
    combo->setCurrentText(QStringLiteral("zz"));
    CHECK_FALSE(page.available());
    CHECK_FALSE(boxNamed(page, QStringLiteral("Split words"))->isEnabled());

    combo->setCurrentText(QStringLiteral("fr"));
    CHECK(page.available());
    CHECK(boxNamed(page, QStringLiteral("Split words"))->isEnabled());
}

TEST_CASE("the boxes read back what the settings gave them", "[gui][join-split-page]") {
    const WordListSpellProvider provider = providerOf({"fr"});
    JoinSplitPage page{&provider};
    CorrectionSettings settings = settingsFor("fr");
    settings.joinWords = false;
    settings.splitWords = true;

    page.applySettings(settings);

    CHECK_FALSE(page.join());
    CHECK(page.split());
}

TEST_CASE("with no language chosen the system's is taken, exactly and then by its language",
          "[gui][join-split-page]") {
    const std::vector<std::string> offered{"de_DE", "en_GB", "fr_FR"};

    CHECK(spellLanguageFor(offered, "fr_FR") == "fr_FR"); // exact
    CHECK(spellLanguageFor(offered, "fr_CA") == "fr_FR"); // its language alone
    CHECK(spellLanguageFor(offered, "es_ES") == "es_ES"); // none: the system's own, said missing
    CHECK(spellLanguageFor(offered, "en") == "en_GB");    // a bare language finds its country
    CHECK(spellLanguageFor({}, "fr_FR") == "fr_FR");      // nothing offered at all
    CHECK(spellLanguageFor(offered, "C") == "en_GB");     // not a code: read as English
    CHECK(spellLanguageFor(offered, "") == "en_GB");
}
