// What the window makes of the three dialogs — issue #132.
//
// The fake `Prompts` plays the user: it receives the dialog, writes into it
// what the scenario wants, and says whether it is validated. The modal loop is
// never reached.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/operation_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QDialog>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::StandardFrameRate;
using subedit::gui::DurationAdjustDialog;
using subedit::gui::FrameRateDialog;
using subedit::gui::MainWindow;
using subedit::gui::OperationDialog;
using subedit::gui::ShiftDialog;
using subedit::gui::SnapDialog;
using subedit::gui::TransformDialog;
using subedit::test::FakePrompts;

/// Four subtitles, at one, three, five and seven seconds.
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

/// The start of a row, as the table shows it.
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
    MainWindow window{files, fourIn(files), prompts};
    window.show();

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
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.shiftAction()->trigger();

    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK(startAt(window, 1) == "00:00:04,000");
    CHECK(startAt(window, 2) == "00:00:05,000");
}

TEST_CASE("a partial selection can be widened to the whole project from the dialog",
          "[gui][GUI-SCOPE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& shift = dynamic_cast<ShiftDialog&>(dialog);
        shift.chooseWholeProject(true);
        shift.setTyped(QStringLiteral("00:00:01,000"));
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.shiftAction()->trigger();

    // The selection was one row; the dialog widened it to the four.
    CHECK(startAt(window, 0) == "00:00:02,000");
    CHECK(startAt(window, 3) == "00:00:08,000");
}

TEST_CASE("the shift dialog previews the change of the target it is on", "[gui][GUI-PREVIEW-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string seen;
    prompts.fill = [&seen](QDialog& dialog) {
        auto& shift = dynamic_cast<ShiftDialog&>(dialog);
        shift.setTyped(QStringLiteral("00:00:01,000"));
        shift.findChild<QPushButton*>(QStringLiteral("preview-button"))->click();

        const auto* table =
            shift.findChild<QDialog*>(QStringLiteral("preview"))->findChild<QTableWidget*>();
        seen = std::to_string(table->rowCount()) + "|" + table->item(0, 1)->text().toStdString() +
               "|" + table->item(0, 2)->text().toStdString();
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.shiftAction()->trigger();

    // One row selected: one row previewed, from where it is to where it goes.
    CHECK(seen == "1|00:00:03,000 → 00:00:04,000|00:00:04,000 → 00:00:05,000");
    // And nothing moved: a preview applies nothing.
    CHECK(startAt(window, 1) == "00:00:03,000");
}

namespace {

/// What the preview of `dialog` says on its first line, after the button was
/// pressed — the dialog being the one the window opened.
[[nodiscard]] std::string previewSaid(QDialog& dialog) {
    auto* button = dialog.findChild<QPushButton*>(QStringLiteral("preview-button"));
    REQUIRE(button != nullptr);
    button->click();
    return dialog.findChild<QDialog*>(QStringLiteral("preview"))
        ->findChild<QLabel*>()
        ->text()
        .toStdString();
}

} // namespace

TEST_CASE("every dialog that offers a preview says what it would do", "[gui][GUI-PREVIEW-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string said;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    SECTION("shift, once a duration is typed") {
        prompts.fill = [&said](QDialog& dialog) {
            dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("00:00:01,000"));
            said = previewSaid(dialog);
        };
        window.shiftAction()->trigger();
        CHECK(said == "4 subtitles would change.");
    }

    SECTION("shift, while the duration cannot be read") {
        prompts.fill = [&said](QDialog& dialog) { said = previewSaid(dialog); };
        window.shiftAction()->trigger();
        CHECK(said == "Nothing would change.");
    }

    SECTION("transform, from two references that move something") {
        prompts.fill = [&said](QDialog& dialog) {
            dynamic_cast<TransformDialog&>(dialog).setTyped(
                1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:13,000"));
            said = previewSaid(dialog);
        };
        window.transformAction()->trigger();
        CHECK(said == "4 subtitles would change.");
    }

    SECTION("transform, from two references that define nothing") {
        prompts.fill = [&said](QDialog& dialog) {
            dynamic_cast<TransformDialog&>(dialog).setTyped(
                2, QStringLiteral("00:00:03,000"), 2, QStringLiteral("00:00:09,000"));
            said = previewSaid(dialog);
        };
        window.transformAction()->trigger();
        CHECK(said == "Nothing would change.");
    }

    SECTION("transform, while a reference cannot be read") {
        prompts.fill = [&said](QDialog& dialog) {
            dynamic_cast<TransformDialog&>(dialog).setTyped(
                1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("plus tard"));
            said = previewSaid(dialog);
        };
        window.transformAction()->trigger();
        CHECK(said == "Nothing would change.");
    }

    SECTION("frame rate conversion") {
        prompts.fill = [&said](QDialog& dialog) {
            auto& rates = dynamic_cast<FrameRateDialog&>(dialog);
            rates.setRates(subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps23976},
                           subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});
            said = previewSaid(dialog);
        };
        window.frameRateAction()->trigger();
        CHECK(said.find("would change") != std::string::npos);
    }

    SECTION("duration adjustment") {
        prompts.fill = [&said](QDialog& dialog) {
            said = previewSaid(dynamic_cast<DurationAdjustDialog&>(dialog));
        };
        window.adjustDurationsAction()->trigger();
        CHECK_FALSE(said.empty());
    }

    SECTION("snap to a frame rate") {
        prompts.fill = [&said](QDialog& dialog) {
            said = previewSaid(dynamic_cast<SnapDialog&>(dialog));
        };
        window.snapAction()->trigger();
        CHECK_FALSE(said.empty());
    }
}

TEST_CASE("a dialog names the selection, and not the file", "[gui][GUI-SHIFT-01]") {
    // The defect the end-of-phase review found: the window handed the count of
    // the file to the four dialogs, which announced « 4 subtitles » while the
    // operation changed two. The dialog tests built them by hand, so none of
    // them could see it.
    //
    // Read from `fill`, while the dialog lives: it sits on the window's stack
    // and does not survive the return of the action.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string said;
    prompts.fill = [&said](QDialog& dialog) {
        said = dynamic_cast<OperationDialog&>(dialog).targetLabel().toStdString();
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

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
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.shiftAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("a shift that would go before the origin is refused, and names the subtitle",
          "[gui][GUI-SHIFT-01]") {
    // A negative position is representable — the core says so — but no
    // subtitle file can hold one. The refusal names the first offender, which
    // is the one to look at.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("-0:02,000"));
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.shiftAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.at(0).find("subtitle 1") != std::string::npos);
    CHECK(startAt(window, 0) == "00:00:01,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("transforming corrects the file from two references", "[gui][GUI-TRANSFORM-01]") {
    // The first stays where it is, the last goes twice as far: everything
    // stretches between the two, and both references land exactly.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TransformDialog&>(dialog).setTyped(
            1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:13,000"));
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

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
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.frameRateAction()->trigger();

    // Twenty-five frames per second played back at twenty-four: everything
    // lasts longer, in the ratio 25/24.
    CHECK(startAt(window, 0) == "00:00:01,042");
    CHECK(window.undoAction()->text().toStdString() == "Undo: converting the frame rate");
}

TEST_CASE("the three operations are reachable from a menu of their own", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    CHECK(window.shiftAction()->isEnabled());
    CHECK(window.transformAction()->isEnabled());
    CHECK(window.frameRateAction()->isEnabled());
}

TEST_CASE("an operation on an empty file is not offered", "[gui][GUI-SHIFT-01]") {
    // Nothing to shift, nothing to transform: the actions are disabled rather
    // than opening a dialog that could apply to nothing.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    CHECK_FALSE(window.shiftAction()->isEnabled());
    CHECK_FALSE(window.transformAction()->isEnabled());
    CHECK_FALSE(window.frameRateAction()->isEnabled());
}

TEST_CASE("the transform dialog shows the subtitles its numbers name", "[gui][GUI-TRANSFORM-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string shown;
    prompts.fill = [&shown](QDialog& dialog) {
        auto& transform = dynamic_cast<TransformDialog&>(dialog);
        shown = transform.firstCurrent().toStdString() + "|" + transform.firstText().toStdString() +
                "|" + transform.secondCurrent().toStdString() + "|" +
                transform.secondText().toStdString();
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.transformAction()->trigger();

    // The first and the last subtitle of the file, as the table reads them.
    CHECK(shown == "00:00:01,000|Un.|00:00:07,000|Quatre.");
}

TEST_CASE("giving up on the transform dialog applies nothing", "[gui][GUI-TRANSFORM-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.transformAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("giving up on the frame rate dialog applies nothing", "[gui][GUI-FRAMERATE-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = false;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.frameRateAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

// The three cases below cannot happen to a user: the accept button follows
// `isComplete()`, so an incomplete dialog is not validated. They are guards,
// and a guard no test walks through is a promise nobody checks — the fake
// `Prompts` validates without looking at the button, which is exactly the
// situation they protect against.

TEST_CASE("a shift validated on an unreadable duration does nothing", "[gui][GUI-SHIFT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QStringLiteral("bientôt"));
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

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
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.transformAction()->trigger();

    CHECK(startAt(window, 3) == "00:00:07,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("two references on one subtitle are refused, and said so", "[gui][GUI-TRANSFORM-01]") {
    // The core returns `nullopt` on a zero denominator. The window says so
    // rather than doing nothing for no apparent reason.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<TransformDialog&>(dialog).setTyped(
            2, QStringLiteral("00:00:01,000"), 2, QStringLiteral("00:00:09,000"));
    };
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    window.transformAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.at(0).find("no correction") != std::string::npos);
    CHECK(startAt(window, 1) == "00:00:03,000");
}
