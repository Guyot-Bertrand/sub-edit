#include <subedit/gui/cell_delegates.hpp>

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QWidget>

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

} // namespace

QWidget* TextDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& /*option*/,
                                    const QModelIndex& /*index*/) const {
    // `plainText` is a `QPlainTextEdit`'s USER property, which is all the
    // inherited `setEditorData` and `setModelData` need: they read and write
    // that one. Nothing to override to reach the text.
    auto* editor = new QPlainTextEdit{parent};

    // Otherwise a tab would put a character in the text rather than move to
    // the next cell, which is not what anyone expects of a table.
    editor->setTabChangesFocus(true);
    return editor;
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
