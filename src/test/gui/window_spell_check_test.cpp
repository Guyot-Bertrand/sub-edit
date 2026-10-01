// `Tools > Check Spelling…` and `Tools > Spell-Check Settings…` in the window
// — issue #509, D6: the entries, and the greying of the first when there is
// no dictionary for the language (GUI-SPELL-02).

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/spell_check_dialog.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabBar>
#include <QTableView>
#include <QTextBlock>
#include <QTextLayout>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::noDictionaryFor;
using subedit::core::openProject;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;
using subedit::gui::MainWindow;
using subedit::gui::SpellCheckDialog;
using subedit::gui::SpellCheckSettingsDialog;
using subedit::test::FakePrompts;

/// A directory that removes itself: the list of replacements is written under
/// it, and never under a resolved configuration location.
class ScratchDirectory {

public:
    ScratchDirectory() {
        static int serial = 0;
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-spell-window-" + std::to_string(++serial) + "-" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(m_path);
    }

    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory(ScratchDirectory&&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(ScratchDirectory&&) = delete;

    ~ScratchDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

[[nodiscard]] InMemoryFileSystem withFile() {
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nok qqq\n\n");
    return files;
}

[[nodiscard]] subedit::core::OpenedFile fileIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::shared_ptr<WordListSpellProvider> provider() {
    WordList list;
    list.words = {"ok", "salut"};
    list.suggestions = {{"qqq", {"salut"}}};
    auto result = std::make_shared<WordListSpellProvider>();
    result->add("fr", std::move(list));
    return result;
}

[[nodiscard]] subedit::core::Settings settingsIn(const char* language) {
    subedit::core::Settings settings;
    settings.spellCheck.language = language;
    return settings;
}

/// The words of the editor open on the text cell of the first row that are
/// underlined as misspelt, and whether there is such an editor at all.
struct OpenEditor {
    bool found = false;
    std::vector<QString> underlined;
};

[[nodiscard]] OpenEditor openTextEditor(MainWindow& window) {
    window.table()->edit(window.table()->model()->index(0, 4));
    QCoreApplication::processEvents();
    OpenEditor result;
    const auto* field = window.table()->findChild<QPlainTextEdit*>();
    if (field == nullptr)
        return result;
    result.found = true;
    for (QTextBlock block = field->document()->firstBlock(); block.isValid();
         block = block.next()) {
        for (const QTextLayout::FormatRange& range : block.layout()->formats()) {
            if (range.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline)
                result.underlined.push_back(block.text().mid(range.start, range.length));
        }
    }
    return result;
}

[[nodiscard]] subedit::core::Settings inlineSettingsIn(const char* language, bool inlineCheck) {
    subedit::core::Settings settings = settingsIn(language);
    settings.spellCheck.inlineCheck = inlineCheck;
    return settings;
}

} // namespace

TEST_CASE("the Tools menu holds Check Spelling and its settings", "[gui][spell-check-window]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files), prompts};

    QMenu* tools = nullptr;
    for (QMenu* menu : window.menuBar()->findChildren<QMenu*>()) {
        if (menu->title() == QStringLiteral("&Tools"))
            tools = menu;
    }
    REQUIRE(tools != nullptr);
    CHECK(tools->actions().contains(window.checkSpellingAction()));
    CHECK(tools->actions().contains(window.spellCheckSettingsAction()));
    CHECK(window.checkSpellingAction()->text().remove(QLatin1Char('&')) ==
          QStringLiteral("Check Spelling…"));
    CHECK(window.spellCheckSettingsAction()->text().remove(QLatin1Char('&')) ==
          QStringLiteral("Spell-Check Settings…"));
}

TEST_CASE("GUI-SPELL-02: Check Spelling is greyed, with the reason, when there is no dictionary",
          "[gui][spell-check-window][GUI-SPELL-02]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(settingsIn("de"));
    window.show();

    const QString reason = QString::fromStdString(noDictionaryFor("de"));
    CHECK_FALSE(window.checkSpellingAction()->isEnabled());
    CHECK(window.checkSpellingAction()->toolTip() == reason);
    CHECK(window.checkSpellingAction()->statusTip() == reason);
    // The way out is never closed by the dictionary.
    CHECK(window.spellCheckSettingsAction()->isEnabled());
}

TEST_CASE("GUI-SPELL-02: a window with no dictionary provider greys Check Spelling",
          "[gui][spell-check-window][GUI-SPELL-02]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.applySettings(settingsIn("fr"));

    CHECK_FALSE(window.checkSpellingAction()->isEnabled());
    CHECK(window.checkSpellingAction()->toolTip() == QString::fromStdString(noDictionaryFor("fr")));
    CHECK(window.spellCheckSettingsAction()->isEnabled());
}

TEST_CASE("GUI-SPELL-02: choosing a language that has a dictionary lights the entry again",
          "[gui][spell-check-window][GUI-SPELL-02]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<SpellCheckSettingsDialog&>(dialog).apply({.language = "fr"});
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(settingsIn("de"));
    window.show();
    REQUIRE_FALSE(window.checkSpellingAction()->isEnabled());

    window.spellCheckSettingsAction()->trigger();

    CHECK(window.checkSpellingAction()->isEnabled());
    CHECK(window.checkSpellingAction()->toolTip() != QString::fromStdString(noDictionaryFor("de")));
    CHECK(window.settings().spellCheck.language == "fr");
}

TEST_CASE("the spell-check settings survive through the window's settings",
          "[gui][spell-check-window]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    subedit::core::Settings settings;
    settings.spellCheck = {.language = "fr",
                           .target = subedit::core::SpellCheckTarget::AllProjects,
                           .document = subedit::core::SpellCheckDocument::Translation};

    window.applySettings(settings);

    CHECK(window.settings().spellCheck == settings.spellCheck);
}

TEST_CASE("GUI-SPELL-01: Check Spelling from the menu corrects the text of the window",
          "[gui][spell-check-window][GUI-SPELL-01]") {
    const ScratchDirectory scratch;
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<SpellCheckDialog&>(dialog).replaceButton()->click();
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), scratch.path());
    window.applySettings(settingsIn("fr"));
    window.show();

    window.checkSpellingAction()->trigger();

    CHECK(window.table()
              ->model()
              ->data(window.table()->model()->index(0, 4), Qt::DisplayRole)
              .toString() == QStringLiteral("ok salut"));
}

TEST_CASE("GUI-SPELL-04: with the setting on and a dictionary, the open editor underlines",
          "[gui][spell-check-window][GUI-SPELL-04]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(inlineSettingsIn("fr", true));
    window.show();

    const OpenEditor editor = openTextEditor(window);

    REQUIRE(editor.found);
    CHECK(editor.underlined == std::vector<QString>{QStringLiteral("qqq")});
}

TEST_CASE("GUI-SPELL-04: with the setting off nothing is underlined",
          "[gui][spell-check-window][GUI-SPELL-04]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(inlineSettingsIn("fr", false));
    window.show();

    const OpenEditor editor = openTextEditor(window);

    REQUIRE(editor.found);
    CHECK(editor.underlined.empty());
}

TEST_CASE("GUI-SPELL-04: without a dictionary for the language the editor is silent",
          "[gui][spell-check-window][GUI-SPELL-04]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(inlineSettingsIn("de", true));
    window.show();

    const OpenEditor editor = openTextEditor(window);

    REQUIRE(editor.found);
    CHECK(editor.underlined.empty());
}

TEST_CASE("GUI-SPELL-04: a window with no provider underlines nothing",
          "[gui][spell-check-window][GUI-SPELL-04]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.applySettings(inlineSettingsIn("fr", true));
    window.show();

    const OpenEditor editor = openTextEditor(window);

    REQUIRE(editor.found);
    CHECK(editor.underlined.empty());
}

TEST_CASE("GUI-SPELL-04: the setting made in the dialog takes effect on the next editor",
          "[gui][spell-check-window][GUI-SPELL-04]") {
    InMemoryFileSystem files = withFile();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<SpellCheckSettingsDialog&>(dialog).inlineCheckBox()->setChecked(true);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), "/nonexistent-config");
    window.applySettings(inlineSettingsIn("fr", false));
    window.show();

    window.spellCheckSettingsAction()->trigger();

    CHECK(window.settings().spellCheck.inlineCheck);
    const OpenEditor editor = openTextEditor(window);
    REQUIRE(editor.found);
    CHECK(editor.underlined == std::vector<QString>{QStringLiteral("qqq")});
}

TEST_CASE("GUI-SPELL-01: a word found in another tab brings that tab forward",
          "[gui][spell-check-window][GUI-SPELL-01]") {
    const ScratchDirectory scratch;
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nok\n\n");
    files.addFile("autre.srt", "1\n00:00:01,000 --> 00:00:02,000\nok qqq\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<SpellCheckDialog&>(dialog).replaceButton()->click();
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setSpellChecking(provider(), scratch.path());
    subedit::core::Settings settings = settingsIn("fr");
    settings.spellCheck.target = subedit::core::SpellCheckTarget::AllProjects;
    window.applySettings(settings);
    window.show();
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->count() == 2);
    window.tabBar()->setCurrentIndex(0);

    window.checkSpellingAction()->trigger();

    CHECK(window.tabBar()->currentIndex() == 1);
}
