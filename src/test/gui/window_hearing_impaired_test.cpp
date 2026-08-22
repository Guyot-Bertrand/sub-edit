// Le retrait des mentions depuis la fenêtre — issue #133.
//
// **Le seul chemin de la phase qui fasse disparaître des lignes**, donc le seul
// qui exerce le retrait puis la ré-insertion à l'annulation.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::OperationDialog;
using subedit::test::FakePrompts;

/// Un fichier réel en miniature : un sous-titre net, un qui porte une mention
/// au milieu, un qui n'est que mention, un net.
constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nAttends [il tousse] Marie\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\n[Bruit de pas]\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nAu revoir.\n\n";

/// Aucune mention nulle part.
constexpr const char* kClean = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

[[nodiscard]] InMemoryFileSystem withFile(const char* content) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, 4), Qt::DisplayRole)
        .toString()
        .toStdString();
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("removing mentions cleans what it can and takes away what it empties",
          "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    const MainWindow window{files, fileIn(files), prompts};

    window.hearingImpairedAction()->trigger();

    REQUIRE(window.table()->model()->rowCount({}) == 3);
    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "Attends Marie");
    CHECK(textAt(window, 2) == "Au revoir.");
}

TEST_CASE("the removal reports what it did", "[gui][GUI-HEARING-02]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    const MainWindow window{files, fileIn(files), prompts};

    window.hearingImpairedAction()->trigger();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "1 subtitle cleaned, 1 removed");
}

TEST_CASE("removing mentions from a selection leaves the rest alone", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    const MainWindow window{files, fileIn(files), prompts};
    selectRow(window, 1);

    window.hearingImpairedAction()->trigger();

    // Le troisième aurait été vidé, mais il n'était pas visé.
    REQUIRE(window.table()->model()->rowCount({}) == 4);
    CHECK(textAt(window, 1) == "Attends Marie");
    CHECK(textAt(window, 2) == "[Bruit de pas]");
}

TEST_CASE("a removal that changes nothing says so and stays out of the history",
          "[gui][GUI-HEARING-02]") {
    // Une opération qui ne change rien n'est pas une opération à annuler.
    InMemoryFileSystem files = withFile(kClean);
    FakePrompts prompts;
    prompts.nextRun = true;
    const MainWindow window{files, fileIn(files), prompts};

    window.hearingImpairedAction()->trigger();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "no mention to remove");
    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("giving up on the confirmation removes nothing", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = false;
    const MainWindow window{files, fileIn(files), prompts};

    window.hearingImpairedAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK(window.table()->model()->rowCount({}) == 4);
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("undoing a removal puts the subtitles back, with their own text",
          "[gui][GUI-HEARING-01]") {
    // Ce que l'ADR 0019 assume : un changement de structure réinitialise la
    // table. Ce qu'il ne coûte pas : le contenu, qui revient intact.
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    const MainWindow window{files, fileIn(files), prompts};
    window.hearingImpairedAction()->trigger();
    REQUIRE(window.table()->model()->rowCount({}) == 3);

    window.undoAction()->trigger();

    REQUIRE(window.table()->model()->rowCount({}) == 4);
    CHECK(textAt(window, 1) == "Attends [il tousse] Marie");
    CHECK(textAt(window, 2) == "[Bruit de pas]");
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("the confirmation names the selection, and not the file", "[gui][GUI-HEARING-01]") {
    // La cible **est** la question de ce dialogue : il n'a rien d'autre à
    // demander. Un compte qui annonce le fichier entier pendant que l'opération
    // porte sur deux lignes fait dire à la confirmation le contraire de ce
    // qu'elle confirme.
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string said;
    prompts.fill = [&said](QDialog& dialog) {
        said = dynamic_cast<OperationDialog&>(dialog).targetLabel().toStdString();
    };
    const MainWindow window{files, fileIn(files), prompts};
    selectRow(window, 1);
    selectRow(window, 2);

    window.hearingImpairedAction()->trigger();

    CHECK(said == "2 subtitles");
}

TEST_CASE("the removal is not offered on an empty file", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window{files, OpenedFile{}, prompts};

    CHECK_FALSE(window.hearingImpairedAction()->isEnabled());
}
