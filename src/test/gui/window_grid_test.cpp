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
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>

#include <QAction>
#include <QDialog>
#include <QLabel>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <grid_fixtures.hpp>
#include <string>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::deduceFrameRate;
using subedit::core::FrameRateDeduction;
using subedit::core::InMemoryFileSystem;
using subedit::gui::GridAnalysisDialog;
using subedit::gui::MainWindow;
using subedit::gui::openProject;
using subedit::test::FakePrompts;

/// A window showing the grid fixture named, opened as a user would open it.
[[nodiscard]] MainWindow
windowOn(std::string_view fixture, InMemoryFileSystem& files, FakePrompts& prompts) {
    files.addFile("a.srt", subedit::test::gridBytes(fixture));
    return MainWindow{files, openProject(files, "a.srt").value(), prompts, {}, {}};
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
