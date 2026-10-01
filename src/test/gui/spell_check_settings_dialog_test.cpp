// The settings dialog of the spell check — issue #509, D6.

#include <subedit/core/config/spell_check_settings.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using subedit::core::noDictionaryFor;
using subedit::core::SpellCheckDocument;
using subedit::core::SpellCheckSettings;
using subedit::core::SpellCheckTarget;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::SpellCheckSettingsDialog;

WordListSpellProvider providerOf(std::initializer_list<const char*> languages) {
    WordListSpellProvider provider;
    for (const char* code : languages)
        provider.add(code, WordList{});
    return provider;
}

} // namespace

TEST_CASE("the dialog opens on the language given and offers those with a dictionary",
          "[gui][spell-check-settings]") {
    const WordListSpellProvider provider = providerOf({"en", "fr"});
    SpellCheckSettingsDialog dialog{&provider, true, true};

    dialog.apply({.language = "fr"});

    CHECK(dialog.language() == "fr");
    CHECK(dialog.languageBox()->count() == 2);
    CHECK(dialog.available());
    CHECK(dialog.unavailableReason().isEmpty());
}

TEST_CASE("an empty language opens on one of those offered when the system's is",
          "[gui][spell-check-settings]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, true, true};

    dialog.apply({});

    CHECK_FALSE(dialog.language().empty());
}

TEST_CASE("GUI-SPELL-02: a language without a dictionary is said so and stays validable",
          "[gui][spell-check-settings][GUI-SPELL-02]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, true, true};

    dialog.apply({.language = "fr"});

    CHECK(dialog.language() == "fr");
    CHECK_FALSE(dialog.available());
    CHECK(dialog.unavailableReason() == QString::fromStdString(noDictionaryFor("fr")));
    CHECK(dialog.buttons()->button(QDialogButtonBox::Ok)->isEnabled());
}

TEST_CASE("the target and document chosen are given back", "[gui][spell-check-settings]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, true, true};

    dialog.apply({.language = "en",
                  .target = SpellCheckTarget::AllProjects,
                  .document = SpellCheckDocument::Translation});

    CHECK(dialog.allProjectsRadio()->isChecked());
    CHECK(dialog.translationRadio()->isChecked());
    CHECK(dialog.settings() == SpellCheckSettings{.language = "en",
                                                  .target = SpellCheckTarget::AllProjects,
                                                  .document = SpellCheckDocument::Translation});

    dialog.selectionRadio()->setChecked(true);
    dialog.textRadio()->setChecked(true);
    CHECK(dialog.target() == SpellCheckTarget::Selection);
    CHECK(dialog.document() == SpellCheckDocument::Main);
}

TEST_CASE("the selection target is kept when something is selected",
          "[gui][spell-check-settings]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, true, false};

    dialog.apply({.language = "en", .target = SpellCheckTarget::Selection});

    CHECK(dialog.selectionRadio()->isEnabled());
    CHECK(dialog.selectionRadio()->isChecked());
    CHECK(dialog.target() == SpellCheckTarget::Selection);
}

TEST_CASE("the selection is greyed when nothing is selected, and the translation without one",
          "[gui][spell-check-settings]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, false, false};

    dialog.apply({.language = "en",
                  .target = SpellCheckTarget::Selection,
                  .document = SpellCheckDocument::Translation});

    CHECK_FALSE(dialog.selectionRadio()->isEnabled());
    CHECK_FALSE(dialog.translationRadio()->isEnabled());
    CHECK(dialog.target() == SpellCheckTarget::CurrentProject);
    CHECK(dialog.document() == SpellCheckDocument::Main);
}

TEST_CASE("a dialog without a provider says there is no dictionary",
          "[gui][spell-check-settings]") {
    SpellCheckSettingsDialog dialog{nullptr, false, false};

    dialog.apply({.language = "en"});

    CHECK_FALSE(dialog.available());
    CHECK(dialog.language() == "en");
}

TEST_CASE("GUI-SPELL-04: the dialog offers the inline check, off by default",
          "[gui][spell-check-settings][GUI-SPELL-04]") {
    const WordListSpellProvider provider = providerOf({"en"});
    SpellCheckSettingsDialog dialog{&provider, true, true};

    dialog.apply({.language = "en"});
    CHECK(dialog.inlineCheckBox()->text() == QStringLiteral("Check spelling while editing"));
    CHECK_FALSE(dialog.inlineCheck());
    CHECK_FALSE(dialog.settings().inlineCheck);

    dialog.apply({.language = "en", .inlineCheck = true});
    CHECK(dialog.inlineCheckBox()->isChecked());
    CHECK(dialog.settings().inlineCheck);

    dialog.inlineCheckBox()->setChecked(false);
    CHECK_FALSE(dialog.settings().inlineCheck);
}
