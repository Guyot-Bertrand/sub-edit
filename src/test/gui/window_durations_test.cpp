// Adjusting durations from the window — issue #383.
//
// The order and the arithmetic live in the core and are tested there. What is
// under test here is the dialog — each constraint switched off is absent, and
// nothing to do is not a request — and what the window adds: the target, the
// single entry in the history, and the account of what was given up.

#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QItemSelectionModel>
#include <QLocale>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::constraintsOf;
using subedit::core::Duration;
using subedit::core::DurationAdjustmentSettings;
using subedit::core::DurationConstraints;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::ReadingSpeed;
using subedit::gui::DurationAdjustDialog;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// Three short subtitles: half a second each, the second one second after the
/// first, the third far away.
constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:01,500\nUn.\n\n"
                               "2\n00:00:02,000 --> 00:00:02,500\nDeux.\n\n"
                               "3\n00:00:10,000 --> 00:00:10,500\nTrois.\n\n";

[[nodiscard]] InMemoryFileSystem withThree() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    return files;
}

[[nodiscard]] OpenedFile threeIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

constexpr int kEndColumn = 2;

[[nodiscard]] std::string endAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, kEndColumn), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// Only a minimum of `seconds`, every other constraint off.
[[nodiscard]] auto onlyMinimum(double seconds) {
    return [seconds](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        adjust.lengthenCheck()->setChecked(false);
        adjust.shortenCheck()->setChecked(false);
        adjust.maximumCheck()->setChecked(false);
        adjust.gapCheck()->setChecked(false);
        adjust.minimumCheck()->setChecked(true);
        adjust.minimumBox()->setValue(seconds);
    };
}

} // namespace

TEST_CASE("the dialog opens on Gaupol's defaults", "[gui][GUI-ADJUST-01]") {
    const DurationAdjustDialog dialog{3, DurationAdjustmentSettings{}};

    CHECK(dialog.speedBox()->value() == 15.0);
    CHECK(dialog.lengthenCheck()->isChecked());
    CHECK_FALSE(dialog.shortenCheck()->isChecked());
    CHECK(dialog.minimumCheck()->isChecked());
    CHECK(dialog.minimumBox()->value() == 1.5);
    CHECK_FALSE(dialog.maximumCheck()->isChecked());
    CHECK(dialog.maximumBox()->value() == 6.0);
    CHECK_FALSE(dialog.maximumBox()->isEnabled());
    CHECK(dialog.gapCheck()->isChecked());
    CHECK(dialog.gapBox()->value() == 0.0);

    CHECK(dialog.settings() == DurationAdjustmentSettings{});
    CHECK(constraintsOf(dialog.settings()) == DurationConstraints{});
    CHECK(dialog.isComplete());
}

TEST_CASE("the dialog writes a decimal point whatever the machine's locale",
          "[gui][GUI-ADJUST-01]") {
    // Found on the capture of the manual: under a French locale, the fields
    // read `15,0 char/s` beneath English labels.
    const QLocale before;
    QLocale::setDefault(QLocale{QLocale::French, QLocale::France});
    const DurationAdjustDialog dialog{3, DurationAdjustmentSettings{}};
    QLocale::setDefault(before);

    CHECK(dialog.speedBox()->text().toStdString() == "15.0 char/s");
    CHECK(dialog.minimumBox()->text().toStdString() == "1.500 s");
}

TEST_CASE("a constraint switched off is absent, and its field goes grey", "[gui][GUI-ADJUST-01]") {
    const DurationAdjustDialog dialog{3, DurationAdjustmentSettings{}};

    dialog.minimumCheck()->setChecked(false);
    dialog.lengthenCheck()->setChecked(false);

    CHECK_FALSE(dialog.minimumBox()->isEnabled());
    CHECK_FALSE(dialog.speedBox()->isEnabled());
    CHECK_FALSE(constraintsOf(dialog.settings()).minimum.has_value());
    CHECK_FALSE(constraintsOf(dialog.settings()).speed.has_value());
    // The gap is still on, at zero — and zero means zero.
    CHECK(constraintsOf(dialog.settings()).gap == Duration::zero());
}

TEST_CASE("with every constraint off, there is nothing to ask for", "[gui][GUI-ADJUST-01]") {
    const DurationAdjustDialog dialog{3, DurationAdjustmentSettings{}};

    for (QCheckBox* check : {dialog.lengthenCheck(),
                             dialog.shortenCheck(),
                             dialog.minimumCheck(),
                             dialog.maximumCheck(),
                             dialog.gapCheck()})
        check->setChecked(false);

    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("the dialog reads what it was given back", "[gui][GUI-ADJUST-01]") {
    const DurationAdjustmentSettings asked{.charactersPerSecond = 12.5,
                                           .lengthen = false,
                                           .shorten = true,
                                           .minimumEnabled = false,
                                           .minimumMilliseconds = 2000,
                                           .maximumEnabled = true,
                                           .maximumMilliseconds = 4250,
                                           .gapEnabled = true,
                                           .gapMilliseconds = 80};

    const DurationAdjustDialog dialog{3, asked};

    // The form comes back whole, the value of the case that is off included...
    CHECK(dialog.settings() == asked);

    // ...and the request leaves that case out.
    const DurationConstraints request = constraintsOf(dialog.settings());
    CHECK(request.speed == ReadingSpeed::create(12.5, false, true));
    CHECK_FALSE(request.minimum.has_value());
    CHECK(request.maximum == Duration::fromMilliseconds(4250));
    CHECK(request.gap == Duration::fromMilliseconds(80));
}

TEST_CASE("adjusting moves the ends of the target, and undoes in one step",
          "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = onlyMinimum(2.0);
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    REQUIRE(window.adjustDurationsAction()->isEnabled());
    window.adjustDurationsAction()->trigger();

    CHECK(endAt(window, 0) == "00:00:03,000");
    CHECK(endAt(window, 2) == "00:00:12,000");
    CHECK(window.undoAction()->text().toStdString() == "Undo: adjusting durations");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == "adjusted the durations of 3 subtitles");

    window.undoAction()->trigger();

    CHECK(endAt(window, 0) == "00:00:01,500");
    CHECK(endAt(window, 2) == "00:00:10,500");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("adjusting says what no end could satisfy", "[gui][GUI-ADJUST-02]") {
    // The second subtitle starts one second after the first: a minimum of two
    // seconds and a gap of zero cannot both hold, and the gap wins.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        onlyMinimum(2.0)(dialog);
        dynamic_cast<DurationAdjustDialog&>(dialog).gapCheck()->setChecked(true);
    };
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    window.adjustDurationsAction()->trigger();

    CHECK(endAt(window, 0) == "00:00:02,000");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() ==
          "adjusted the durations of 3 subtitles; could not satisfy the minimum duration in "
          "1 subtitle");
}

TEST_CASE("an adjustment that moves nothing says so, and enters no history",
          "[gui][GUI-ADJUST-02]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = onlyMinimum(0.1);
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    window.adjustDurationsAction()->trigger();

    CHECK_FALSE(window.undoAction()->isEnabled());
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == "no duration to adjust");
}

TEST_CASE("the next dialog offers what the last one asked", "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = onlyMinimum(2.0);
    MainWindow window{files, threeIn(files), prompts};
    window.show();
    window.adjustDurationsAction()->trigger();

    DurationConstraints offered;
    prompts.nextRun = false;
    prompts.fill = [&offered](QDialog& dialog) {
        offered = constraintsOf(dynamic_cast<DurationAdjustDialog&>(dialog).settings());
    };
    window.adjustDurationsAction()->trigger();

    CHECK(offered.minimum == Duration::fromMilliseconds(2000));
    CHECK_FALSE(offered.gap.has_value());
    CHECK_FALSE(offered.speed.has_value());
}

TEST_CASE("a cancelled adjustment moves nothing and says nothing", "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = false;
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    window.adjustDurationsAction()->trigger();

    CHECK(endAt(window, 0) == "00:00:01,500");
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("an empty document has nothing to adjust", "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window{files, OpenedFile{}, prompts};

    CHECK_FALSE(window.adjustDurationsAction()->isEnabled());
}

TEST_CASE("an unchecked minimum keeps its value across two openings of the dialog",
          "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        adjust.minimumBox()->setValue(2.0);
        adjust.minimumCheck()->setChecked(false);
    };
    window.adjustDurationsAction()->trigger();

    double offered = 0.0;
    bool checked = true;
    prompts.nextRun = false;
    prompts.fill = [&offered, &checked](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        offered = adjust.minimumBox()->value();
        checked = adjust.minimumCheck()->isChecked();
    };
    window.adjustDurationsAction()->trigger();

    CHECK(offered == 2.0);
    CHECK_FALSE(checked);
}

TEST_CASE("a reading speed with both cases unchecked keeps its value across two openings",
          "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        adjust.speedBox()->setValue(22.5);
        adjust.lengthenCheck()->setChecked(false);
        adjust.shortenCheck()->setChecked(false);
    };
    window.adjustDurationsAction()->trigger();

    double offered = 0.0;
    prompts.nextRun = false;
    prompts.fill = [&offered](QDialog& dialog) {
        offered = dynamic_cast<DurationAdjustDialog&>(dialog).speedBox()->value();
    };
    window.adjustDurationsAction()->trigger();

    CHECK(offered == 22.5);
}
