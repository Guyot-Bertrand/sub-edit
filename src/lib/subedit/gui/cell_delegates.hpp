#pragma once

#include <QStyledItemDelegate>

class QEvent;
class QModelIndex;
class QObject;
class QStyleOptionViewItem;
class QWidget;

namespace subedit::gui {

/// The editor a text cell opens: a multiline field.
class TextDelegate final : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    [[nodiscard]] QWidget* createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
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
