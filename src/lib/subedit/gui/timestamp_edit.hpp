#pragma once

#include <QLineEdit>
#include <QString>

#include <optional>

class QFocusEvent;
class QKeyEvent;

namespace subedit::gui {

/// A field to type a time in, the way Gaupol does: `HH:MM:SS,mmm`, whose shape never changes.
///
/// - **Overwrite, not insert**: a digit replaces the digit under the cursor and the cursor moves
/// on,
///   so the field is always as long as it was;
/// - **digits only** — and `-` where a negative time makes sense: no letter, no stray separator;
/// - **the separators are skipped**: the cursor jumps over `:` and `,` while one types.
///
/// It is still a `QLineEdit` whose `text()` is the time as `Timestamp::parse` reads it, so what
/// reads a field of text reads this one. **A text that is given to it** (a value to start from, a
/// paste) **is put into shape when the time it holds is readable**, so `1:02` becomes
/// `00:01:02,000`; a text that is not readable is left alone — it is what a caller that set it
/// wants to see, and the reading refuses it as it always did.
///
/// Used wherever a time is typed in a fixed shape: shifting, the two-point transformation, the
/// range of a transcription and the position cells of the table. One input behaviour, not one copy
/// per dialog.
class TimestampEdit : public QLineEdit {
    Q_OBJECT

public:
    /// What the field takes.
    struct Shape {
        /// Whether `-` is typed: a shift, a position before the film. Off for a duration.
        bool negative = false;

        /// Whether the milliseconds are there (`HH:MM:SS,mmm`) or not (`HH:MM:SS`).
        bool milliseconds = true;

        /// Whether the field may be empty, which then means "no time". Typing into an empty field
        /// begins at the first digit; deleting the whole selection empties it.
        bool optional = false;
    };

    explicit TimestampEdit(Shape shape, QWidget* parent = nullptr);

    [[nodiscard]] Shape shape() const { return m_shape; }

    /// `text` put into the shape of `shape`, or nothing when it holds no readable time (or one the
    /// shape cannot write: a negative time for a field with no sign). Public for the tests, which
    /// check the shape without a keyboard.
    [[nodiscard]] static std::optional<QString> shaped(const QString& text, Shape shape);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;

private:
    /// The number of digits the shape holds: 9, or 6 without milliseconds.
    [[nodiscard]] int digitCount() const;

    /// The place in the text of digit `index` (or just past the last, for `digitCount()`).
    [[nodiscard]] int positionOf(int index, bool negative) const;

    /// How many digits lie before `position`.
    [[nodiscard]] int digitsBefore(int position) const;

    void typeDigit(int digit);
    void erase(bool backwards);
    void toggleSign(bool negative);
    void moveBy(int digits);
    void putIntoShape();

    Shape m_shape;
    bool m_shaping = false;
};

} // namespace subedit::gui
