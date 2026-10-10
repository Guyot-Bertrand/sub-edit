#pragma once

#include <QDialog>
#include <QString>

#include <cstddef>
#include <functional>
#include <vector>

class QDialogButtonBox;
class QFormLayout;
class QRadioButton;
class QVBoxLayout;

namespace subedit::core {
struct CommandPreview;
}

namespace subedit::gui {

/// What an operation could apply to: the whole project, or the rows selected in it.
///
/// **Two counts, because the dialog needs both to decide whether to ask.** With
/// nothing selected, or everything selected, there is one possible target and no
/// question; with some of the rows selected, there are two, and the dialog
/// offers the choice. A single number could say how many rows an operation
/// touches, never whether the user could have meant another set.
struct OperationScope {
    /// Implicit on purpose: a bare count is the common case — « all of them, and
    /// nothing selected » — and what the tests of a dialog care about is rarely
    /// the selection.
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    OperationScope(std::size_t wholeCount, std::size_t selectedCount = 0)
        : whole(wholeCount), selected(selectedCount) {}

    /// How many subtitles the project holds.
    std::size_t whole;

    /// How many rows are selected; zero when none is.
    std::size_t selected;

    /// Whether the user has a choice to make: some rows selected, not all.
    [[nodiscard]] bool offersChoice() const { return selected > 0 && selected < whole; }
};

/// One line of a preview, as the dialog shows it.
struct PreviewRow {
    QString number;
    QString before;
    QString after;
    QString text;
};

/// What a preview says: the first changes, and how many there are in all.
struct OperationPreview {
    std::vector<PreviewRow> rows;
    std::size_t changed = 0;
};

/// A preview as the dialog shows it, from what the core computed.
[[nodiscard]] OperationPreview describedPreview(const core::CommandPreview& preview);

/// What the operation dialogs have in common.
///
/// **A base class, and only for what is genuinely shared**: they all say what
/// they are about to touch — and let the user choose it when the selection
/// leaves a choice —, they all refuse to be validated while what was typed
/// makes no operation, they may all show what the operation would change before
/// it is applied, and they all lay their fields out the same way. What each one
/// asks for is its own business.
///
/// `isComplete()` is the one thing a subclass owes: it is asked after every
/// keystroke, and it is what drives the accept button. A dialog that could be
/// validated on unreadable input would apply something nobody asked for.
class OperationDialog : public QDialog {
    Q_OBJECT

public:
    /// What the operation could apply to, for the choice and the label.
    explicit OperationDialog(OperationScope scope, QWidget* parent = nullptr);

    /// Whether what was typed makes an operation.
    [[nodiscard]] virtual bool isComplete() const = 0;

    /// What the dialog says it is about to touch: « 4 subtitles ».
    ///
    /// Shown because « the selection, or the whole file » is not a rule anyone
    /// guesses in front of a dialog box. It follows the choice when there is
    /// one.
    [[nodiscard]] QString targetLabel() const;

    /// Whether the operation applies to every subtitle, rather than to the
    /// selected rows.
    ///
    /// Without a choice there is no difference: nothing selected means the whole
    /// file, and everything selected is the whole file.
    [[nodiscard]] bool wholeProject() const;

    /// Chooses the whole project, or the selection, as a user would. Only
    /// meaningful when the scope offers a choice.
    void chooseWholeProject(bool whole);

    /// What a preview reads: the first subtitles the operation would change.
    using PreviewProvider = std::function<OperationPreview()>;

    /// Offers a button that shows what the operation would change, from what the
    /// dialog holds at that moment. Opt-in: an operation that has no useful
    /// before and after to show does not call it.
    ///
    /// Called once the dialog is built: the owner of the dialog is the one who
    /// can say what the operation would do, the dialog only knows what was typed.
    void offerPreview(PreviewProvider provider);

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
    void showPreview(const OperationPreview& preview);

    OperationScope m_scope;
    QVBoxLayout* m_stack = nullptr;
    QFormLayout* m_fields;
    QDialogButtonBox* m_buttons;
    QRadioButton* m_selection = nullptr;
    QRadioButton* m_project = nullptr;
};

} // namespace subedit::gui
