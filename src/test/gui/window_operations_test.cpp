// Ce que la fenêtre fait des trois dialogues — issue #132.
//
// Le faux `Prompts` joue l'utilisateur : il reçoit le dialogue, y écrit ce que
// le scénario veut, et dit s'il valide. La boucle modale n'est jamais atteinte.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/operation_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QString>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::StandardFrameRate;
using subedit::gui::FrameRateDialog;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::OperationDialog;
using subedit::gui::ShiftDialog;
using subedit::gui::TransformDialog;
using subedit::test::FakePrompts;

/// Quatre sous-titres à une, trois, cinq et sept secondes.
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

/// Le début d'une ligne, tel que la table le montre.
[[nodiscard]] std::string startAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, 1), Qt::DisplayRole)
        .toString()
        .toStdString();
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("shifting with nothing selected moves the whole file", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("00:00:01,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.shiftAction()->trigger();

    CHECK(startAt(window, 0) == "00:00:02,000");
    CHECK(startAt(window, 3) == "00:00:08,000");
    CHECK(window.undoAction()->text().toStdString() == "Undo: shifting");
}

TEST_CASE("shifting a selection moves only it", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("00:00:01,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};
    selectRow(window, 1);

    window.shiftAction()->trigger();

    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK(startAt(window, 1) == "00:00:04,000");
    CHECK(startAt(window, 2) == "00:00:05,000");
}

TEST_CASE("a dialog names the selection, and not the file", "[gui][GUI-SHIFT-01]") {
    // Le défaut que la relecture de fin de phase a trouvé : la fenêtre passait
    // le compte du fichier aux quatre dialogues, qui annonçaient « 4 subtitles »
    // pendant que l'opération en changeait deux. Les tests de dialogue le
    // construisaient à la main, donc aucun ne pouvait le voir.
    //
    // Lu depuis `fill`, pendant que le dialogue vit : il est sur la pile de la
    // fenêtre et ne survit pas au retour de l'action.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string said;
    prompts.fill = [&said](QDialog& dialog) {
        said = dynamic_cast<OperationDialog&>(dialog).targetLabel().toStdString();
    };
    const MainWindow window{files, fourIn(files), prompts};

    SECTION("nothing selected is the whole file") {
        window.shiftAction()->trigger();

        CHECK(said == "4 subtitles");
    }

    SECTION("two rows selected are two subtitles") {
        selectRow(window, 1);
        selectRow(window, 2);

        window.shiftAction()->trigger();

        CHECK(said == "2 subtitles");
    }

    SECTION("and the three others count the same way") {
        selectRow(window, 0);

        window.transformAction()->trigger();
        CHECK(said == "1 subtitle");

        window.frameRateAction()->trigger();
        CHECK(said == "1 subtitle");
    }
}

TEST_CASE("giving up on a dialog applies nothing", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false; // l'utilisateur a fermé la boîte
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("00:00:01,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.shiftAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("a shift that would go before the origin is refused, and names the subtitle",
          "[gui][GUI-SHIFT-01]") {
    // Une position négative est représentable — le noyau le dit — mais aucun
    // fichier de sous-titres ne peut la porter. Le refus nomme le premier
    // fautif, qui est celui qu'il faut regarder.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("-0:02,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.shiftAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.at(0).find("subtitle 1") != std::string::npos);
    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("transforming corrects the file from two references", "[gui][GUI-TRANSFORM-01]") {
    // Le premier reste où il est, le dernier va deux fois plus loin : tout
    // s'étire entre les deux, et les deux repères atterrissent exactement.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TransformDialog&>(dialog).setTyped(
            1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:13,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.transformAction()->trigger();

    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK(startAt(window, 3) == "00:00:13,000");
    CHECK(startAt(window, 1) == "00:00:05,000");
    CHECK(window.undoAction()->text().toStdString() == "Undo: transforming");
}

TEST_CASE("converting the frame rate re-times the file", "[gui][GUI-FRAMERATE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<FrameRateDialog&>(dialog).setRates(FrameRate{StandardFrameRate::Fps25},
                                                        FrameRate{StandardFrameRate::Fps24});
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.frameRateAction()->trigger();

    // Vingt-cinq images par seconde relues à vingt-quatre : tout dure plus
    // longtemps, dans le rapport 25/24.
    CHECK(startAt(window, 0) == "00:00:01,042");
    CHECK(window.undoAction()->text().toStdString() == "Undo: converting the frame rate");
}

TEST_CASE("the three operations are reachable from a menu of their own", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    const MainWindow window{files, fourIn(files), prompts};

    CHECK(window.shiftAction()->isEnabled());
    CHECK(window.transformAction()->isEnabled());
    CHECK(window.frameRateAction()->isEnabled());
}

TEST_CASE("an operation on an empty file is not offered", "[gui][GUI-SHIFT-01]") {
    // Rien à décaler, rien à transformer : les actions sont inactives plutôt
    // que d'ouvrir un dialogue qui ne pourrait porter sur rien.
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window{files, OpenedFile{}, prompts};

    CHECK_FALSE(window.shiftAction()->isEnabled());
    CHECK_FALSE(window.transformAction()->isEnabled());
    CHECK_FALSE(window.frameRateAction()->isEnabled());
}

TEST_CASE("giving up on the transform dialog applies nothing", "[gui][GUI-TRANSFORM-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    const MainWindow window{files, fourIn(files), prompts};

    window.transformAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("giving up on the frame rate dialog applies nothing", "[gui][GUI-FRAMERATE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    const MainWindow window{files, fourIn(files), prompts};

    window.frameRateAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

// Les trois cas suivants ne peuvent pas arriver à un utilisateur : le bouton de
// validation suit `isComplete()`, donc un dialogue incomplet ne se valide pas.
// Ce sont des gardes, et une garde qu'aucun test ne traverse est une promesse
// que personne ne vérifie — le faux `Prompts` valide sans regarder le bouton,
// ce qui est exactement la situation dont elles protègent.

TEST_CASE("a shift validated on an unreadable duration does nothing", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("bientôt"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.shiftAction()->trigger();

    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK(prompts.failures.empty());
}

TEST_CASE("a transform validated on an unreadable reference does nothing",
          "[gui][GUI-TRANSFORM-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TransformDialog&>(dialog).setTyped(
            1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("plus tard"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.transformAction()->trigger();

    CHECK(startAt(window, 3) == "00:00:07,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("two references on one subtitle are refused, and said so", "[gui][GUI-TRANSFORM-01]") {
    // Le noyau rend `nullopt` sur un dénominateur nul. La fenêtre le dit plutôt
    // que de ne rien faire sans raison apparente.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TransformDialog&>(dialog).setTyped(
            2, QStringLiteral("00:00:01,000"), 2, QStringLiteral("00:00:09,000"));
    };
    const MainWindow window{files, fourIn(files), prompts};

    window.transformAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.at(0).find("no correction") != std::string::npos);
    CHECK(startAt(window, 1) == "00:00:03,000");
}
