// Making the ranges disjoint from the window: the action, and the single box
// that offers the corrections when a file is opened with something wrong —
// issue #698.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/disjoint_dialog.hpp>
#include <subedit/gui/fixes_proposal_dialog.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QItemSelectionModel>
#include <QSpinBox>
#include <QStatusBar>
#include <QString>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::DisjointChoice;
using subedit::gui::DisjointDialog;
using subedit::gui::FixesProposalDialog;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// A [0, 5] and B [3, 8]: one overlap, in order.
constexpr const char* kOverlapping = "1\n00:00:00,000 --> 00:00:05,000\nA\n\n"
                                     "2\n00:00:03,000 --> 00:00:08,000\nB\n\n";

/// The same two, the last one first: out of order, and overlapping once sorted.
constexpr const char* kBoth = "1\n00:00:03,000 --> 00:00:08,000\nB\n\n"
                              "2\n00:00:00,000 --> 00:00:05,000\nA\n\n";

constexpr const char* kClean = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:05,000 --> 00:00:06,000\nCinq.\n\n";

/// Out of order and not overlapping, once sorted.
constexpr const char* kOnlyOutOfOrder = "1\n00:00:05,000 --> 00:00:06,000\nCinq.\n\n"
                                        "2\n00:00:01,000 --> 00:00:02,000\nUn.\n\n";

[[nodiscard]] OpenedFile read(const InMemoryFileSystem& files, const char* name) {
    auto opened = openProject(files, name);
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

[[nodiscard]] int rowsOf(const MainWindow& window) {
    return window.table()->model()->rowCount();
}

} // namespace

TEST_CASE("the dialog hands back what its fields say", "[gui][GUI-DISJOINT-01]") {
    const DisjointDialog dialog{DisjointChoice{.shortestMs = 120, .mostLines = 3, .dashes = true}};

    CHECK(dialog.choice() == DisjointChoice{.shortestMs = 120, .mostLines = 3, .dashes = true});

    dialog.shortestBox()->setValue(0);
    dialog.linesBox()->setValue(0);
    dialog.dashesCheck()->setChecked(false);
    CHECK(dialog.choice() == DisjointChoice{.shortestMs = 0, .mostLines = 0, .dashes = false});
}

TEST_CASE("making the ranges disjoint cuts what overlaps, selects it, and can be undone",
          "[gui][GUI-DISJOINT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kOverlapping);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    // In order, and overlapping once: the box at opening offers it, and is
    // declined, so that the action can be tried.
    REQUIRE(prompts.runAsked == 0);
    REQUIRE(window.makeDisjointAction()->isEnabled());
    prompts.nextRun = true;

    window.makeDisjointAction()->trigger();

    CHECK(rowsOf(window) == 3);
    CHECK(textAt(window, 0) == "A");
    CHECK(textAt(window, 1) == "A\nB");
    CHECK(textAt(window, 2) == "B");
    CHECK(window.table()->selectionModel()->selectedRows().size() == 3);
    CHECK(window.statusBar()->currentMessage().toStdString() == "2 subtitles split, 1 merged");

    window.undoAction()->trigger();
    CHECK(rowsOf(window) == 2);

    // A second run, on a file that no longer overlaps, finds nothing to do.
    window.redoAction()->trigger();
    window.makeDisjointAction()->trigger();
    CHECK(window.statusBar()->currentMessage().toStdString() == "already disjoint");
}

TEST_CASE("the settings of the dialog reach the operation, and are offered again",
          "[gui][GUI-DISJOINT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kOverlapping);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& disjoint = dynamic_cast<DisjointDialog&>(dialog);
        disjoint.dashesCheck()->setChecked(true);
    };

    window.makeDisjointAction()->trigger();

    CHECK(textAt(window, 1) == "- A\n- B");

    // The second use opens on what the first asked for.
    window.undoAction()->trigger();
    bool dashesOffered = false;
    prompts.fill = [&dashesOffered](QDialog& dialog) {
        dashesOffered = dynamic_cast<DisjointDialog&>(dialog).dashesCheck()->isChecked();
    };
    window.makeDisjointAction()->trigger();
    CHECK(dashesOffered);
}

TEST_CASE("cancelling the dialog changes nothing", "[gui][GUI-DISJOINT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kOverlapping);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    prompts.nextRun = false;

    window.makeDisjointAction()->trigger();

    CHECK(rowsOf(window) == 2);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("a project out of order is refused, and told to be sorted first",
          "[gui][GUI-DISJOINT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kOnlyOutOfOrder);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    const int asked = prompts.runAsked;

    window.makeDisjointAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front().find("sort them first") != std::string::npos);
    CHECK(prompts.runAsked == asked);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("what could not be done in full is said in a box to close", "[gui][GUI-DISJOINT-01]") {
    // Three subtitles on one range with a limit of two lines: shown in windows.
    InMemoryFileSystem files;
    files.addFile("film.srt",
                  "1\n00:00:00,000 --> 00:00:03,000\nA\n\n"
                  "2\n00:00:00,000 --> 00:00:03,000\nB\n\n"
                  "3\n00:00:00,000 --> 00:00:03,000\nC\n\n");
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    prompts.nextRun = true;

    window.makeDisjointAction()->trigger();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front().find("shown in windows") != std::string::npos);
    CHECK(textAt(window, 0) == "A\nB");
    CHECK(textAt(window, 1) == "B\nC");
}

TEST_CASE("a file out of order that also overlaps is offered both corrections in one box",
          "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("both.srt", kBoth);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.nextFileToOpen = "both.srt";
    std::string shown;
    prompts.fill = [&shown](QDialog& dialog) {
        shown = dynamic_cast<FixesProposalDialog&>(dialog).message().toStdString();
    };
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    // One box, and the two problems in it.
    CHECK(prompts.runAsked == 1);
    CHECK(shown.find("out of place") != std::string::npos);
    CHECK(shown.find("start before the previous one ends") != std::string::npos);
    // Sorted, then cut: A, A and B, B.
    CHECK(rowsOf(window) == 3);
    CHECK(textAt(window, 0) == "A");
    CHECK(textAt(window, 1) == "A\nB");
    CHECK(textAt(window, 2) == "B");

    // One account of both.
    const std::string said = window.statusBar()->currentMessage().toStdString();
    CHECK(said.find("moved") != std::string::npos);
    CHECK(said.find("split") != std::string::npos);
}

TEST_CASE("either correction can be left out", "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("both.srt", kBoth);

    SECTION("only the sort") {
        FakePrompts prompts;
        prompts.nextRun = true;
        prompts.nextFileToOpen = "both.srt";
        prompts.fill = [](QDialog& dialog) {
            dynamic_cast<FixesProposalDialog&>(dialog).disjointCheck()->setChecked(false);
        };
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(rowsOf(window) == 2);
        CHECK(textAt(window, 0) == "A");
        CHECK(textAt(window, 1) == "B");
    }

    SECTION("only the disjoint ranges, which needs the sort") {
        FakePrompts prompts;
        prompts.nextRun = true;
        prompts.nextFileToOpen = "both.srt";
        bool greyed = false;
        prompts.fill = [&greyed](QDialog& dialog) {
            auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
            proposal.sortCheck()->setChecked(false);
            greyed =
                !proposal.disjointCheck()->isEnabled() && !proposal.disjointCheck()->isChecked();
        };
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        // Nothing wanted that can be done: the file is as it came.
        CHECK(greyed);
        CHECK(textAt(window, 0) == "B");
        CHECK_FALSE(window.undoAction()->isEnabled());
    }

    SECTION("the sort back on gives the case back") {
        const FixesProposalDialog dialog{0, 2, 1};
        dialog.sortCheck()->setChecked(false);
        CHECK_FALSE(dialog.disjointCheck()->isEnabled());

        dialog.sortCheck()->setChecked(true);
        CHECK(dialog.disjointCheck()->isEnabled());
    }
}

TEST_CASE("a file that only overlaps is offered only the disjoint ranges",
          "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("over.srt", kOverlapping);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.nextFileToOpen = "over.srt";
    bool onlyDisjoint = false;
    prompts.fill = [&onlyDisjoint](QDialog& dialog) {
        auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
        onlyDisjoint = proposal.sortCheck() == nullptr && proposal.disjointCheck() != nullptr;
    };
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    CHECK(onlyDisjoint);
    CHECK(rowsOf(window) == 3);
}

TEST_CASE("a file that only needs the sort is offered nothing about overlaps",
          "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("late.srt", kOnlyOutOfOrder);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.nextFileToOpen = "late.srt";
    bool onlySort = false;
    prompts.fill = [&onlySort](QDialog& dialog) {
        auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
        onlySort = proposal.sortCheck() != nullptr && proposal.disjointCheck() == nullptr;
    };
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    CHECK(onlySort);
    CHECK(textAt(window, 0) == "Un.");
}

TEST_CASE("keeping the file as it is changes nothing, and a clean file asks nothing",
          "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("both.srt", kBoth);
    files.addFile("clean.srt", kClean);

    SECTION("a no") {
        FakePrompts prompts;
        prompts.nextRun = false;
        prompts.nextFileToOpen = "both.srt";
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(prompts.runAsked == 1);
        CHECK(textAt(window, 0) == "B");
        CHECK_FALSE(window.undoAction()->isEnabled());
    }

    SECTION("nothing to correct") {
        FakePrompts prompts;
        prompts.nextFileToOpen = "clean.srt";
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(prompts.runAsked == 0);
    }

    SECTION("both unticked") {
        FakePrompts prompts;
        prompts.nextRun = true;
        prompts.nextFileToOpen = "both.srt";
        prompts.fill = [](QDialog& dialog) {
            auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
            proposal.sortCheck()->setChecked(false);
        };
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK_FALSE(window.undoAction()->isEnabled());
    }
}

namespace {

/// One text in three pieces that follow each other, and another over the whole
/// range, given out of order: the third starts before the second.
constexpr const char* kPieces = "1\n00:00:47,240 --> 00:00:47,520\nAlpha\n\n"
                                "2\n00:00:47,520 --> 00:00:50,080\nAlpha\n\n"
                                "3\n00:00:47,240 --> 00:00:52,000\nBeta\n\n"
                                "4\n00:00:50,080 --> 00:00:52,000\nAlpha\n\n";

} // namespace

TEST_CASE("joining the repeated subtitles makes one of the pieces, and can be undone",
          "[gui][GUI-JOIN-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kPieces);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    REQUIRE(window.joinRepeatsAction()->isEnabled());

    window.joinRepeatsAction()->trigger();

    CHECK(rowsOf(window) == 2);
    CHECK(textAt(window, 0) == "Alpha");
    CHECK(textAt(window, 1) == "Beta");
    CHECK(window.statusBar()->currentMessage().toStdString() == "2 repeated subtitles joined");

    window.undoAction()->trigger();
    CHECK(rowsOf(window) == 4);

    window.redoAction()->trigger();
    window.joinRepeatsAction()->trigger();
    CHECK(window.statusBar()->currentMessage().toStdString() == "no repeated subtitles");
}

TEST_CASE("the box counts each problem after the corrections above it", "[gui][GUI-DISJOINT-02]") {
    // Joined, the file is in order: the sort is not offered, though the file as
    // it came is out of order.
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("pieces.srt", kPieces);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.nextFileToOpen = "pieces.srt";
    bool joinOnly = false;
    std::string shown;
    prompts.fill = [&joinOnly, &shown](QDialog& dialog) {
        auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
        joinOnly = proposal.joinCheck() != nullptr && proposal.sortCheck() == nullptr &&
                   proposal.disjointCheck() != nullptr;
        shown = proposal.message().toStdString();
    };
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    CHECK(joinOnly);
    CHECK(prompts.runAsked == 1);
    CHECK(shown.find("2 subtitles repeat one that they touch") != std::string::npos);
    // Joined, then the two texts that share one range are one subtitle.
    CHECK(rowsOf(window) == 1);
    CHECK(textAt(window, 0) == "Alpha\nBeta");
    const std::string said = window.statusBar()->currentMessage().toStdString();
    CHECK(said.find("2 repeated subtitles joined") != std::string::npos);
    CHECK(said.find("split") != std::string::npos);
}

TEST_CASE("the repeats can be left out of the box", "[gui][GUI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kClean);
    files.addFile("pieces.srt", kPieces);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.nextFileToOpen = "pieces.srt";
    prompts.fill = [](QDialog& dialog) {
        auto& proposal = dynamic_cast<FixesProposalDialog&>(dialog);
        proposal.joinCheck()->setChecked(false);
        proposal.disjointCheck()->setChecked(false);
    };
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    // Nothing ticked: the file is as it came.
    CHECK(rowsOf(window) == 4);
    CHECK_FALSE(window.undoAction()->isEnabled());
}
