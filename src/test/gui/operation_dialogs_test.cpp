// The operation dialogs — issue #132.
//
// **They are tested without ever entering `exec()`.** They are our own widgets:
// a test builds one, fills its fields and reads what it makes of them. Only the
// modal loop stays out of reach, and it is behind `Prompts::run`.

#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/command_preview.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::kStandardFrameRates;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;
using subedit::gui::anchorIn;
using subedit::gui::AnchorLookup;
using subedit::gui::AnchorView;
using subedit::gui::FrameRateDialog;
using subedit::gui::OperationPreview;
using subedit::gui::OperationScope;
using subedit::gui::PreviewRow;
using subedit::gui::ShiftDialog;
using subedit::gui::SnapDialog;
using subedit::gui::TransformDialog;
using subedit::gui::TypedReference;

} // namespace

TEST_CASE("the shift dialog reads a signed duration", "[gui][GUI-SHIFT-01]") {
    ShiftDialog dialog{4};

    dialog.setTyped(QStringLiteral("00:00:02,500"));
    CHECK(dialog.shift() == Duration::fromMilliseconds(2500));

    dialog.setTyped(QStringLiteral("-0:01,250"));
    CHECK(dialog.shift() == Duration::fromMilliseconds(-1250));
}

TEST_CASE("a shift that cannot be read is no shift at all", "[gui][GUI-SHIFT-01]") {
    // Rather than inventing a duration. The dialog then refuses to be
    // validated, which is the only way not to shift a file at random.
    ShiftDialog dialog{4};

    dialog.setTyped(QStringLiteral("bientôt"));

    CHECK_FALSE(dialog.shift().has_value());
    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("the shift dialog says what it is about to touch", "[gui][GUI-SHIFT-01]") {
    // The count, because « the selection, or the whole file » is not a rule
    // anyone guesses in front of a dialog box.
    const ShiftDialog whole{4};
    const ShiftDialog some{2};

    CHECK(whole.targetLabel().toStdString() == "4 subtitles");
    CHECK(some.targetLabel().toStdString() == "2 subtitles");
}

TEST_CASE("the transform dialog reads two references", "[gui][GUI-TRANSFORM-01]") {
    const TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:09,000"));

    // The whole reference rather than its fields: clang-tidy does not
    // recognise Catch2's REQUIRE as a check, and the project's convention is to
    // compare the optional itself.
    CHECK(dialog.first() ==
          TypedReference{.number = 1, .target = Timestamp::fromMilliseconds(1000)});
    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
    CHECK(dialog.isComplete());
}

TEST_CASE("two references on the same subtitle define no transform", "[gui][GUI-TRANSFORM-01]") {
    // The core already refuses it — `TransformCommand::create` returns
    // `nullopt` on a zero denominator. The dialog says so beforehand, rather
    // than letting the user validate for nothing.
    const TransformDialog dialog{4};

    dialog.setTyped(2, QStringLiteral("00:00:01,000"), 2, QStringLiteral("00:00:09,000"));

    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("a reference outside the file cannot be asked for", "[gui][GUI-TRANSFORM-01]") {
    // Not refused afterwards, but impossible to type: the field is bounded by
    // the number of subtitles. A ninth reference in a file of four falls back
    // to the fourth.
    const TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 9, QStringLiteral("00:00:09,000"));

    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("the transform dialog counts its target apart from its bounds",
          "[gui][GUI-TRANSFORM-01]") {
    // Two counts, and they do not say the same thing. The operation applies to
    // two subtitles; a reference is still a subtitle number, so it goes up to
    // the last of the file, selected or not.
    const TransformDialog dialog{OperationScope{4, 2}};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:09,000"));

    CHECK(dialog.targetLabel().toStdString() == "2 subtitles");
    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("an unreadable reference position is refused", "[gui][GUI-TRANSFORM-01]") {
    const TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("plus tard"));

    CHECK_FALSE(dialog.second().has_value());
    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("the frame rate dialog opens on the rate of the project", "[gui][GUI-FRAMERATE-01]") {
    // Pre-filled, and without a heuristic: the file does not carry its rate,
    // and getting it wrong shifts everything without a word.
    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
}

TEST_CASE("the frame rate dialog reads both rates", "[gui][GUI-FRAMERATE-01]") {
    FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    dialog.setRates(FrameRate{StandardFrameRate::Fps23976}, FrameRate{StandardFrameRate::Fps25});

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps23976});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps25});
    CHECK(dialog.isComplete());
}

TEST_CASE("converting a rate into itself changes nothing, and the dialog says so",
          "[gui][GUI-FRAMERATE-01]") {
    FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    dialog.setRates(FrameRate{StandardFrameRate::Fps25}, FrameRate{StandardFrameRate::Fps25});

    CHECK_FALSE(dialog.isComplete());
}

// Decision D6, in the dialog: what the container names is proposed on « should
// play at », because that is the rate the film actually runs at — what the
// document was timed against is the field above, and only the user knows it.
TEST_CASE("the frame rate the film declares is proposed", "[gui][GUI-FRAMERATE-02]") {
    const FrameRateDialog dialog{
        4, FrameRate{StandardFrameRate::Fps25}, FrameRate{StandardFrameRate::Fps23976}};

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps23976});
    // And the dialog is ready: the two rates differ, which is the case this is
    // meant to make easy.
    CHECK(dialog.isComplete());
}

// « Sa provenance est dite » : the number alone would be a value out of
// nowhere, and D6 rests on the user knowing it is a proposal they may refuse.
TEST_CASE("the dialog says where the proposed rate comes from", "[gui][GUI-FRAMERATE-02]") {
    const FrameRateDialog dialog{
        4, FrameRate{StandardFrameRate::Fps25}, FrameRate{StandardFrameRate::Fps23976}};

    CHECK(dialog.declaredLabel().toStdString() == "24000/1001");
}

// Without a film, or without `ffprobe`: the row is not there, and the dialog is
// exactly the one that came before.
TEST_CASE("with nothing declared the dialog is the one from before", "[gui][GUI-FRAMERATE-02]") {
    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.declaredLabel().isEmpty());
    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps25});
    CHECK_FALSE(dialog.isComplete());
}

// A film may declare a rate this dialog cannot offer — the list is the eight
// standards, closed on purpose. It is still said, because knowing that the film
// runs at something unusual is the information; there is simply nothing to
// convert to.
TEST_CASE("a rate outside the eight standards is said and not picked", "[gui][GUI-FRAMERATE-02]") {
    const std::optional<FrameRate> unusual = FrameRate::create(15, 1);
    REQUIRE(unusual.has_value());

    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}, unusual};

    CHECK(dialog.declaredLabel().toStdString() == "15");
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps25});
}

// Phase 16, and the sentence the manual had to unlearn: that the file had been
// timed against a rate it alone knew, and that nobody could guess it in its
// stead. Something can now — not a guess but a measurement of the positions
// themselves.
TEST_CASE("the measured grid pre-fills what the file was timed against",
          "[gui][GUI-FRAMERATE-03]") {
    const FrameRateDialog dialog{
        4, FrameRate{StandardFrameRate::Fps25}, std::nullopt, FrameRate{StandardFrameRate::Fps24}};

    // The project said 25; the positions say 24, and the positions win the
    // pre-fill. The box stays as free as it ever was.
    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps24});
    CHECK(dialog.deducedLabel().toStdString() == "24");
}

TEST_CASE("without a clean grid the field opens where it used to", "[gui][GUI-FRAMERATE-03]") {
    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.deducedLabel().isEmpty());
    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
}

// Decision D13. The film runs at one rate and the file was written on another
// grid: that is not a contradiction to resolve, it is the very case the
// alignment exists for. Both are shown, neither is imposed.
TEST_CASE("a disagreement between the two sources is shown, not arbitrated",
          "[gui][GUI-FRAMERATE-04]") {
    const FrameRateDialog dialog{4,
                                 FrameRate{StandardFrameRate::Fps30},
                                 FrameRate{StandardFrameRate::Fps25},
                                 FrameRate{StandardFrameRate::Fps24}};

    CHECK(dialog.deducedLabel().toStdString() == "24");
    CHECK(dialog.declaredLabel().toStdString() == "25");
    // Each proposal lands on the field it answers: the grid the file was
    // written on above, the rate the film runs at below.
    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps24});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps25});
}

TEST_CASE("the user may still overrule both", "[gui][GUI-FRAMERATE-04]") {
    FrameRateDialog dialog{4,
                           FrameRate{StandardFrameRate::Fps30},
                           FrameRate{StandardFrameRate::Fps25},
                           FrameRate{StandardFrameRate::Fps24}};

    dialog.setRates(FrameRate{StandardFrameRate::Fps50}, FrameRate{StandardFrameRate::Fps60});

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps50});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps60});
}

// The alignment opens on the grid of the **film**, not on the one the positions
// are leaving: the intention is to join the first, and the deduction names the
// second.
TEST_CASE("the alignment opens on what the film declares", "[gui][GUI-SNAP-01]") {
    const SnapDialog dialog{
        4, FrameRate{StandardFrameRate::Fps24}, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.rate() == FrameRate{StandardFrameRate::Fps25});
    // Always ready: aligning a file on the grid it already sits on moves
    // nothing, which is a no-op and not an error.
    CHECK(dialog.isComplete());
}

TEST_CASE("without a film the alignment opens on the project rate", "[gui][GUI-SNAP-01]") {
    const SnapDialog dialog{4, FrameRate{StandardFrameRate::Fps24}};

    CHECK(dialog.rate() == FrameRate{StandardFrameRate::Fps24});
}

// One list, and the proof that it is one — issue #224.
//
// The two dialogues carried their own copy of the eight standards until
// `FrameRateBox` took them over. What a test can see of that is not the class
// but its consequence: every rate the list is supposed to hold can be chosen in
// both dialogues, and comes back as itself.
TEST_CASE("both dialogues offer the same eight rates", "[gui][GUI-FRAMERATE-01][GUI-SNAP-01]") {
    SnapDialog snap{4, FrameRate{StandardFrameRate::Fps24}};
    FrameRateDialog conversion{4, FrameRate{StandardFrameRate::Fps24}};

    for (const StandardFrameRate standard : kStandardFrameRates) {
        const FrameRate rate{standard};

        snap.setRate(rate);
        CHECK(snap.rate() == rate);

        // The conversion refuses a rate converted into itself, so the two
        // fields are set apart and read back one at a time.
        conversion.setRates(rate, rate);
        CHECK(conversion.input() == rate);
        CHECK(conversion.output() == rate);
    }
}

// Phase 9, and the third provenance. A file counted in frames states no rate;
// what its positions were computed with is the one number that decides them,
// and the dialog is where a user says it was the wrong one.
TEST_CASE("the rate a file was read at pre-fills what it was timed against",
          "[gui][GUI-FRAMES-01]") {
    const FrameRateDialog dialog{4,
                                 FrameRate{StandardFrameRate::Fps23976},
                                 std::nullopt,
                                 std::nullopt,
                                 FrameRate{StandardFrameRate::Fps23976}};

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps23976});
    CHECK(dialog.readLabel().toStdString() == "24000/1001");
}

TEST_CASE("the field opens on what the document counts at, not on what it was read at",
          "[gui][GUI-FRAMES-01]") {
    // **The two differ exactly once a conversion has happened**, and that is
    // the case this pins: the file was read at 23.976, the user corrected the
    // document to 25, and reopening the dialog must not propose the stale
    // number back. The row still says where the file's own came from.
    const FrameRateDialog dialog{4,
                                 FrameRate{StandardFrameRate::Fps25},
                                 std::nullopt,
                                 std::nullopt,
                                 FrameRate{StandardFrameRate::Fps23976}};

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
    CHECK(dialog.readLabel().toStdString() == "24000/1001");
}

TEST_CASE("a document counted in time has no such row", "[gui][GUI-FRAMES-01]") {
    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.readLabel().isEmpty());
}

TEST_CASE("the transform dialog shows what each number stands for", "[gui][GUI-TRANSFORM-01]") {
    // The current start and the text of the subtitle a number names, so the
    // user need not look the line up in the table before typing a time.
    const AnchorLookup lookup = [](int number) -> std::optional<AnchorView> {
        return AnchorView{.start = QStringLiteral("00:00:0%1,000").arg(number),
                          .text = QStringLiteral("line %1").arg(number)};
    };
    const TransformDialog dialog{4, lookup};

    // Both rows are filled on opening: the first on subtitle 1, the second on
    // the last.
    CHECK(dialog.firstCurrent().toStdString() == "00:00:01,000");
    CHECK(dialog.firstText().toStdString() == "line 1");
    CHECK(dialog.secondCurrent().toStdString() == "00:00:04,000");
    CHECK(dialog.secondText().toStdString() == "line 4");

    // And they follow the number, the proposed new start with them.
    dialog.setTyped(3, QStringLiteral("00:00:09,000"), 2, QStringLiteral("00:00:08,000"));
    CHECK(dialog.firstCurrent().toStdString() == "00:00:03,000");
    CHECK(dialog.firstText().toStdString() == "line 3");
    CHECK(dialog.secondCurrent().toStdString() == "00:00:02,000");
    CHECK(dialog.secondText().toStdString() == "line 2");
    CHECK(dialog.first() ==
          TypedReference{.number = 3, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("the transform dialog proposes the current start as the new one",
          "[gui][GUI-TRANSFORM-01]") {
    const AnchorLookup lookup = [](int number) -> std::optional<AnchorView> {
        return AnchorView{.start = QStringLiteral("00:00:0%1,500").arg(number), .text = {}};
    };
    const TransformDialog dialog{4, lookup};

    // Untouched, the dialog already describes the identity: nothing moves
    // until the user changes a time.
    CHECK(dialog.first() ==
          TypedReference{.number = 1, .target = Timestamp::fromMilliseconds(1500)});
    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(4500)});
    CHECK(dialog.isComplete());
}

TEST_CASE("a number with nothing behind it leaves the typed time alone",
          "[gui][GUI-TRANSFORM-01]") {
    const AnchorLookup lookup = [](int) { return std::optional<AnchorView>{}; };
    const TransformDialog dialog{4, lookup};

    dialog.setTyped(1, QStringLiteral("00:00:02,000"), 4, QStringLiteral("00:00:09,000"));

    CHECK(dialog.firstCurrent().isEmpty());
    CHECK(dialog.firstText().isEmpty());
    CHECK(dialog.first() ==
          TypedReference{.number = 1, .target = Timestamp::fromMilliseconds(2000)});
}

TEST_CASE("a number outside the file names no subtitle", "[gui][GUI-TRANSFORM-01]") {
    subedit::core::Project project;
    project.setSubtitles({subedit::core::Subtitle{.start = Timestamp::fromMilliseconds(1500),
                                                  .end = Timestamp::fromMilliseconds(2500),
                                                  .mainText = "Only."}});

    const std::optional<AnchorView> inside = anchorIn(project, 1);
    const AnchorView shown = inside.value_or(AnchorView{});
    CHECK(shown.start.toStdString() == "00:00:01,500");
    CHECK(shown.text.toStdString() == "Only.");

    CHECK_FALSE(anchorIn(project, 0).has_value());
    CHECK_FALSE(anchorIn(project, 2).has_value());
    CHECK_FALSE(anchorIn(project, -3).has_value());
}

TEST_CASE("a partial selection leaves the choice of the target to the user",
          "[gui][GUI-SCOPE-01]") {
    ShiftDialog dialog{OperationScope{4, 2}};

    // The selection is the default, as it was before the choice existed.
    CHECK_FALSE(dialog.wholeProject());
    CHECK(dialog.targetLabel().toStdString() == "2 subtitles");

    dialog.chooseWholeProject(true);
    CHECK(dialog.wholeProject());
    CHECK(dialog.targetLabel().toStdString() == "4 subtitles");

    dialog.chooseWholeProject(false);
    CHECK_FALSE(dialog.wholeProject());
}

TEST_CASE("with nothing to choose between, the target is the whole project",
          "[gui][GUI-SCOPE-01]") {
    // Nothing selected, or everything selected: one target only, and no
    // question asked.
    for (const OperationScope scope : {OperationScope{4, 0}, OperationScope{4, 4}}) {
        ShiftDialog dialog{scope};

        CHECK(dialog.wholeProject());
        CHECK(dialog.targetLabel().toStdString() == "4 subtitles");

        // Choosing is a no-op where there is nothing to choose.
        dialog.chooseWholeProject(false);
        CHECK(dialog.wholeProject());
    }
}

TEST_CASE("a preview shows the changes the owner computes", "[gui][GUI-PREVIEW-01]") {
    ShiftDialog dialog{4};
    dialog.offerPreview([] {
        return OperationPreview{
            .rows = {PreviewRow{.number = QStringLiteral("2"),
                                .before = QStringLiteral("00:00:01,000 → 00:00:02,000"),
                                .after = QStringLiteral("00:00:02,000 → 00:00:03,000"),
                                .text = QStringLiteral("Hello")}},
            .changed = 5};
    });

    auto* button = dialog.findChild<QPushButton*>(QStringLiteral("preview-button"));
    REQUIRE(button != nullptr);
    button->click();

    auto* box = dialog.findChild<QDialog*>(QStringLiteral("preview"));
    REQUIRE(box != nullptr);
    const auto* table = box->findChild<QTableWidget*>();
    REQUIRE(table != nullptr);
    CHECK(table->rowCount() == 1);
    CHECK(table->item(0, 0)->text().toStdString() == "2");
    CHECK(table->item(0, 2)->text().toStdString() == "00:00:02,000 → 00:00:03,000");
    CHECK(table->item(0, 3)->text().toStdString() == "Hello");
    CHECK(box->findChild<QLabel*>()->text().toStdString() == "5 subtitles would change.");

    // Opens wide enough for every column, and stays resizable: the first
    // version showed the start of a column and made the user scroll.
    CHECK(box->sizeHint().width() >= table->horizontalHeader()->length());
    CHECK(box->isSizeGripEnabled());
    box->close();
}

TEST_CASE("a preview of nothing says so", "[gui][GUI-PREVIEW-01]") {
    ShiftDialog dialog{4};
    dialog.offerPreview([] { return OperationPreview{}; });

    dialog.findChild<QPushButton*>(QStringLiteral("preview-button"))->click();

    const auto* box = dialog.findChild<QDialog*>(QStringLiteral("preview"));
    REQUIRE(box != nullptr);
    CHECK(box->findChild<QLabel*>()->text().toStdString() == "Nothing would change.");
    CHECK(box->findChild<QTableWidget*>()->rowCount() == 0);
}

TEST_CASE("the preview lists what the core says would change", "[gui][GUI-PREVIEW-01]") {
    subedit::core::CommandPreview computed;
    computed.changed = 1;
    computed.shown.push_back(subedit::core::PreviewedChange{
        .index = subedit::core::SubtitleIndex::fromValue(2),
        .before = subedit::core::Subtitle{.start = Timestamp::fromMilliseconds(1000),
                                          .end = Timestamp::fromMilliseconds(2000),
                                          .mainText = "First line\nSecond line"},
        .after = subedit::core::Subtitle{.start = Timestamp::fromMilliseconds(1500),
                                         .end = Timestamp::fromMilliseconds(2500)}});

    const OperationPreview described = subedit::gui::describedPreview(computed);

    CHECK(described.changed == 1);
    REQUIRE(described.rows.size() == 1);
    CHECK(described.rows.front().number.toStdString() == "3");
    CHECK(described.rows.front().before.toStdString() == "00:00:01,000 → 00:00:02,000");
    CHECK(described.rows.front().after.toStdString() == "00:00:01,500 → 00:00:02,500");
    CHECK(described.rows.front().text.toStdString() == "First line");
}
