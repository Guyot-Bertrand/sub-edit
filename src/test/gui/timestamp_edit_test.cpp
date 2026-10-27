// The field a time is typed in — issue #805.
//
// What matters is what a person does at the keyboard: digits typed over the digits, the separators
// skipped, a sign where a sign makes sense, and nothing else getting in. The keys are sent as a
// person sends them, to the field itself.

#include <subedit/gui/timestamp_edit.hpp>

#include <QCoreApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QString>
#include <QTest>
#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::gui::TimestampEdit;

TimestampEdit::Shape position() {
    return {.negative = true};
}

TimestampEdit::Shape range() {
    return {.milliseconds = false, .optional = true};
}

void type(TimestampEdit& field, const char* keys) {
    QTest::keyClicks(&field, QString::fromLatin1(keys));
}

} // namespace

TEST_CASE("a field that must hold a time begins as zeros", "[gui][GUI-TIME-01]") {
    const TimestampEdit field{{}};

    CHECK(field.text() == "00:00:00,000");
    CHECK(field.hasAcceptableInput());
}

TEST_CASE("digits are typed over the digits, and the separators are skipped",
          "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setCursorPosition(0);

    type(field, "012345678");

    // Nine digits, nine places: nothing was inserted, the shape never changed.
    CHECK(field.text() == "01:23:45,678");
    CHECK(field.text().size() == 12);
    // A tenth digit has nowhere to go.
    type(field, "9");
    CHECK(field.text() == "01:23:45,678");
}

TEST_CASE("a digit replaces the digit under the cursor", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("00:00:01,500");
    field.setCursorPosition(10);

    type(field, "7");

    CHECK(field.text() == "00:00:01,570");
    CHECK(field.cursorPosition() == 11);
}

TEST_CASE("nothing but digits and the sign gets in", "[gui][GUI-TIME-01]") {
    TimestampEdit field{{}};
    field.setCursorPosition(0);

    type(field, "ab:,;x 1");

    // The letters, the separators and the space are refused; the 1 is typed over the first digit.
    CHECK(field.text() == "10:00:00,000");

    // A duration has no sign.
    type(field, "-");
    CHECK(field.text() == "10:00:00,000");
}

TEST_CASE("the sign is typed where a negative time makes sense", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("00:00:01,500");

    QTest::keyClick(&field, Qt::Key_Minus);
    CHECK(field.text() == "-00:00:01,500");

    // Typing again over a negative time keeps the sign and the shape.
    field.setCursorPosition(1);
    type(field, "2");
    CHECK(field.text() == "-20:00:01,500");

    QTest::keyClick(&field, Qt::Key_Plus);
    CHECK(field.text() == "20:00:01,500");

    // A sign already as asked is left as it is.
    QTest::keyClick(&field, Qt::Key_Plus);
    CHECK(field.text() == "20:00:01,500");
}

TEST_CASE("backspace and delete put zeros back, they do not shorten the field",
          "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("12:34:56,789");
    field.setCursorPosition(12);

    QTest::keyClick(&field, Qt::Key_Backspace);
    CHECK(field.text() == "12:34:56,780");
    CHECK(field.cursorPosition() == 11);

    field.setCursorPosition(0);
    QTest::keyClick(&field, Qt::Key_Delete);
    CHECK(field.text() == "02:34:56,780");
}

TEST_CASE("the arrows move from digit to digit, over the separators", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("12:34:56,789");
    field.setCursorPosition(1);

    // From the second digit of the hours to the first of the minutes: the colon is stepped over.
    QTest::keyClick(&field, Qt::Key_Right);
    CHECK(field.cursorPosition() == 3);
    QTest::keyClick(&field, Qt::Key_Right);
    CHECK(field.cursorPosition() == 4);

    QTest::keyClick(&field, Qt::Key_Left);
    CHECK(field.cursorPosition() == 3);
    QTest::keyClick(&field, Qt::Key_Left);
    CHECK(field.cursorPosition() == 1);
}

TEST_CASE("a text given to the field is put into shape when it can be", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};

    field.setText("1:02");
    CHECK(field.text() == "00:01:02,000");

    field.setText("-0:01,5");
    CHECK(field.text() == "-00:00:01,500");

    // A text that holds no time is left alone, as a caller that set it wants to see it.
    field.setText("bientôt");
    CHECK(field.text() == "bientôt");
}

TEST_CASE("a range bound may be empty, has no milliseconds, and begins at its first digit",
          "[gui][GUI-TIME-01]") {
    TimestampEdit field{range()};
    CHECK(field.text().isEmpty());
    CHECK(field.hasAcceptableInput());

    type(field, "00230");
    CHECK(field.text() == "00:23:00");

    // All of it selected and deleted: empty again.
    field.selectAll();
    QTest::keyClick(&field, Qt::Key_Delete);
    CHECK(field.text().isEmpty());
}

TEST_CASE("the shapes a text is put in", "[gui][GUI-TIME-01]") {
    CHECK(TimestampEdit::shaped("1:30", range()) == "00:01:30");
    CHECK(TimestampEdit::shaped("1:02:03,456", range()) == "01:02:03");
    CHECK_FALSE(TimestampEdit::shaped("-1", range()).has_value());
    // A sign the shape does not take.
    CHECK_FALSE(TimestampEdit::shaped("-0:01", range()).has_value());
    CHECK_FALSE(TimestampEdit::shaped("abc", position()).has_value());
}

TEST_CASE("the edges of the field are respected", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("12:34:56,789");

    // Past the last digit there is nowhere to type, delete or move to.
    field.setCursorPosition(12);
    type(field, "1");
    QTest::keyClick(&field, Qt::Key_Delete);
    QTest::keyClick(&field, Qt::Key_Right);
    CHECK(field.text() == "12:34:56,789");
    CHECK(field.cursorPosition() == 12);

    // Before the first one, Backspace has nothing to put back.
    field.setCursorPosition(0);
    QTest::keyClick(&field, Qt::Key_Backspace);
    QTest::keyClick(&field, Qt::Key_Left);
    CHECK(field.text() == "12:34:56,789");
    CHECK(field.cursorPosition() == 0);

    // Home and End go to the first and the last place, over the sign too.
    QTest::keyClick(&field, Qt::Key_End);
    CHECK(field.cursorPosition() == 12);
    QTest::keyClick(&field, Qt::Key_Minus);
    QTest::keyClick(&field, Qt::Key_Home);
    CHECK(field.cursorPosition() == 1);
    QTest::keyClick(&field, Qt::Key_End);
    CHECK(field.cursorPosition() == 13);
}

TEST_CASE("a text that holds no time is not typed into", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("bientôt");
    field.setCursorPosition(0);

    QTest::keyClick(&field, Qt::Key_Backspace);
    QTest::keyClick(&field, Qt::Key_Delete);

    CHECK(field.text() == "bientôt");
}

TEST_CASE("what is not typing is left to the line edit", "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("12:34:56,789");

    // Select all, and a key that is neither a digit nor a printable character.
    QTest::keyClick(&field, Qt::Key_A, Qt::ControlModifier);
    CHECK(field.selectedText() == "12:34:56,789");
    QTest::keyClick(&field, Qt::Key_Escape);
    CHECK(field.text() == "12:34:56,789");

    // All of it deleted: zeros for a field that must hold a time, the sign kept.
    QTest::keyClick(&field, Qt::Key_Minus);
    field.selectAll();
    QTest::keyClick(&field, Qt::Key_Backspace);
    CHECK(field.text() == "-00:00:00,000");
}

TEST_CASE("entering the field with the keyboard puts the cursor on its first digit",
          "[gui][GUI-TIME-01]") {
    TimestampEdit field{position()};
    field.setText("12:34:56,789");
    field.setCursorPosition(8);

    QFocusEvent entering{QEvent::FocusIn, Qt::TabFocusReason};
    QCoreApplication::sendEvent(&field, &entering);

    CHECK(field.cursorPosition() == 0);

    // A click leaves the cursor where it was put.
    field.setCursorPosition(8);
    QFocusEvent clicked{QEvent::FocusIn, Qt::MouseFocusReason};
    QCoreApplication::sendEvent(&field, &clicked);
    CHECK(field.cursorPosition() == 8);
}
