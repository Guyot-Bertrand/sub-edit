// What the window says of the grid the positions were written on — issue #208.
//
// Two surfaces, and they answer different questions. The status bar carries the
// answer, standing and unasked; the analysis carries the working, and opens
// only when someone wants it.
//
// **Neither marks a row of the table**, which is the rule phase 5 gave itself
// and this phase keeps: the moment a user corrects a position by hand it stops
// being aligned, and a naive detector would accuse them of their own work.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/snap_dialog.hpp>

#include <QAction>
#include <QDialog>
#include <QLabel>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <grid_fixtures.hpp>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::deduceFrameRate;
using subedit::core::FrameRateDeduction;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::GridAnalysisDialog;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// A window showing the grid fixture named, opened as a user would open it.
[[nodiscard]] MainWindow
windowOn(std::string_view fixture, InMemoryFileSystem& files, FakePrompts& prompts) {
    files.addFile("a.srt", subedit::test::gridBytes(fixture));
    return MainWindow{files, openProject(files, "a.srt").value(), prompts, {}, {}};
}

/// A file on a 25 fps grid offset by five milliseconds, whose **first** start
/// was moved by hand to sit nearer the origin than that offset.
///
/// It takes a partial file to reach the refusal: on a clean one the smallest
/// start is itself a grid line plus the phase, so shifting back by the phase
/// lands it exactly on the origin and never before.
[[nodiscard]] std::string nearTheOrigin() {
    const auto stamp = [](std::int64_t ms) {
        std::ostringstream text;
        text << std::setfill('0') << std::setw(2) << ms / 3'600'000 << ':' << std::setw(2)
             << ms / 60'000 % 60 << ':' << std::setw(2) << ms / 1'000 % 60 << ',' << std::setw(3)
             << ms % 1'000;
        return text.str();
    };

    std::string content;
    for (int index = 1; index <= 20; ++index) {
        const std::int64_t start = index == 1 ? 2 : (index * 2'000) + 5;
        content += std::to_string(index) + "\n" + stamp(start) + " --> " + stamp(start + 1'000) +
                   "\nLigne.\n\n";
    }
    return content;
}

[[nodiscard]] FrameRateDeduction deductionOf(std::string_view fixture) {
    return deduceFrameRate(subedit::test::gridStarts(fixture));
}

} // namespace

TEST_CASE("the status bar names the grid the file was written on", "[gui][GUI-GRID-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    CHECK(window.gridStatus()->text().toStdString() == "Grid: 24 fps");
}

TEST_CASE("a partial grid is named as partial", "[gui][GUI-GRID-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("melange-groupe.srt", files, prompts);
    window.show();

    CHECK_THAT(window.gridStatus()->text().toStdString(), ContainsSubstring("(partial)"));
}

TEST_CASE("a file on no known grid says so, and names no rate", "[gui][GUI-GRID-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-absurde.srt", files, prompts);
    window.show();

    CHECK(window.gridStatus()->text().toStdString() == "No grid");
}

TEST_CASE("the analysis shows the eight candidates in order", "[gui][GUI-GRID-02]") {
    const GridAnalysisDialog dialog{deductionOf("grille-25.srt")};

    CHECK(dialog.candidateCount() == 8);
    // A grid at 25 is included in a grid at 50, so both reach full marks and the
    // lower one is retained. The reader sees the pair rather than a lone answer.
    CHECK_THAT(dialog.candidateAt(0).toStdString(), ContainsSubstring("100.0%"));
    CHECK_THAT(dialog.candidateAt(1).toStdString(), ContainsSubstring("100.0%"));
}

TEST_CASE("the analysis says what qualifies the answer", "[gui][GUI-GRID-02]") {
    const GridAnalysisDialog dialog{deductionOf("grille-24-courte.srt")};

    const std::string said = dialog.summary().toStdString();
    CHECK_THAT(said, ContainsSubstring("24 fps grid"));
    // Ten seconds cannot tell 24 from 24000/1001, and answering without saying
    // so would be lying by omission.
    CHECK_THAT(said, ContainsSubstring("too short to tell it from 24000/1001 fps"));
}

TEST_CASE("the analysis counts the starts that left the grid", "[gui][GUI-GRID-02]") {
    const GridAnalysisDialog dialog{deductionOf("melange-disperse.srt")};

    // As many runs as strays: positions corrected one at a time, which is not
    // at all the same story as a retimed section.
    CHECK_THAT(dialog.summary().toStdString(), ContainsSubstring("leave the grid, in 33 runs"));
}

TEST_CASE("the analysis opens from the menu, and changes nothing", "[gui][GUI-GRID-02]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    // Looked at **while it lives**: the dialog is a local of the slot, so
    // `lastDialog` dangles the moment the slot returns, and asking a freed
    // object what it is would be reading memory that is gone.
    bool wasTheAnalysis = false;
    prompts.fill = [&wasTheAnalysis](QDialog& dialog) {
        wasTheAnalysis = dynamic_cast<GridAnalysisDialog*>(&dialog) != nullptr;
    };

    REQUIRE(window.analyseGridAction()->isEnabled());
    window.analyseGridAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK(wasTheAnalysis);
    CHECK(window.gridStatus()->text().toStdString() == "Grid: 24 fps");
}

TEST_CASE("the analysis says when there was too little to look at", "[gui][GUI-GRID-02]") {
    // Two starts always look perfectly concentrated: the noise floor is one
    // over the square root of the count. The two causes of a silent verdict are
    // not the same answer, and the summary distinguishes them rather than
    // printing « none » beside « 100 % ».
    const std::vector<subedit::core::Timestamp> two = {
        subedit::core::Timestamp::fromMilliseconds(1'000),
        subedit::core::Timestamp::fromMilliseconds(2'000)};

    const GridAnalysisDialog dialog{deduceFrameRate(two)};

    CHECK_THAT(dialog.summary().toStdString(), ContainsSubstring("Too few subtitles to tell: 2"));
}

TEST_CASE("the analysis says when nothing explains the positions", "[gui][GUI-GRID-02]") {
    const GridAnalysisDialog dialog{deductionOf("grille-absurde.srt")};

    const std::string said = dialog.summary().toStdString();
    CHECK_THAT(said, ContainsSubstring("No frame rate grid explains these positions"));
    // The closest candidate is given so that a reader sees by how much it
    // fails, and no rate is offered as an answer.
    CHECK_THAT(said, ContainsSubstring("15.3%"));
}

TEST_CASE("the analysis gives the offset of a shifted grid", "[gui][GUI-GRID-02]") {
    const GridAnalysisDialog dialog{deductionOf("grille-24-decalee.srt")};

    CHECK_THAT(dialog.summary().toStdString(), ContainsSubstring("Shifted off the grid by"));
}

TEST_CASE("the conversion dialog opens on what the positions say", "[gui][GUI-FRAMERATE-03]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    std::optional<subedit::core::FrameRate> opened;
    prompts.fill = [&opened](QDialog& dialog) {
        if (auto* rates = dynamic_cast<subedit::gui::FrameRateDialog*>(&dialog))
            opened = rates->input();
    };

    window.frameRateAction()->trigger();

    REQUIRE(opened.has_value());
    CHECK(opened == subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps24});
}

TEST_CASE("a partial grid does not pre-fill the conversion", "[gui][GUI-FRAMERATE-03]") {
    // Evidence the deduction itself calls partial has no business deciding an
    // operation on the whole file. The status bar and the analysis carry that
    // case; this field does not.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("melange-groupe.srt", files, prompts);
    window.show();

    QString said;
    prompts.fill = [&said](QDialog& dialog) {
        if (auto* rates = dynamic_cast<subedit::gui::FrameRateDialog*>(&dialog))
            said = rates->deducedLabel();
    };

    window.frameRateAction()->trigger();

    CHECK(said.isEmpty());
}

TEST_CASE("the window aligns the document on a frame rate", "[gui][GUI-SNAP-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        if (auto* snap = dynamic_cast<subedit::gui::SnapDialog*>(&dialog))
            snap->setRate(subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});
    };

    REQUIRE(window.snapAction()->isEnabled());
    window.snapAction()->trigger();

    // The file was written on a grid at 24 and now sits on one at 25, which the
    // status bar says without being asked.
    CHECK(window.gridStatus()->text().toStdString() == "Grid: 25 fps");
}

TEST_CASE("an alignment is undone like any other operation", "[gui][GUI-SNAP-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        if (auto* snap = dynamic_cast<subedit::gui::SnapDialog*>(&dialog))
            snap->setRate(subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});
    };
    window.snapAction()->trigger();
    REQUIRE(window.gridStatus()->text().toStdString() == "Grid: 25 fps");

    window.undoAction()->trigger();

    CHECK(window.gridStatus()->text().toStdString() == "Grid: 24 fps");
}

TEST_CASE("bringing back onto the grid says its amount first", "[gui][GUI-GRID-03]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24-decalee.srt", files, prompts);
    window.show();

    // The measured amount is in the entry itself: the operation takes no
    // option, so there is no dialog to carry it, and a menu entry that will
    // move a whole file has to say by how much before it is chosen.
    REQUIRE(window.shiftOntoGridAction()->isEnabled());
    CHECK_THAT(window.shiftOntoGridAction()->text().toStdString(), ContainsSubstring("0.001 s"));
}

TEST_CASE("bringing back onto the grid puts the positions on frames", "[gui][GUI-GRID-03]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24-decalee.srt", files, prompts);
    window.show();

    window.shiftOntoGridAction()->trigger();

    // No dialog was needed, and none was opened.
    CHECK(prompts.runAsked == 0);
    CHECK(window.gridStatus()->text().toStdString() == "Grid: 24 fps");
    // The entry now offers nothing to do: the file is on its grid.
    CHECK_THAT(window.shiftOntoGridAction()->text().toStdString(), ContainsSubstring("0.000 s"));
}

TEST_CASE("without a grid there is nothing to be brought back onto", "[gui][GUI-GRID-03]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-absurde.srt", files, prompts);
    window.show();

    // Out, and not merely offering an amount of zero: « nothing to rejoin » and
    // « rejoin by nothing » are different things to tell a user.
    CHECK_FALSE(window.shiftOntoGridAction()->isEnabled());
    CHECK(window.shiftOntoGridAction()->text().toStdString() == "Shift onto Grid");
}

TEST_CASE("cancelling the alignment leaves the file alone", "[gui][GUI-SNAP-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-24.srt", files, prompts);
    window.show();

    prompts.nextRun = false;
    prompts.fill = [](QDialog& dialog) {
        if (auto* snap = dynamic_cast<subedit::gui::SnapDialog*>(&dialog))
            snap->setRate(subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});
    };

    window.snapAction()->trigger();

    // The rate was picked and the dialog dismissed: picking is not applying.
    CHECK(window.gridStatus()->text().toStdString() == "Grid: 24 fps");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("the entry that is out does nothing when triggered", "[gui][GUI-GRID-03]") {
    // Measured rather than reasoned about: the slot guards against an absent
    // amount, and whether that guard is reachable at all depends on what Qt
    // does with a disabled action. This case is what says so.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn("grille-absurde.srt", files, prompts);
    window.show();

    REQUIRE_FALSE(window.shiftOntoGridAction()->isEnabled());
    window.shiftOntoGridAction()->trigger();

    CHECK(window.gridStatus()->text().toStdString() == "No grid");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("a correction that would cross the origin is refused", "[gui][GUI-GRID-03]") {
    // The rule the core has held since #132, reached by the one route that can:
    // a partial file whose first start was moved by hand nearer the origin than
    // the phase measured around it. A file cannot hold a negative timestamp.
    InMemoryFileSystem files;
    FakePrompts prompts;
    files.addFile("a.srt", nearTheOrigin());
    MainWindow window{files, openProject(files, "a.srt").value(), prompts, {}, {}};
    window.show();

    REQUIRE(window.shiftOntoGridAction()->isEnabled());
    window.shiftOntoGridAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK_THAT(prompts.failures.front(), ContainsSubstring("before the origin"));
    CHECK_FALSE(window.undoAction()->isEnabled());
}
