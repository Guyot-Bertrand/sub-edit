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

/// A subtitle of `text` from `start` to `end`, written as a SubRip block.
[[nodiscard]] std::string
blockOf(int number, const char* start, const char* end, const std::string& text) {
    return std::to_string(number) + "\n" + start + " --> " + end + "\n" + text + "\n\n";
}

/// All four constraints on together, the reading speed both ways, with the
/// values given: `speed` in characters a second, the others in seconds.
[[nodiscard]] auto allFour(double speed, double minimum, double maximum, double gap) {
    return [speed, minimum, maximum, gap](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        for (QCheckBox* check : {adjust.lengthenCheck(),
                                 adjust.shortenCheck(),
                                 adjust.minimumCheck(),
                                 adjust.maximumCheck(),
                                 adjust.gapCheck()})
            check->setChecked(true);
        adjust.speedBox()->setValue(speed);
        adjust.minimumBox()->setValue(minimum);
        adjust.maximumBox()->setValue(maximum);
        adjust.gapBox()->setValue(gap);
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

TEST_CASE("the four constraints act in their order, and the order shows in the result",
          "[gui][GUI-ADJUST-01]") {
    // Reading speed 10 characters a second, both ways; minimum 2 s; maximum 4 s;
    // gap 0.5 s. Worked by hand from the manual's order — speed, minimum,
    // maximum, gap, the last one applied winning. Each of the first three
    // subtitles catches a different permutation: another end comes out if the
    // speed and the minimum (1), the speed and the maximum (2), or the minimum
    // and the gap (3) change places. The fourth is the last subtitle, which has
    // no next one to keep a gap from.
    InMemoryFileSystem files;
    files.addFile("film.srt",
                  blockOf(1, "00:00:00,000", "00:00:01,000", "Hi") +
                      blockOf(2, "00:00:10,000", "00:00:11,000", std::string(60, 'x')) +
                      blockOf(3, "00:00:20,000", "00:00:20,500", "Hi") +
                      blockOf(4, "00:00:22,000", "00:00:22,500", "Hi"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = allFour(10.0, 2.0, 4.0, 0.5);
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    window.adjustDurationsAction()->trigger();

    // 1 — "Hi" is 0.2 s of reading, so the speed shortens the second it lasts
    // to 0.2 s, and the minimum then raises it to 2 s. The other way round, the
    // speed would have the last word and leave 0.2 s.
    CHECK(endAt(window, 0) == "00:00:02,000");
    // 2 — 60 characters are 6 s of reading: the speed lengthens it to 16 s, and
    // the maximum then brings it back to 14 s. The other way round, the speed
    // would leave 16 s.
    CHECK(endAt(window, 1) == "00:00:14,000");
    // 3 — the speed gives 20.2 s and the minimum 22 s, which is where the next
    // subtitle starts: the gap, applied last, takes it to 21.5 s. Applied
    // before the minimum, it would leave 22 s.
    CHECK(endAt(window, 2) == "00:00:21,500");
    // 4 — the last subtitle has no next one to keep a gap from: the speed
    // gives 22.2 s and the minimum 24 s, and nothing follows.
    CHECK(endAt(window, 3) == "00:00:24,000");
}

TEST_CASE("a minimum above the maximum gives way to it, and both come after the speed",
          "[gui][GUI-ADJUST-01]") {
    // The one case where the order of the minimum and the maximum shows: with
    // the minimum below the maximum the two commute, and no result tells them
    // apart. Here, a minimum of 5 s and a maximum of 3 s, gap 0.5 s and speed
    // 10 characters a second.
    InMemoryFileSystem files;
    files.addFile("film.srt",
                  blockOf(1, "00:00:00,000", "00:00:01,000", "Hi") +
                      blockOf(2, "00:00:10,000", "00:00:11,000", "Hi"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = allFour(10.0, 5.0, 3.0, 0.5);
    MainWindow window{files, threeIn(files), prompts};
    window.show();

    window.adjustDurationsAction()->trigger();

    // The speed gives 0.2 s, the minimum 5 s, the maximum 3 s. The maximum
    // applied first would leave 5 s; the speed applied last, 0.2 s.
    CHECK(endAt(window, 0) == "00:00:03,000");
    CHECK(endAt(window, 1) == "00:00:13,000");
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

TEST_CASE("the form of the adjustment is kept by the preferences", "[gui][GUI-ADJUST-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, threeIn(files), prompts};
    window.applySettings(
        subedit::core::Settings{.durationAdjustment = {.charactersPerSecond = 12.5,
                                                       .lengthen = false,
                                                       .shorten = true,
                                                       .minimumEnabled = false,
                                                       .minimumMilliseconds = 2000}});
    window.show();

    // What the preferences carry is what the dialog opens on...
    double speed = 0.0;
    double minimum = 0.0;
    bool minimumChecked = true;
    prompts.nextRun = true;
    prompts.fill = [&](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        speed = adjust.speedBox()->value();
        minimum = adjust.minimumBox()->value();
        minimumChecked = adjust.minimumCheck()->isChecked();
        adjust.gapCheck()->setChecked(false);
        adjust.maximumBox()->setValue(4.25);
        adjust.maximumCheck()->setChecked(true);
    };
    window.adjustDurationsAction()->trigger();

    CHECK(speed == 12.5);
    CHECK(minimum == 2.0);
    CHECK_FALSE(minimumChecked);

    // ...and what the dialog last asked is what the preferences hand back.
    const DurationAdjustmentSettings kept = window.settings().durationAdjustment;
    CHECK(kept.charactersPerSecond == 12.5);
    CHECK_FALSE(kept.minimumEnabled);
    CHECK(kept.minimumMilliseconds == 2000);
    CHECK(kept.maximumEnabled);
    CHECK(kept.maximumMilliseconds == 4250);
    CHECK_FALSE(kept.gapEnabled);
}
