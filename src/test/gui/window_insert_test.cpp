// Insérer et supprimer des lignes depuis la fenêtre — issue #242.
//
// **Les deux commandes du noyau existent depuis la phase 2 et n'avaient aucune
// surface.** Ces cas sont leur première preuve de bout en bout : jusqu'ici
// `InsertCommand` et `RemoveCommand` étaient éprouvées seules, sans qu'aucun
// utilisateur puisse les déclencher.
//
// Le faux `Prompts` joue l'utilisateur : il reçoit le dialogue, y écrit ce que
// le scénario veut, et dit s'il est validé. La boucle modale n'est jamais
// atteinte.

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QDialog>
#include <QItemSelectionModel>
#include <QSpinBox>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::InsertPlacement;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::InsertDialog;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// Quatre sous-titres, à une, trois, cinq et sept secondes.
constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nQuatre.\n\n";

[[nodiscard]] InMemoryFileSystem withFour() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kFour);
    return files;
}

[[nodiscard]] OpenedFile fourIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// La colonne `Text`, la cinquième.
constexpr int kTextColumn = 4;

/// Le texte d'une ligne, tel que la table le montre.
[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, kTextColumn), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] int rowCount(const MainWindow& window) {
    return window.table()->model()->rowCount({});
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

/// Ce que l'utilisateur écrit dans le dialogue d'insertion.
[[nodiscard]] auto typing(int count, InsertPlacement placement) {
    return [count, placement](QDialog& dialog) {
        auto& insertion = dynamic_cast<InsertDialog&>(dialog);
        insertion.countBox()->setValue(count);
        insertion.setPlacement(placement);
    };
}

} // namespace

TEST_CASE("inserting places the lines after the last selected one", "[gui][GUI-INSERT-01]") {
    // **Le dernier et non le premier**, et la sélection est faite pour que les
    // deux ne se confondent pas : le premier donnerait l'index 1, le dernier
    // donne l'index 3. C'est le point qu'on invente mal sans lire Gaupol.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(1, InsertPlacement::Below);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);
    selectRow(window, 2);

    window.insertAction()->trigger();

    REQUIRE(rowCount(window) == 5);
    CHECK(textAt(window, 2) == "Trois.");
    CHECK(textAt(window, 3).empty());
    CHECK(textAt(window, 4) == "Quatre.");
    CHECK(window.undoAction()->text().toStdString() == "Undo: inserting");
}

TEST_CASE("inserting above places the lines before the last selected one", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(1, InsertPlacement::Above);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);
    selectRow(window, 2);

    window.insertAction()->trigger();

    REQUIRE(rowCount(window) == 5);
    CHECK(textAt(window, 1) == "Deux.");
    CHECK(textAt(window, 2).empty());
    CHECK(textAt(window, 3) == "Trois.");
}

TEST_CASE("inserting adds as many lines as were asked for", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(3, InsertPlacement::Below);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 3);

    window.insertAction()->trigger();

    REQUIRE(rowCount(window) == 7);
    CHECK(textAt(window, 3) == "Quatre.");
    CHECK(textAt(window, 4).empty());
    CHECK(textAt(window, 6).empty());
}

TEST_CASE("the inserted lines are selected, so it can be done again", "[gui][GUI-INSERT-01]") {
    // La table a été réinitialisée : sans cette sélection rendue, l'action
    // s'éteindrait et un second `Ins` ne ferait rien.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(1, InsertPlacement::Below);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);

    window.insertAction()->trigger();

    CHECK(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().first().row() == 1);
    CHECK(window.insertAction()->isEnabled());

    window.insertAction()->trigger();

    CHECK(rowCount(window) == 6);
}

TEST_CASE("a cancelled insertion adds nothing", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    prompts.fill = typing(2, InsertPlacement::Below);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.insertAction()->trigger();

    CHECK(rowCount(window) == 4);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("inserting can be undone", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(2, InsertPlacement::Below);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.insertAction()->trigger();
    REQUIRE(rowCount(window) == 6);

    window.undoAction()->trigger();

    CHECK(rowCount(window) == 4);
    CHECK(textAt(window, 2) == "Trois.");
}

TEST_CASE("with no selection, inserting is disabled while the document holds lines",
          "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    CHECK_FALSE(window.insertAction()->isEnabled());

    selectRow(window, 2);

    CHECK(window.insertAction()->isEnabled());
}

TEST_CASE("inserting into an empty document needs no selection", "[gui][GUI-INSERT-02]") {
    // La seule façon de commencer un fichier neuf : il n'y a rien à
    // sélectionner, donc exiger une sélection rendrait l'insertion impossible.
    InMemoryFileSystem files;
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(2, InsertPlacement::Below);
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    REQUIRE(window.insertAction()->isEnabled());
    window.insertAction()->trigger();

    CHECK(rowCount(window) == 2);
}

TEST_CASE("the chosen side is kept from one insertion to the next", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = typing(1, InsertPlacement::Above);
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 2);

    window.insertAction()->trigger();

    // Ce qui part aux réglages, et ce que le prochain dialogue montrera.
    CHECK(window.settings().insertPlacement == InsertPlacement::Above);

    InsertPlacement offered = InsertPlacement::Below;
    prompts.fill = [&offered](QDialog& dialog) {
        offered = dynamic_cast<InsertDialog&>(dialog).placement();
    };
    window.insertAction()->trigger();

    CHECK(offered == InsertPlacement::Above);
}

TEST_CASE("a side read from the settings is the one the dialog offers", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    MainWindow window{files, fourIn(files), prompts};
    window.applySettings(subedit::core::Settings{.insertPlacement = InsertPlacement::Above});
    window.show();
    selectRow(window, 1);

    InsertPlacement offered = InsertPlacement::Below;
    prompts.fill = [&offered](QDialog& dialog) {
        offered = dynamic_cast<InsertDialog&>(dialog).placement();
    };
    window.insertAction()->trigger();

    CHECK(offered == InsertPlacement::Above);
}

TEST_CASE("removing takes out the selection, and can be undone", "[gui][GUI-REMOVE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);
    selectRow(window, 2);

    window.removeAction()->trigger();

    REQUIRE(rowCount(window) == 2);
    CHECK(textAt(window, 0) == "Un.");
    CHECK(textAt(window, 1) == "Quatre.");
    CHECK(window.undoAction()->text().toStdString() == "Undo: removing");

    window.undoAction()->trigger();

    REQUIRE(rowCount(window) == 4);
    CHECK(textAt(window, 1) == "Deux.");
    CHECK(textAt(window, 2) == "Trois.");
}

TEST_CASE("removing asks nothing and shows no dialog", "[gui][GUI-REMOVE-01]") {
    // L'opération entre dans l'historique comme les autres : une modale devant
    // un geste annulable coûterait un clic à chaque fois.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);

    window.removeAction()->trigger();

    CHECK(prompts.runAsked == 0);
    CHECK(prompts.failures.empty());
}

TEST_CASE("with no selection, removing is disabled", "[gui][GUI-REMOVE-01]") {
    // Ce qui tient la règle : `targetOf` lit « rien de sélectionné » comme
    // « tout le fichier », ce qui serait ici un fichier vidé d'un `Suppr`.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    CHECK_FALSE(window.removeAction()->isEnabled());

    selectRow(window, 0);

    CHECK(window.removeAction()->isEnabled());
}

TEST_CASE("an empty removal does not enter the history", "[gui][GUI-REMOVE-01]") {
    // **Le second garde**, celui que l'entrée éteinte cache : une action
    // éteinte ne se déclenche pas, donc ce chemin demande de la rallumer à la
    // main pour être atteint. Il existe quand même, et ce qu'il empêche est
    // qu'un retrait de rien du tout se retrouve dans l'historique, à défaire.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.removeAction()->setEnabled(true);
    window.removeAction()->trigger();

    CHECK(rowCount(window) == 4);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("after a removal, the line that took the place is selected", "[gui][GUI-REMOVE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.removeAction()->trigger();

    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().first().row() == 1);

    // Et l'on peut donc recommencer, ce qui est tout l'intérêt.
    window.removeAction()->trigger();

    CHECK(rowCount(window) == 2);
}

TEST_CASE("removing the end of the file leaves the last line selected", "[gui][GUI-REMOVE-01]") {
    // `min(première retirée, dernière restante)` : la place laissée par la
    // dernière ligne d'un fichier n'existe plus une fois celle-ci retirée.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 3);

    window.removeAction()->trigger();

    REQUIRE(rowCount(window) == 3);
    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().first().row() == 2);
}

TEST_CASE("emptying the file leaves the window usable", "[gui][GUI-REMOVE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    for (int row = 0; row < 4; ++row)
        selectRow(window, row);

    window.removeAction()->trigger();

    CHECK(rowCount(window) == 0);
    // Vide, le document se remplit de nouveau sans sélection — et c'est le seul
    // cas où insérer reste possible sans en avoir une.
    CHECK(window.insertAction()->isEnabled());
    CHECK_FALSE(window.removeAction()->isEnabled());
}

TEST_CASE("both entries live in the Edit menu", "[gui][GUI-INSERT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    const MainWindow window{files, fourIn(files), prompts};

    CHECK(window.insertAction()->text().toStdString() == "Insert Subtitles…");
    CHECK(window.removeAction()->text().toStdString() == "Remove Subtitles");
    CHECK(window.insertAction()->shortcut() == QKeySequence{Qt::Key_Insert});
    CHECK(window.removeAction()->shortcut() == QKeySequence{QKeySequence::Delete});
}
