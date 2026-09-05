#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QKeySequence>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QString>
#include <QTableView>
#include <QTest>
#include <QToolBar>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::RealFileSystem;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::PositionDelegate;
using subedit::gui::TextDelegate;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:03,000 --> 00:00:04,000\n"
                               "Deux.\n"
                               "\n"
                               "3\n"
                               "00:00:05,000 --> 00:00:06,000\n"
                               "Trois.\n";

/// One subtitle of one line, one of two, one of three — what a height that
/// follows its text has to tell apart.
constexpr const char* kOneTwoThree = "1\n"
                                     "00:00:01,000 --> 00:00:02,000\n"
                                     "Un.\n"
                                     "\n"
                                     "2\n"
                                     "00:00:03,000 --> 00:00:04,000\n"
                                     "Deux.\n"
                                     "Et demi.\n"
                                     "\n"
                                     "3\n"
                                     "00:00:05,000 --> 00:00:06,000\n"
                                     "Trois.\n"
                                     "Et un tiers.\n"
                                     "Et un peu plus.\n";

[[nodiscard]] InMemoryFileSystem withFile(const std::string& path, const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(path, content);
    return files;
}

/// What a window has around it, held together.
///
/// It keeps references to the file system and to the questions put to the
/// user, so both have to outlive it. Leaving them in the body of the test would
/// work; gathering them here saves having to think about it.
class Windowed {

public:
    /// On a file, or on nothing at all.
    explicit Windowed(const char* content = nullptr) {
        if (content == nullptr) {
            m_window = std::make_unique<MainWindow>(m_files, OpenedFile{}, m_prompts);
            m_window->show();
            return;
        }

        m_files.addFile("film.srt", content);
        auto opened = openProject(m_files, "film.srt");
        REQUIRE(opened.has_value());
        m_window = std::make_unique<MainWindow>(m_files, std::move(*opened), m_prompts);
        m_window->show();
    }

    [[nodiscard]] MainWindow& window() const { return *m_window; }

private:
    InMemoryFileSystem m_files;
    subedit::test::FakePrompts m_prompts;
    std::unique_ptr<MainWindow> m_window;
};

} // namespace

TEST_CASE("opening a file gives a project that remembers where it came from",
          "[gui][GUI-OPEN-01]") {
    const InMemoryFileSystem files = withFile("film.srt", kThree);

    const std::expected<OpenedFile, subedit::core::OpenError> opened =
        openProject(files, "film.srt");

    REQUIRE(opened.has_value());
    CHECK(opened->project.count() == 3);
    CHECK(opened->project.sourceFile().format == SubtitleFormat::SubRip);
    // The whole optional rather than its content: clang-tidy does not
    // recognise Catch2's REQUIRE as a check.
    CHECK(opened->project.sourceFile().path == std::filesystem::path{"film.srt"});
}

TEST_CASE("a file of the corpus opens through the real file system", "[gui][GUI-OPEN-01]") {
    // **The one test of the window that touches a disk.** Every other one goes
    // through `InMemoryFileSystem` and a string written for the occasion, which
    // is the right way to put a window to the test — and left `openProject`
    // without a single proof on a file that exists, while the phase spec said
    // the corpus of `src/test/data/` served the opening.
    //
    // What only a real file can carry: bytes read by the platform, an encoding
    // the reader had to accept, and a path a project remembers.
    RealFileSystem files;
    const std::filesystem::path path =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "valides" / "trois.srt";

    std::expected<OpenedFile, subedit::core::OpenError> opened = openProject(files, path);

    REQUIRE(opened.has_value());
    CHECK(opened->diagnostics.empty());

    subedit::test::FakePrompts prompts;
    MainWindow window{files, std::move(*opened), prompts};
    window.show();

    const QAbstractItemModel* table = window.table()->model();
    REQUIRE(table->rowCount({}) == 3);
    CHECK(table->data(table->index(1, 1), Qt::DisplayRole).toString().toStdString() ==
          "00:00:05,001");
    CHECK(table->data(table->index(0, 4), Qt::DisplayRole).toString().toStdString() ==
          "Le vent se lève sur le port.");
    CHECK(window.windowTitle().toStdString() == "trois.srt[*] — subedit");
}

TEST_CASE("the window shows the subtitles of the project it was given", "[gui][GUI-OPEN-01]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    REQUIRE(window.table() != nullptr);
    REQUIRE(window.table()->model() != nullptr);
    CHECK(window.table()->model()->rowCount({}) == 3);
    CHECK(window.table()->model()->columnCount({}) == 5);
}

TEST_CASE("a window with no file shows an empty table", "[gui][GUI-OPEN-01]") {
    // Not a case to guard against: an empty project is a project, and the table
    // over it is empty rather than absent.
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    CHECK(window.table()->model()->rowCount({}) == 0);
}

TEST_CASE("the window names the file it holds", "[gui][GUI-OPEN-01]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    CHECK(window.windowTitle().toStdString().find("film.srt") != std::string::npos);
}

TEST_CASE("a row of the table is as tall as its subtitle", "[gui][GUI-OPEN-01]") {
    // **The three settings of #322, seen from where they show.** A row of one
    // line, a row of two, a row of three: what is asserted is not a number of
    // pixels — the font decides that — but that the second row is taller than
    // the first by as much as the third is taller than the second.
    const Windowed windowed{kOneTwoThree};
    QCoreApplication::processEvents();
    const subedit::gui::SubtitleTable& table = *windowed.window().table();

    const int one = table.rowHeight(0);
    const int two = table.rowHeight(1);
    const int three = table.rowHeight(2);

    CHECK(two > one);
    CHECK(three - two == two - one);
}

TEST_CASE("the table wraps on nothing but a real line break", "[gui][GUI-OPEN-01]") {
    // Word wrap off is what makes a height depend on the text alone and never
    // on the width of a column — so a drag of a column edge recomputes nothing,
    // and a line too long keeps eliding, which is the one thing an ellipsis
    // should mean. Scrolling by pixel is what a table of unequal rows needs:
    // by item, the bar sits where nobody put it.
    const Windowed windowed{kOneTwoThree};
    const subedit::gui::SubtitleTable& table = *windowed.window().table();

    CHECK_FALSE(table.wordWrap());
    CHECK(table.verticalScrollMode() == QAbstractItemView::ScrollPerPixel);
    CHECK(table.verticalHeader()->sectionResizeMode(0) == QHeaderView::ResizeToContents);
}

TEST_CASE("a row grows when its subtitle gains a line", "[gui][GUI-EDIT-01]") {
    // The recomputation, and where it comes from: nothing in the window asks
    // for it. `ResizeToContents` listens to the model, and the model announces
    // an edit the way it announces every other — so the row that grew is the
    // row that changed, and no other.
    const Windowed windowed{kOneTwoThree};
    QCoreApplication::processEvents();
    const subedit::gui::SubtitleTable& table = *windowed.window().table();
    const int before = table.rowHeight(0);
    const int untouched = table.rowHeight(2);

    QAbstractItemModel* model = table.model();
    REQUIRE(model->setData(model->index(0, 4), QStringLiteral("Un.\nEt demi."), Qt::EditRole));
    QCoreApplication::processEvents();

    CHECK(table.rowHeight(0) > before);
    CHECK(table.rowHeight(2) == untouched);
}

TEST_CASE("the window puts an editor on the three cells that can be edited", "[gui][GUI-EDIT-01]") {
    const Windowed fixture;
    const MainWindow& window = fixture.window();
    const QTableView* table = window.table();

    // The number and the duration have none: they are not editable, and a
    // delegate put there would promise otherwise.
    CHECK(table->itemDelegateForColumn(0) == nullptr);
    CHECK(qobject_cast<PositionDelegate*>(table->itemDelegateForColumn(1)) != nullptr);
    CHECK(qobject_cast<PositionDelegate*>(table->itemDelegateForColumn(2)) != nullptr);
    CHECK(table->itemDelegateForColumn(3) == nullptr);
    CHECK(qobject_cast<TextDelegate*>(table->itemDelegateForColumn(4)) != nullptr);
}

TEST_CASE("typing in a cell of the window changes the file it holds", "[gui][GUI-EDIT-01]") {
    // The one test that walks the whole chain — view, delegate, model,
    // command, session — and the only one that shows what the user does.
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();
    QTableView* table = window.table();
    const QModelIndex cell = table->model()->index(1, 4);
    table->setCurrentIndex(cell);
    table->edit(cell);

    auto* editor = table->findChild<QPlainTextEdit*>();
    REQUIRE(editor != nullptr);
    editor->setPlainText(QStringLiteral("Deux, autrement."));
    QTest::keyClick(editor, Qt::Key_Return);

    CHECK(table->model()->data(cell, Qt::DisplayRole).toString().toStdString() ==
          "Deux, autrement.");
}

// Undo and redo — issue #130.
//
// No `QUndoStack`: the core's history is the authority, since the command line
// depends on it too, and two sources of truth for one question would be one too
// many. The two `QAction` only read it.

namespace {

/// What a cell holds, seen from the window.
[[nodiscard]] std::string cell(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] bool edits(const MainWindow& window, int row, int column, const char* typed) {
    return window.table()->model()->setData(
        window.table()->model()->index(row, column), QString::fromUtf8(typed), Qt::EditRole);
}

} // namespace

TEST_CASE("with nothing done, both actions are inactive", "[gui][GUI-UNDO-01]") {
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK_FALSE(window.redoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo");
    CHECK(window.redoAction()->text().toStdString() == "Redo");
}

TEST_CASE("an edit makes undo possible, and names it", "[gui][GUI-UNDO-01]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    REQUIRE(edits(window, 0, 4, "Autre chose."));

    CHECK(window.undoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo: editing a text");
    CHECK_FALSE(window.redoAction()->isEnabled());

    // The long label in the menu, the short one on the toolbar — and now is
    // when it can be checked, the two having finally diverged. A button whose
    // width followed the last operation would move under the pointer.
    CHECK(window.undoAction()->iconText().toStdString() == "Undo");
    CHECK(window.undoAction()->toolTip().toStdString() == "Undo: editing a text");
}

TEST_CASE("undo and redo walk a run of edits both ways", "[gui][GUI-UNDO-02]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();
    REQUIRE(edits(window, 0, 4, "Un bis."));
    REQUIRE(edits(window, 1, 4, "Deux bis."));
    REQUIRE(edits(window, 0, 1, "00:00:09,000"));

    window.undoAction()->trigger();
    window.undoAction()->trigger();
    window.undoAction()->trigger();

    CHECK(cell(window, 0, 4) == "Un.");
    CHECK(cell(window, 1, 4) == "Deux.");
    CHECK(cell(window, 0, 1) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());

    window.redoAction()->trigger();
    window.redoAction()->trigger();
    window.redoAction()->trigger();

    CHECK(cell(window, 0, 4) == "Un bis.");
    CHECK(cell(window, 1, 4) == "Deux bis.");
    CHECK(cell(window, 0, 1) == "00:00:09,000");
    CHECK_FALSE(window.redoAction()->isEnabled());
}

TEST_CASE("the redo action names what it would replay", "[gui][GUI-UNDO-02]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();
    REQUIRE(edits(window, 0, 1, "00:00:09,000"));

    window.undoAction()->trigger();

    CHECK(window.redoAction()->isEnabled());
    CHECK(window.redoAction()->text().toStdString() == "Redo: editing a start");
}

TEST_CASE("the window carries the mark of unsaved changes", "[gui][GUI-UNDO-01]") {
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    CHECK_FALSE(window.isWindowModified());

    REQUIRE(edits(window, 0, 4, "Autre chose."));

    CHECK(window.isWindowModified());
}

TEST_CASE("undoing back to the save point clears the mark", "[gui][GUI-UNDO-02]") {
    // What a boolean could never have done: the core counts the changes, so
    // coming back to the save point can be told. The point, here, is the
    // opening — « Save » does not exist yet.
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();
    REQUIRE(edits(window, 0, 4, "Un bis."));
    REQUIRE(edits(window, 1, 4, "Deux bis."));
    REQUIRE(window.isWindowModified());

    window.undoAction()->trigger();
    CHECK(window.isWindowModified());

    window.undoAction()->trigger();
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("a validation that changed nothing leaves the actions alone", "[gui][GUI-UNDO-01]") {
    // The state is recomputed after every operation, the one that changed
    // nothing included — and it has to come out right.
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    REQUIRE(edits(window, 0, 4, "Un."));

    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("both actions are reachable from the menu and the toolbar", "[gui][GUI-UNDO-01]") {
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    // Found by its title and not by its rank: every interface ticket adds a
    // menu, and counting positions would fail this test every time without
    // anything being broken.
    const QList<QMenu*> menus = window.menuBar()->findChildren<QMenu*>();
    const auto edit = std::ranges::find_if(
        menus, [](const QMenu* menu) { return menu->title() == QStringLiteral("&Edit"); });
    REQUIRE(edit != menus.end());
    CHECK((*edit)->actions().contains(window.undoAction()));
    CHECK((*edit)->actions().contains(window.redoAction()));

    const QList<QToolBar*> bars = window.findChildren<QToolBar*>();
    REQUIRE(bars.size() == 1);
    CHECK(bars.at(0)->actions().contains(window.undoAction()));
    CHECK(bars.at(0)->actions().contains(window.redoAction()));

    CHECK(window.undoAction()->shortcut() == QKeySequence::Undo);
    CHECK(window.redoAction()->shortcut() == QKeySequence::Redo);
}

TEST_CASE("every shortcut a desktop gives to redo is live", "[gui][GUI-UNDO-01]") {
    // **Ce que `QKeySequence` rend dépend du thème de plateforme, et ce binaire
    // n'en a aucun** — issue #274. Sous `offscreen`, Qt retombe sur sa table
    // interne et met `Ctrl+Y` en tête ; sous n'importe quel bureau, le thème
    // donne `Ctrl+Maj+Z` et lui seul. `setShortcut` n'en retenait que la
    // première, donc la fenêtre répondait à un raccourci différent selon
    // l'endroit — et ce test-ci n'aurait vu que celui que l'utilisateur n'a pas.
    //
    // Ce qui est éprouvé est donc l'invariant qui vaut des deux côtés : la
    // liaison que tout bureau Linux donne est vivante.
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    CHECK(window.redoAction()->shortcuts().contains(QKeySequence{QStringLiteral("Ctrl+Shift+Z")}));
}

TEST_CASE("save as always carries a shortcut, whatever the platform gives", "[gui][GUI-SAVE-02]") {
    // **La table interne de Qt ne définit `SaveAs` que pour macOS et Windows.**
    // Tout thème de bureau donne `Ctrl+Maj+S` — mesuré sous xcb, sous wayland,
    // et sous `offscreen` dès qu'un thème est posé —, mais sans thème la
    // fenêtre n'avait aucun raccourci pour une commande qui écrit un fichier.
    //
    // La liaison conventionnelle est ajoutée quand la plateforme se tait, ce
    // qui rend l'invariant vrai des deux côtés : jamais vide, et toujours celle
    // que le bureau aurait donnée.
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    const QList<QKeySequence> shortcuts = window.saveAsAction()->shortcuts();

    CHECK_FALSE(shortcuts.isEmpty());
    CHECK(shortcuts.contains(QKeySequence{QStringLiteral("Ctrl+Shift+S")}));
}
