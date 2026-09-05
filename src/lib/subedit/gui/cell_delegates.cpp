#include <subedit/gui/cell_delegates.hpp>

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QWidget>

#include <algorithm>

namespace subedit::gui {

namespace {

/// The shape a timestamp may be typed in, and nothing more.
///
/// **It constrains the characters, not the bounds.** `00:70:00,000` goes
/// through, and deliberately: saying that a minute stops at sixty is
/// `Timestamp::parse`'s work, which it already does, and repeating it here
/// would leave two definitions to keep in agreement. What the shape refuses is
/// what the reading would never usefully catch — a letter in the middle of a
/// timestamp.
///
/// Permissive as the reading is: hours optional, one or two digits per field,
/// one to three decimals or none, comma or period.
constexpr auto kPositionPattern = R"(\s*-?\d{1,2}:\d{1,2}(:\d{1,2})?([.,]\d{1,3})?\s*)";

/// The margin a `QTextDocument` keeps on each side of its text.
///
/// **Read from a document rather than written here.** Qt's default is four
/// pixels today, and a four copied into this file would be a second truth that
/// nothing would keep in agreement. Read once and kept: a `QTextDocument` is
/// not free, and this is asked once per row of the table.
[[nodiscard]] int documentMargin() {
    static const int margin = static_cast<int>(QTextDocument{}.documentMargin());
    return margin;
}

/// The pixel the frame and the margin do not account for.
///
/// **Measured, and it is the same under both styles the window serves.** An
/// editor of `lines × spacing + 2 × (frame + margin)` keeps a vertical
/// scrollbar with two arrows and almost no travel, and hides the very line the
/// height was meant to show; one pixel more and it has nowhere to go. Where it
/// comes from inside `QPlainTextEdit` is not written here, because guessing at
/// it would be worse than measuring it — `cell_delegates_test.cpp` opens an
/// editor on one, two and three lines and asks its scrollbar the only question
/// that settles this.
constexpr int kRoundingPixel = 1;

/// What a `QPlainTextEdit` adds around its lines, top and bottom together.
///
/// The frame counts twice, the margin of its document counts twice, and the
/// pixel above once. The style is asked rather than a number written down: the
/// window serves two, and the frame is one pixel under Fusion and two under
/// Windows — eleven pixels here, thirteen there, and both are exactly what an
/// editor needs to lose its scrollbar.
[[nodiscard]] int roomAroundTheLines(const QWidget* widget) {
    const QStyle* style = widget != nullptr ? widget->style() : nullptr;
    const int frame =
        style == nullptr ? 0 : style->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, widget);
    return (2 * (frame + documentMargin())) + kRoundingPixel;
}

/// The multiline field a text cell opens, and the height it asks for.
///
/// **A `QPlainTextEdit` asks for the same height whatever it holds** — a
/// default of several lines, meant for a field of its own — and a table sizing
/// a row around a persistent editor takes it at its word: a two-line subtitle
/// opened a cell two hundred pixels tall. What this one asks for is the height
/// of its document, which is the height its row already has.
///
/// No `Q_OBJECT`: it declares neither signal nor slot, and a macro that buys
/// nothing costs a generated file. `SubtitleTable` is here for the same reason.
class SubtitleEditor final : public QPlainTextEdit {

public:
    using QPlainTextEdit::QPlainTextEdit;

    [[nodiscard]] QSize sizeHint() const override {
        return {QPlainTextEdit::sizeHint().width(), heightOfItsLines()};
    }

    /// What its lines take, room around them included.
    ///
    /// **`QPlainTextDocumentLayout` measures its document in lines**, not in
    /// pixels — the one place in Qt where the height of a `QSizeF` is a count.
    /// That is what is wanted here: a line the editor wrapped for want of width
    /// counts as much as one the user broke.
    [[nodiscard]] int heightOfItsLines() const {
        const auto lines = static_cast<int>(document()->documentLayout()->documentSize().height());
        return (std::max(1, lines) * fontMetrics().lineSpacing()) + roomAroundTheLines(this);
    }
};

} // namespace

QWidget* TextDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& /*option*/,
                                    const QModelIndex& /*index*/) const {
    // `plainText` is a `QPlainTextEdit`'s USER property, which is all the
    // inherited `setEditorData` and `setModelData` need: they read and write
    // that one. Nothing to override to reach the text.
    auto* editor = new SubtitleEditor{parent};

    // Otherwise a tab would put a character in the text rather than move to
    // the next cell, which is not what anyone expects of a table.
    editor->setTabChangesFocus(true);

    // **The editor grows with what is typed into it**, and that closes the one
    // case the row height cannot: the row is two lines tall, the editor with
    // it, and a `Shift+Enter` making a third line would bring back the
    // scrollbar until the edit is validated. The row catches up afterwards, by
    // way of `dataChanged`; this is what holds until then.
    //
    // It grows downwards over the row beneath, which is what an editor is
    // allowed to do and a cell is not.
    connect(editor->document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            editor,
            [editor](const QSizeF&) {
                if (editor->height() < editor->heightOfItsLines())
                    editor->resize(editor->width(), editor->heightOfItsLines());
            });

    return editor;
}

QSize TextDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.rheight() += roomAroundTheLines(option.widget);
    return size;
}

bool TextDelegate::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        // `dynamic_cast` where the type of the event would do: the project's
        // rule refuses to walk down a hierarchy unchecked, and checking a
        // keystroke twice costs nothing measurable.
        const auto* key = dynamic_cast<const QKeyEvent*>(event);
        if (key != nullptr && (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)) {
            // **The line break belongs to the editor.** Handing the keystroke
            // back without validating anything is all there is to do: it is the
            // `QPlainTextEdit` that will make another line of it.
            if (key->modifiers().testFlag(Qt::ShiftModifier))
                return false;

            // Swallowed, and that is the point. The inherited filter
            // validates on `Enter` but hands the keystroke back to the editor —
            // right for a one-line field, disastrous here: the validated text
            // would carry the line break the validation had just refused.
            if (auto* editor = qobject_cast<QWidget*>(object); editor != nullptr) {
                emit commitData(editor);
                emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
                return true;
            }
        }
    }

    // Escape cancels, losing the focus validates: both come from Qt, and
    // nothing here touches them.
    return QStyledItemDelegate::eventFilter(object, event);
}

QWidget* PositionDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& /*option*/,
                                        const QModelIndex& /*index*/) const {
    auto* editor = new QLineEdit{parent};
    editor->setValidator(new QRegularExpressionValidator{
        QRegularExpression{QString::fromUtf8(kPositionPattern)}, editor});
    return editor;
}

} // namespace subedit::gui
