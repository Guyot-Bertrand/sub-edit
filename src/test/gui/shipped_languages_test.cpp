// The window under every language that ships — issues #668 and #720. The other language tests
// read fixtures; this one reads `src/po/<language>.po` as the build compiles and installs it,
// beside the binaries of the build tree, and finds the window's shell in that language: every text
// of it, since a shipped catalogue is complete.
//
// **The languages are those of `src/po/LINGUAS`**, handed to this file alone by its CMakeLists
// (`SUBEDIT_SHIPPED_LANGUAGES`): what is asked of every language is asked of its own catalogue,
// and French keeps a case of its own, which names the words.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/i18n/catalogue.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/gui/interface_language.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/target.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QTabBar>
#include <QTableView>
#include <QToolBar>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Catalogue;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::gui::InterfaceLanguage;
using subedit::gui::MainWindow;

/// The catalogues of the build tree, where `cmake/Translations.cmake` puts what it installs.
const std::filesystem::path kShipped{SUBEDIT_SHIPPED_LOCALE_DIR};

/// The lines of `src/po/LINGUAS`, comma-separated by the build.
[[nodiscard]] std::vector<std::string> shippedLanguages() {
    std::vector<std::string> languages;
    const std::string_view list{SUBEDIT_SHIPPED_LANGUAGES};
    std::size_t start = 0;
    while (start < list.size()) {
        std::size_t end = list.find(',', start);
        if (end == std::string_view::npos)
            end = list.size();
        if (end > start)
            languages.emplace_back(list.substr(start, end - start));
        start = end + 1;
    }
    return languages;
}

/// A user of `language`'s environment, of the test's own and never the process's.
[[nodiscard]] subedit::core::EnvironmentLookup userOf(const std::string& language) {
    return [language](std::string_view name) -> std::optional<std::string> {
        if (name == "LANGUAGE")
            return language;
        if (name == "LANG")
            return language + ".UTF-8";
        return std::nullopt;
    };
}

/// Every text of the shell a catalogue may translate: the menus and their entries, submenus
/// included, the toolbar's words and the columns.
[[nodiscard]] std::vector<QString> shellTexts(MainWindow& window) {
    std::vector<QString> texts;
    const auto walk = [&texts](const auto& self, const QList<QAction*>& actions) -> void {
        for (const QAction* action : actions) {
            if (action->isSeparator())
                continue;
            texts.push_back(action->text());
            if (const QMenu* menu = action->menu())
                self(self, menu->actions());
        }
    };
    walk(walk, window.menuBar()->actions());
    for (const QToolBar* bar : window.findChildren<QToolBar*>()) {
        for (const QAction* action : bar->actions())
            if (!action->isSeparator())
                texts.push_back(action->iconText());
    }
    const QAbstractItemModel* model = window.findChild<QTableView*>()->model();
    for (int column = 0; column < model->columnCount(); ++column)
        texts.push_back(model->headerData(column, Qt::Horizontal).toString());
    return texts;
}

/// A shipped catalogue, read directly: the test asks it what each text should become.
[[nodiscard]] Catalogue shipped(const std::string& language) {
    auto catalogue = Catalogue::fromFile(kShipped / language / "LC_MESSAGES" / "subedit.mo");
    REQUIRE(catalogue.has_value());
    return *std::move(catalogue);
}

/// A window with nothing open, as a user meets it.
struct EmptyWindow {
    InMemoryFileSystem files;
    subedit::test::FakePrompts prompts;
    MainWindow window{files, OpenedFile{}, prompts};
};

} // namespace

TEST_CASE("the window speaks every language that ships, every text of its shell",
          "[gui][i18n][GUI-I18N-01]") {
    std::vector<QString> english;
    std::vector<QString> englishTitles;
    {
        EmptyWindow shell;
        english = shellTexts(shell.window);
        for (const QAction* action : shell.window.menuBar()->actions())
            englishTitles.push_back(action->text());
    }

    const std::string code = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << code) {
        const InterfaceLanguage language{userOf(code), kShipped, QString{}};
        REQUIRE(language.language() == code);
        EmptyWindow shell;
        const Catalogue catalogue = shipped(code);

        // The menu titles, each as the catalogue gives it, and not the English ones.
        std::vector<QString> titles;
        for (const QAction* action : shell.window.menuBar()->actions())
            titles.push_back(action->text());
        REQUIRE(titles.size() == englishTitles.size());
        REQUIRE(titles.size() > 5);
        for (std::size_t at = 0; at < titles.size(); ++at) {
            const std::string source = englishTitles.at(at).toStdString();
            INFO(source);
            CHECK(titles.at(at).toStdString() ==
                  catalogue.find({}, source).value_or("<not in the shipped catalogue>"));
        }
        CHECK(titles != englishTitles);

        // **Complete, so nothing is left in English**: every text of the shell that has a word is
        // in the shipped catalogue, and shown as the catalogue gives it; a sign, such as the `#` of
        // the numbers' column, is no message.
        const std::vector<QString> translated = shellTexts(shell.window);
        REQUIRE(translated.size() == english.size());
        REQUIRE(english.size() > 80);
        for (std::size_t at = 0; at < english.size(); ++at) {
            const std::string source = english.at(at).toStdString();
            INFO(source);
            if (std::ranges::none_of(english.at(at),
                                     [](QChar character) { return character.isLetter(); })) {
                CHECK(translated.at(at) == english.at(at));
                continue;
            }
            const std::optional<std::string_view> expected = catalogue.find({}, source);
            // Absent from the catalogue, the text would fail here: nothing of the window matches
            // it.
            CHECK(expected.has_value());
            CHECK(translated.at(at).toStdString() ==
                  expected.value_or("<not in the shipped catalogue>"));
        }
    }
}

TEST_CASE("the window says the French words the catalogue ships", "[gui][i18n][GUI-I18N-01]") {
    const InterfaceLanguage language{userOf("fr"), kShipped, QString{}};
    REQUIRE(language.language() == "fr");
    const EmptyWindow shell;

    std::vector<QString> titles;
    for (const QAction* action : shell.window.menuBar()->actions())
        titles.push_back(action->text());
    CHECK(titles == std::vector<QString>{QStringLiteral("&Fichier"),
                                         QStringLiteral("É&dition"),
                                         QStringLiteral("&Affichage"),
                                         QStringLiteral("&Vidéo"),
                                         QStringLiteral("&Outils"),
                                         QStringLiteral("&Projets"),
                                         QStringLiteral("Aid&e")});

    // The word told apart by a context, and what only a pointer shows.
    CHECK(shell.window.findChild<QToolBar*>()->windowTitle() == QStringLiteral("Principale"));
    CHECK(subedit::gui::documentName(subedit::core::Document::Main) == QStringLiteral("Principal"));
    const auto* const newProject =
        shell.window.menuBar()->actions().front()->menu()->actions().front();
    CHECK(newProject->toolTip() == QStringLiteral("Ouvrir un projet vide dans un nouvel onglet"));
    CHECK(shell.window.tabBar()->tabText(0) == QStringLiteral("sans titre"));
}

TEST_CASE("with a language installed beside it, the window is English when English is asked for",
          "[gui][i18n][GUI-I18N-01]") {
    const auto english = [](const subedit::core::EnvironmentLookup& environment) {
        const InterfaceLanguage language{environment, kShipped, QString{}};
        CHECK_FALSE(language.language().has_value());
        const EmptyWindow shell;
        CHECK(shell.window.menuBar()->actions().front()->text() == QStringLiteral("&File"));
    };

    const std::string code = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << code << ", LC_ALL=C, whatever LANGUAGE asks") {
        english([code](std::string_view name) -> std::optional<std::string> {
            if (name == "LC_ALL")
                return "C";
            if (name == "LANGUAGE")
                return code;
            return std::nullopt;
        });
    }
    DYNAMIC_SECTION("language " << code << ", LANGUAGE naming English first, whatever the locale") {
        english([code](std::string_view name) -> std::optional<std::string> {
            if (name == "LANGUAGE")
                return "en:" + code;
            if (name == "LANG")
                return code + ".UTF-8";
            return std::nullopt;
        });
    }
}
