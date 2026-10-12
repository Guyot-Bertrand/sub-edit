// The window under the French that ships — issue #668. The other language tests read fixtures;
// this one reads `src/po/fr.po` as the build compiles and installs it, beside the binaries of the
// build tree, and finds the window's shell in French: every text of it, since the catalogue is
// complete.

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

/// A French user's environment, of the test's own and never the process's.
[[nodiscard]] std::optional<std::string> french(std::string_view name) {
    if (name == "LANGUAGE")
        return "fr";
    if (name == "LANG")
        return "fr_FR.UTF-8";
    return std::nullopt;
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

/// The shipped French catalogue, read directly: the test asks it what each text should become.
[[nodiscard]] Catalogue shipped() {
    auto catalogue = Catalogue::fromFile(kShipped / "fr" / "LC_MESSAGES" / "subedit.mo");
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

TEST_CASE("the window speaks the French that ships, every text of its shell",
          "[gui][i18n][GUI-I18N-01]") {
    std::vector<QString> english;
    {
        EmptyWindow shell;
        english = shellTexts(shell.window);
    }

    const InterfaceLanguage language{french, kShipped, QString{}};
    REQUIRE(language.language() == "fr");
    EmptyWindow shell;

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

    // **Complete, so nothing is left in English**: every text of the shell that has a word is in
    // the shipped catalogue, and shown as the catalogue gives it; a sign, such as the `#` of the
    // numbers' column, is no message.
    const Catalogue catalogue = shipped();
    const std::vector<QString> frenchTexts = shellTexts(shell.window);
    REQUIRE(frenchTexts.size() == english.size());
    REQUIRE(english.size() > 80);
    for (std::size_t at = 0; at < english.size(); ++at) {
        const std::string source = english.at(at).toStdString();
        INFO(source);
        if (std::ranges::none_of(english.at(at),
                                 [](QChar character) { return character.isLetter(); })) {
            CHECK(frenchTexts.at(at) == english.at(at));
            continue;
        }
        const std::optional<std::string_view> expected = catalogue.find({}, source);
        // Absent from the catalogue, the text would fail here: nothing of the window matches it.
        CHECK(expected.has_value());
        CHECK(frenchTexts.at(at).toStdString() ==
              expected.value_or("<not in the shipped catalogue>"));
    }

    // The word told apart by a context, and what only a pointer shows.
    CHECK(shell.window.findChild<QToolBar*>()->windowTitle() == QStringLiteral("Principale"));
    CHECK(subedit::gui::documentName(subedit::core::Document::Main) == QStringLiteral("Principal"));
    const auto* const newProject =
        shell.window.menuBar()->actions().front()->menu()->actions().front();
    CHECK(newProject->toolTip() == QStringLiteral("Ouvrir un projet vide dans un nouvel onglet"));
    CHECK(shell.window.tabBar()->tabText(0) == QStringLiteral("sans titre"));
}

TEST_CASE("with French installed beside it, the window is English when English is asked for",
          "[gui][i18n][GUI-I18N-01]") {
    const auto english = [](const subedit::core::EnvironmentLookup& environment) {
        const InterfaceLanguage language{environment, kShipped, QString{}};
        CHECK_FALSE(language.language().has_value());
        const EmptyWindow shell;
        CHECK(shell.window.menuBar()->actions().front()->text() == QStringLiteral("&File"));
    };

    SECTION("LC_ALL=C, whatever LANGUAGE asks") {
        english([](std::string_view name) -> std::optional<std::string> {
            if (name == "LC_ALL")
                return "C";
            if (name == "LANGUAGE")
                return "fr";
            return std::nullopt;
        });
    }
    SECTION("LANGUAGE naming English first, whatever the locale") {
        english([](std::string_view name) -> std::optional<std::string> {
            if (name == "LANGUAGE")
                return "en:fr";
            if (name == "LANG")
                return "fr_FR.UTF-8";
            return std::nullopt;
        });
    }
}
