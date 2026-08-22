// The operation dialogs — issue #132.
//
// **They are tested without ever entering `exec()`.** They are our own widgets:
// a test builds one, fills its fields and reads what it makes of them. Only the
// modal loop stays out of reach, and it is behind `Prompts::run`.

#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QLineEdit>
#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;
using subedit::gui::FrameRateDialog;
using subedit::gui::ShiftDialog;
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
    TransformDialog dialog{4, 4};

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
    TransformDialog dialog{4, 4};

    dialog.setTyped(2, QStringLiteral("00:00:01,000"), 2, QStringLiteral("00:00:09,000"));

    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("a reference outside the file cannot be asked for", "[gui][GUI-TRANSFORM-01]") {
    // Not refused afterwards, but impossible to type: the field is bounded by
    // the number of subtitles. A ninth reference in a file of four falls back
    // to the fourth.
    TransformDialog dialog{4, 4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 9, QStringLiteral("00:00:09,000"));

    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("the transform dialog counts its target apart from its bounds",
          "[gui][GUI-TRANSFORM-01]") {
    // Two counts, and they do not say the same thing. The operation applies to
    // two subtitles; a reference is still a subtitle number, so it goes up to
    // the last of the file, selected or not.
    TransformDialog dialog{2, 4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:09,000"));

    CHECK(dialog.targetLabel().toStdString() == "2 subtitles");
    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("an unreadable reference position is refused", "[gui][GUI-TRANSFORM-01]") {
    TransformDialog dialog{4, 4};

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
