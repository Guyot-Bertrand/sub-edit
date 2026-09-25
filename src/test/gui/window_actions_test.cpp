// The actions, the menus and the toolbar, without the window — issue #483.
//
// `WindowActions` builds and lays out, and connects nothing: these cases build
// it on a bare `QMainWindow` and read what it laid down. What each action does
// once the window has connected it is proved by the window's own tests.

#include <subedit/core/text/letter_case.hpp>
#include <subedit/gui/window_actions.hpp>

#include <QAction>
#include <QKeySequence>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace {

using subedit::core::kLetterCases;
using subedit::gui::WindowActions;

/// A menu of `window` found by its title, or nothing.
[[nodiscard]] QMenu* menuTitled(const QMainWindow& window, const QString& title) {
    const QList<QMenu*> menus = window.menuBar()->findChildren<QMenu*>();
    const auto found =
        std::ranges::find_if(menus, [&](const QMenu* menu) { return menu->title() == title; });
    return found == menus.end() ? nullptr : *found;
}

/// The labels of `menu`, `|` for a separator and without the `&` mnemonics.
[[nodiscard]] std::vector<std::string> labelsOf(const QMenu& menu) {
    std::vector<std::string> labels;
    for (const QAction* action : menu.actions()) {
        labels.push_back(action->isSeparator()
                             ? "|"
                             : QString{action->text()}.remove(QLatin1Char('&')).toStdString());
    }
    return labels;
}

} // namespace

TEST_CASE("only the actions that need nothing open start lit", "[gui][GUI-OPEN-01]") {
    QObject owner;
    const WindowActions actions{&owner};

    const std::array lit{actions.newProject,
                         actions.open,
                         actions.save,
                         actions.saveAs,
                         actions.saveAllDocuments,
                         actions.closeAllProjects,
                         actions.nextTab,
                         actions.previousTab,
                         actions.selectVideo,
                         actions.preferences,
                         actions.about};
    for (const QAction* action : lit)
        CHECK(action->isEnabled());

    const std::array out{actions.undo,
                         actions.redo,
                         actions.closeProject,
                         actions.openTranslation,
                         actions.saveTranslation,
                         actions.saveTranslationAs,
                         actions.cut,
                         actions.copy,
                         actions.paste,
                         actions.findAndReplace,
                         actions.insert,
                         actions.remove,
                         actions.mergeSubtitles,
                         actions.splitSubtitle,
                         actions.playPause,
                         actions.shift,
                         actions.transform,
                         actions.frameRate,
                         actions.adjustDurations,
                         actions.appendFile,
                         actions.splitProject,
                         actions.italic,
                         actions.dialogueDashes,
                         actions.hearingImpaired,
                         actions.snap,
                         actions.shiftOntoGrid,
                         actions.analyseGrid,
                         actions.manual};
    for (const QAction* action : out)
        CHECK_FALSE(action->isEnabled());
    for (const QAction* action : actions.letterCase)
        CHECK_FALSE(action->isEnabled());
}

TEST_CASE("the actions belong to the owner they were given", "[gui][GUI-OPEN-01]") {
    QObject owner;
    const WindowActions actions{&owner};

    CHECK(actions.undo->parent() == &owner);
    CHECK(actions.about->parent() == &owner);
    CHECK(actions.letterCase.back()->parent() == &owner);
}

TEST_CASE("the shortcuts are the platform's, and the conventional ones where it says nothing",
          "[gui][GUI-SAVE-02]") {
    QObject owner;
    const WindowActions actions{&owner};

    CHECK(actions.saveAs->shortcuts().contains(QKeySequence{QStringLiteral("Ctrl+Shift+S")}));
    CHECK(actions.redo->shortcuts() == QKeySequence::keyBindings(QKeySequence::Redo));
    CHECK(actions.insert->shortcut() == QKeySequence{Qt::Key_Insert});
    CHECK(actions.italic->shortcut() == QKeySequence{QStringLiteral("Ctrl+I")});
    CHECK(actions.playPause->shortcut() == QKeySequence{QStringLiteral("Ctrl+P")});
    CHECK(actions.nextTab->shortcut() == QKeySequence{QStringLiteral("Ctrl+PgDown")});
    CHECK(actions.previousTab->shortcut() == QKeySequence{QStringLiteral("Ctrl+PgUp")});
    // Merging and splitting have none, on purpose.
    CHECK(actions.mergeSubtitles->shortcut().isEmpty());
    CHECK(actions.splitSubtitle->shortcut().isEmpty());
}

TEST_CASE("no two actions answer the same shortcut", "[gui][GUI-OPEN-01]") {
    QObject owner;
    const WindowActions actions{&owner};

    std::multiset<std::string> taken;
    for (const QAction* action : owner.findChildren<QAction*>()) {
        for (const QKeySequence& sequence : action->shortcuts())
            taken.insert(sequence.toString().toStdString());
    }
    for (const std::string& sequence : taken)
        CHECK(taken.count(sequence) == 1);
}

TEST_CASE("each case has its entry, in Gaupol's order", "[gui][GUI-OPEN-01]") {
    QObject owner;
    const WindowActions actions{&owner};

    for (std::size_t which = 0; which < kLetterCases.size(); ++which)
        CHECK(actions.caseAction(kLetterCases.at(which)) == actions.letterCase.at(which));
    CHECK(actions.letterCase.front()->text() == QStringLiteral("&Title Case"));
    CHECK(actions.letterCase.back()->text() == QStringLiteral("&lower case"));
}

TEST_CASE("the menus are laid in reading order, with the columns under View",
          "[gui][GUI-OPEN-01]") {
    QMainWindow window;
    const WindowActions actions{&window};
    QAction column{QStringLiteral("&Start")};
    const std::array<QAction*, 1> columns{&column};

    actions.placeIn(window, columns);

    QStringList titles;
    for (const QAction* menu : window.menuBar()->actions())
        titles << menu->text();
    CHECK(titles == QStringList{QStringLiteral("&File"),
                                QStringLiteral("&Edit"),
                                QStringLiteral("&View"),
                                QStringLiteral("&Video"),
                                QStringLiteral("&Tools"),
                                QStringLiteral("&Projects"),
                                QStringLiteral("&Help")});

    const QMenu* file = menuTitled(window, QStringLiteral("&File"));
    REQUIRE(file != nullptr);
    CHECK(labelsOf(*file) == std::vector<std::string>{"New Project",
                                                      "Open…",
                                                      "Open Translation…",
                                                      "|",
                                                      "Save",
                                                      "Save As…",
                                                      "|",
                                                      "Save Translation",
                                                      "Save Translation As…",
                                                      "|",
                                                      "Close"});

    const QMenu* columnMenu = menuTitled(window, QStringLiteral("&Columns"));
    REQUIRE(columnMenu != nullptr);
    CHECK(columnMenu->actions() == QList<QAction*>{&column});

    const QMenu* caseMenu = menuTitled(window, QStringLiteral("Ca&se"));
    REQUIRE(caseMenu != nullptr);
    CHECK(caseMenu->actions().size() == static_cast<qsizetype>(kLetterCases.size()));
}

TEST_CASE("the tab shortcuts live on the window and in no menu", "[gui][GUI-TABS-01]") {
    QMainWindow window;
    const WindowActions actions{&window};

    actions.placeIn(window, {});

    CHECK(window.actions().contains(actions.nextTab));
    CHECK(window.actions().contains(actions.previousTab));
    for (const QMenu* menu : window.menuBar()->findChildren<QMenu*>()) {
        CHECK_FALSE(menu->actions().contains(actions.nextTab));
        CHECK_FALSE(menu->actions().contains(actions.previousTab));
    }
}

TEST_CASE("the toolbar reads short words, and the menus the whole entry", "[gui][GUI-OPEN-01]") {
    QMainWindow window;
    const WindowActions actions{&window};

    actions.placeIn(window, {});

    const QList<QToolBar*> bars = window.findChildren<QToolBar*>();
    REQUIRE(bars.size() == 1);
    CHECK(bars.at(0)->toolButtonStyle() == Qt::ToolButtonTextBesideIcon);
    CHECK(actions.newProject->iconText() == QStringLiteral("New"));
    CHECK(actions.newProject->text() == QStringLiteral("&New Project"));
    CHECK(actions.playPause->iconText() == QStringLiteral("Play"));
}
