#pragma once

#include <QSize>
#include <QStyledItemDelegate>

class QEvent;
class QModelIndex;
class QObject;
class QStyleOptionViewItem;
class QWidget;

namespace subedit::gui {

/// The editor a text cell opens: a multiline field, and the height it needs.
///
/// **The height of a row follows the subtitle it carries** — issue #322. It was
/// fixed at one line while the text was not, and that cost twice over. On
/// screen, a two-line subtitle came back as `Premiere ligne....`, an ellipsis
/// falling in the middle of a cell that still had room, saying the same thing
/// as the one that falls at the edge of a line too long. And in the editor,
/// which is handed the geometry of the cell: the second line was cut across the
/// height of its letters, under a scrollbar squeezed into twenty pixels. One
/// edited a text one could not read.
class TextDelegate final : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    [[nodiscard]] QWidget* createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const override;

    /// What the style asks for this text, plus the room its editor needs.
    ///
    /// **The style already counts the line breaks**, once the table has turned
    /// word wrap off — it lays the text out under `ManualWrap`, where only a
    /// real line break makes a line. Nothing here has to count them again, and
    /// two properties come with that: the height stays linear in the length of
    /// the text, and it does not depend on the width of the column, so dragging
    /// a column edge recomputes nothing. A line too long for its column keeps
    /// eliding exactly as it did, which is the one thing an ellipsis should
    /// mean.
    ///
    /// **What the style does not count is the editor**, and this is where it is
    /// added rather than in `updateEditorGeometry`: an editor taller than its
    /// cell would open over the row beneath it, and the two heights have to
    /// agree.
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const override;

    bool eventFilter(QObject* object, QEvent* event) override;
};

/// The editor a position cell opens: a constrained one-line field.
class PositionDelegate final : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    [[nodiscard]] QWidget* createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const override;
};

} // namespace subedit::gui
