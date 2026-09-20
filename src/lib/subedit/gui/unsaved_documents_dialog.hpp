#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/gui/prompts.hpp>

#include <QDialog>
#include <QList>

#include <span>
#include <vector>

class QCheckBox;
class QPushButton;

namespace subedit::gui {

/// The one question asked when more than one document has unsaved changes.
///
/// **One box for all of them**, which is Gaupol's `MultiCloseDialog` and not a
/// question per document: three projects modified would make three boxes, none
/// of which can be cancelled for the others. A project with a translation
/// already has two documents, so the question is here from the first tranche of
/// the phase and serves the two that follow.
///
/// It lists the modified documents with a box each, all ticked; **Save** writes
/// those still ticked, **Close Without Saving** goes on and loses them all,
/// **Cancel** does not go on at all. With a single modified document the window
/// asks the plain question it always asked, and never builds this one.
///
/// **Cancel is the answer until another is given** — the default of a box closed
/// by its cross or by Escape, which means « do nothing », as it does elsewhere.
class UnsavedDocumentsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit UnsavedDocumentsDialog(std::span<const ModifiedDocument> documents,
                                    QWidget* parent = nullptr);

    /// What the user chose, or `Cancel` when nothing was pressed.
    [[nodiscard]] UnsavedChoice choice() const { return m_choice; }

    /// The documents to write, in the order they were listed — those still
    /// ticked when **Save** was pressed, and none for any other answer.
    [[nodiscard]] std::vector<core::Document> toSave() const;

    /// The boxes, one per document, for a test to tick without clicking.
    [[nodiscard]] const QList<QCheckBox*>& boxes() const { return m_boxes; }

    [[nodiscard]] QPushButton* saveButton() const { return m_save; }

    [[nodiscard]] QPushButton* discardButton() const { return m_discard; }

    [[nodiscard]] QPushButton* cancelButton() const { return m_cancel; }

private:
    /// **Out while nothing is ticked**: « save » with nothing to save would be a
    /// way of closing without saving under a name that says the opposite.
    void refreshSave();

    std::vector<core::Document> m_documents;
    QList<QCheckBox*> m_boxes;
    QPushButton* m_save = nullptr;
    QPushButton* m_discard = nullptr;
    QPushButton* m_cancel = nullptr;
    UnsavedChoice m_choice = UnsavedChoice::Cancel;
    std::vector<core::Document> m_chosen;
};

} // namespace subedit::gui
