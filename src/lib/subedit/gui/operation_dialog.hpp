#pragma once

#include <QDialog>

#include <cstddef>

class QDialogButtonBox;
class QFormLayout;
class QString;

namespace subedit::gui {

/// What the three operation dialogs have in common.
///
/// **A base class, and only for what is genuinely shared**: they all say what
/// they are about to touch, they all refuse to be validated while what was
/// typed makes no operation, and they all lay their fields out the same way.
/// What each one asks for is its own business.
///
/// `isComplete()` is the one thing a subclass owes: it is asked after every
/// keystroke, and it is what drives the accept button. A dialog that could be
/// validated on unreadable input would apply something nobody asked for.
class OperationDialog : public QDialog {
    Q_OBJECT

public:
    /// How many subtitles the operation would touch, for the label.
    explicit OperationDialog(std::size_t targetCount, QWidget* parent = nullptr);

    /// Whether what was typed makes an operation.
    [[nodiscard]] virtual bool isComplete() const = 0;

    /// What the dialog says it is about to touch: « 4 subtitles ».
    ///
    /// Shown because « the selection, or the whole file » is not a rule anyone
    /// guesses in front of a dialog box.
    [[nodiscard]] QString targetLabel() const;

protected:
    /// Where a subclass puts its fields.
    [[nodiscard]] QFormLayout* fields() const { return m_fields; }

    /// Re-asks `isComplete()` and moves the accept button accordingly.
    ///
    /// A subclass calls it whenever one of its fields changes. Not automatic:
    /// what counts as a change belongs to whoever built the field.
    void revalidate();

    /// Puts the layout together once the subclass has added its fields.
    void finish();

private:
    std::size_t m_targetCount;
    QFormLayout* m_fields;
    QDialogButtonBox* m_buttons;
};

} // namespace subedit::gui
