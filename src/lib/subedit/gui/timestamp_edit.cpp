#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/timestamp_edit.hpp>

#include <QFocusEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>

namespace subedit::gui {

namespace {

// `HH:MM:SS,mmm`: the place of each digit in the text of a time with no sign.
constexpr std::array<int, 9> kDigitPlaces{0, 1, 3, 4, 6, 7, 9, 10, 11};
constexpr int kDigitsWithMilliseconds = 9;
constexpr int kDigitsWithoutMilliseconds = 6;

constexpr std::int64_t kMillisecondsPerSecond = 1000;
constexpr std::int64_t kMillisecondsPerMinute = 60 * kMillisecondsPerSecond;
constexpr std::int64_t kMillisecondsPerHour = 60 * kMillisecondsPerMinute;

/// `value` written in `width` places, with zeros before.
[[nodiscard]] QString padded(int value, int width) {
    return QString::number(value).rightJustified(width, QLatin1Char('0'));
}

constexpr char16_t kLastAscii = 127;

[[nodiscard]] QString zeros(bool negative, bool milliseconds) {
    return QString{negative ? "-" : ""} + (milliseconds ? "00:00:00,000" : "00:00:00");
}

} // namespace

std::optional<QString> TimestampEdit::shaped(const QString& text, Shape shape) {
    const std::optional<core::Timestamp> read = core::Timestamp::parse(text.toStdString());
    if (!read.has_value())
        return std::nullopt;

    std::int64_t milliseconds = read->milliseconds();
    const bool negative = milliseconds < 0;
    if (negative && !shape.negative)
        return std::nullopt;
    milliseconds = std::abs(milliseconds);

    const auto hours = static_cast<int>(milliseconds / kMillisecondsPerHour);
    const auto minutes =
        static_cast<int>(milliseconds % kMillisecondsPerHour / kMillisecondsPerMinute);
    const auto seconds =
        static_cast<int>(milliseconds % kMillisecondsPerMinute / kMillisecondsPerSecond);
    const auto rest = static_cast<int>(milliseconds % kMillisecondsPerSecond);
    QString out = negative ? QStringLiteral("-") : QString{};
    out += QStringLiteral("%1:%2:%3")
               .arg(padded(hours, 2))
               .arg(padded(minutes, 2))
               .arg(padded(seconds, 2));
    if (shape.milliseconds)
        out += QStringLiteral(",%1").arg(padded(rest, 3));
    return out;
}

TimestampEdit::TimestampEdit(Shape shape, QWidget* parent) : QLineEdit(parent), m_shape(shape) {
    // A field that must hold a time begins as zeros, so there is a shape to overwrite; one that may
    // be empty says what is typed in it.
    if (m_shape.optional)
        setPlaceholderText(QString::fromLatin1(m_shape.milliseconds ? "HH:MM:SS,mmm" : "HH:MM:SS"));
    else
        setText(zeros(false, m_shape.milliseconds));
    // What the field accepts as a value, as `hasAcceptableInput` says it: the shape, as permissive
    // as the reading (`Timestamp::parse`) — the keyboard cannot write anything else, but a text
    // given to the field can be.
    const QString time = m_shape.milliseconds
                             ? QStringLiteral(R"(\d{1,2}:\d{1,2}(:\d{1,2})?([.,]\d{1,3})?)")
                             : QStringLiteral(R"((\d{1,2}:){0,2}\d{1,5})");
    const QString sign = m_shape.negative ? QStringLiteral("-?") : QString{};
    const QString empty = m_shape.optional ? QStringLiteral("|") : QString{};
    setValidator(new QRegularExpressionValidator{
        QRegularExpression{QStringLiteral(R"(\s*(%1%2)%3\s*)").arg(sign, time, empty)}, this});
    // A text that is set from outside, or pasted, is put into shape when it can be.
    connect(this, &QLineEdit::textChanged, this, [this] { putIntoShape(); });
}

int TimestampEdit::digitCount() const {
    return m_shape.milliseconds ? kDigitsWithMilliseconds : kDigitsWithoutMilliseconds;
}

int TimestampEdit::positionOf(int index, bool negative) const {
    const int offset = negative ? 1 : 0;
    if (index >= digitCount())
        return offset + kDigitPlaces.at(static_cast<std::size_t>(digitCount() - 1)) + 1;
    return offset + kDigitPlaces.at(static_cast<std::size_t>(std::max(index, 0)));
}

int TimestampEdit::digitsBefore(int position) const {
    int count = 0;
    for (int at = 0; at < std::min(position, static_cast<int>(text().size())); ++at)
        if (text().at(at).isDigit())
            ++count;
    return count;
}

void TimestampEdit::putIntoShape() {
    if (m_shaping)
        return;
    const QString now = text();
    if (now.isEmpty())
        return;
    const std::optional<QString> shaped = TimestampEdit::shaped(now, m_shape);
    if (!shaped.has_value() || *shaped == now)
        return;

    m_shaping = true;
    const int cursor = cursorPosition();
    setText(*shaped);
    setCursorPosition(std::min(cursor, static_cast<int>(shaped->size())));
    m_shaping = false;
}

void TimestampEdit::typeDigit(int digit) {
    // A selection is typed over from its start.
    const int index = digitsBefore(hasSelectedText() ? selectionStart() : cursorPosition());
    QString current = shaped(text(), m_shape).value_or(zeros(false, m_shape.milliseconds));
    if (index >= digitCount())
        return;

    const bool negative = current.startsWith('-');
    current[positionOf(index, negative)] = QChar{'0' + digit};
    m_shaping = true;
    setText(current);
    // Past the digit just typed, and over the separator that follows.
    setCursorPosition(positionOf(index + 1, negative));
    m_shaping = false;
    emit textEdited(current);
}

void TimestampEdit::erase(bool backwards) {
    if (hasSelectedText() && selectedText().size() >= text().size()) {
        // Everything selected and deleted: nothing for an optional field, zeros otherwise.
        const bool wasNegative = text().startsWith('-');
        m_shaping = true;
        setText(m_shape.optional ? QString{} : zeros(wasNegative, m_shape.milliseconds));
        m_shaping = false;
        setCursorPosition(0);
        emit textEdited(text());
        return;
    }
    const std::optional<QString> current = shaped(text(), m_shape);
    if (!current.has_value())
        return;
    const bool negative = current->startsWith('-');
    int index = digitsBefore(hasSelectedText() ? selectionStart() : cursorPosition());
    if (backwards) {
        if (index == 0)
            return;
        --index;
    }
    if (index >= digitCount())
        return;

    QString edited = *current;
    edited[positionOf(index, negative)] = QChar{'0'};
    m_shaping = true;
    setText(edited);
    setCursorPosition(positionOf(index, negative));
    m_shaping = false;
    emit textEdited(edited);
}

void TimestampEdit::toggleSign(bool negative) {
    if (!m_shape.negative)
        return;
    QString current = shaped(text(), m_shape).value_or(zeros(false, m_shape.milliseconds));
    const bool already = current.startsWith('-');
    if (already == negative)
        return;
    const int cursor = cursorPosition();
    if (negative)
        current.prepend('-');
    else
        current.remove(0, 1);
    m_shaping = true;
    setText(current);
    setCursorPosition(std::max(0, cursor + (negative ? 1 : -1)));
    m_shaping = false;
    emit textEdited(current);
}

void TimestampEdit::moveBy(int digits) {
    const bool negative = text().startsWith('-');
    const int index = std::clamp(digitsBefore(cursorPosition()) + digits, 0, digitCount());
    setCursorPosition(positionOf(index, negative));
    deselect();
}

void TimestampEdit::keyPressEvent(QKeyEvent* event) {
    // What is not a digit, a sign or a movement is what a line edit does with it: copy, select all,
    // tab, undo, the paste (put into shape by `textChanged`).
    if (event->matches(QKeySequence::Paste) || event->matches(QKeySequence::Copy) ||
        event->matches(QKeySequence::Cut) || event->matches(QKeySequence::SelectAll) ||
        event->matches(QKeySequence::Undo) || event->matches(QKeySequence::Redo)) {
        QLineEdit::keyPressEvent(event);
        return;
    }
    const bool modified =
        (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) != 0;
    switch (event->key()) {
    case Qt::Key_Backspace:
        erase(true);
        return;
    case Qt::Key_Delete:
        erase(false);
        return;
    case Qt::Key_Left:
        moveBy(-1);
        return;
    case Qt::Key_Right:
        moveBy(1);
        return;
    case Qt::Key_Home:
        setCursorPosition(positionOf(0, text().startsWith('-')));
        deselect();
        return;
    case Qt::Key_End:
        setCursorPosition(positionOf(digitCount(), text().startsWith('-')));
        deselect();
        return;
    case Qt::Key_Minus:
        toggleSign(true);
        return;
    case Qt::Key_Plus:
        toggleSign(false);
        return;
    default:
        break;
    }
    if (!modified && event->text().size() == 1 && event->text().front().isDigit() &&
        event->text().front().unicode() <= kLastAscii) {
        typeDigit(event->text().front().digitValue());
        return;
    }
    // A printable character that is not a digit is swallowed: nothing but digits goes in.
    if (!modified && !event->text().isEmpty() && event->text().front().isPrint())
        return;
    QLineEdit::keyPressEvent(event);
}

void TimestampEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    // A field that is entered with the keyboard begins at its first digit, as one types a time.
    if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason)
        setCursorPosition(positionOf(0, text().startsWith('-')));
}

} // namespace subedit::gui
