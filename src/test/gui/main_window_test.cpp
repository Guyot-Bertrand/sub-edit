#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QKeySequence>
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
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
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

[[nodiscard]] InMemoryFileSystem withFile(const std::string& path, const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(path, content);
    return files;
}

/// Ce qu'une fenêtre a autour d'elle, tenu ensemble.
///
/// Elle garde des références sur le système de fichiers et sur les questions
/// posées à l'utilisateur, donc les deux doivent lui survivre. Les laisser
/// dans le corps du test marcherait ; les réunir ici évite d'y penser.
class Windowed {

public:
    /// Sur un fichier, ou sur rien du tout.
    explicit Windowed(const char* content = nullptr) {
        if (content == nullptr) {
            m_window = std::make_unique<MainWindow>(m_files, OpenedFile{}, m_prompts);
            return;
        }

        m_files.addFile("film.srt", content);
        auto opened = openProject(m_files, "film.srt");
        REQUIRE(opened.has_value());
        m_window = std::make_unique<MainWindow>(m_files, std::move(*opened), m_prompts);
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

    const std::expected<OpenedFile, ReadError> opened = openProject(files, "film.srt");

    REQUIRE(opened.has_value());
    CHECK(opened->project.count() == 3);
    CHECK(opened->project.sourceFile().format == SubtitleFormat::SubRip);
    // L'option entière plutôt que son contenu : clang-tidy ne reconnaît pas le
    // REQUIRE de Catch2 comme une vérification.
    CHECK(opened->project.sourceFile().path == std::filesystem::path{"film.srt"});
}

TEST_CASE("opening what is not a subtitle file fails, and says why", "[gui][GUI-OPEN-02]") {
    const InMemoryFileSystem files = withFile("film.srt", "rien de reconnaissable\n");

    const std::expected<OpenedFile, ReadError> opened = openProject(files, "film.srt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(opened.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("opening a file that is not there fails", "[gui][GUI-OPEN-02]") {
    const InMemoryFileSystem files;

    CHECK_FALSE(openProject(files, "absent.srt").has_value());
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

TEST_CASE("the window puts an editor on the three cells that can be edited", "[gui][GUI-EDIT-01]") {
    const Windowed fixture;
    const MainWindow& window = fixture.window();
    const QTableView* table = window.table();

    // Le numéro et la durée n'en ont pas : ils ne sont pas éditables, et un
    // délégué posé là promettrait le contraire.
    CHECK(table->itemDelegateForColumn(0) == nullptr);
    CHECK(qobject_cast<PositionDelegate*>(table->itemDelegateForColumn(1)) != nullptr);
    CHECK(qobject_cast<PositionDelegate*>(table->itemDelegateForColumn(2)) != nullptr);
    CHECK(table->itemDelegateForColumn(3) == nullptr);
    CHECK(qobject_cast<TextDelegate*>(table->itemDelegateForColumn(4)) != nullptr);
}

TEST_CASE("typing in a cell of the window changes the file it holds", "[gui][GUI-EDIT-01]") {
    // Le seul test qui parcourt la chaîne entière — vue, délégué, modèle,
    // commande, session —, et le seul qui montre ce que l'utilisateur fait.
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

// Annuler et rétablir — issue #130.
//
// Pas de `QUndoStack` : l'historique du noyau fait autorité, puisque la ligne
// de commande en dépend aussi, et deux sources de vérité pour la même question
// en font une de trop. Les deux `QAction` ne font que le lire.

namespace {

/// Ce qu'une cellule vaut, vu de la fenêtre.
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

    // Le libellé long au menu, le court à la barre d'outils — et c'est
    // maintenant qu'on peut le vérifier, les deux ayant enfin divergé. Un
    // bouton dont la largeur suivrait la dernière opération bougerait sous le
    // pointeur.
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
    // Ce qu'un booléen n'aurait jamais su faire : le noyau compte les
    // modifications, donc revenir au point d'enregistrement se dit. Le point,
    // ici, est l'ouverture — « Enregistrer » n'existe pas encore.
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
    // L'état se recalcule après chaque opération, y compris après celle qui
    // n'a rien changé — et il faut qu'il tombe juste.
    const Windowed fixture{kThree};
    const MainWindow& window = fixture.window();

    REQUIRE(edits(window, 0, 4, "Un."));

    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("both actions are reachable from the menu and the toolbar", "[gui][GUI-UNDO-01]") {
    const Windowed fixture;
    const MainWindow& window = fixture.window();

    // Trouvé par son titre et non par son rang : chaque ticket d'interface
    // ajoute un menu, et compter les positions ferait échouer ce test à chaque
    // fois sans que rien ne soit cassé.
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
