#pragma once

#include <QDialog>
#include <QString>

#include <cstddef>

namespace subedit::gui {

/// What the window says when a file it has just opened is not in order of its
/// start, and the choice it offers: sort now, or keep the file as it is.
///
/// **A proposal and never a decision.** Gaupol sorts at opening, behind a
/// warning, and cannot take it back; here the sort is a command like any other,
/// so « Keep as is » costs nothing and « Sort » can be undone. A file edited by
/// hand may be out of order on purpose, or for a reason the user wants to see
/// first — the anomalies list names every one of them.
///
/// A dialog of ours and not a message box, so that a test reaches it through
/// `Prompts::run` like the others: a `QMessageBox` could only be answered by a
/// human.
class SortProposalDialog final : public QDialog {
    Q_OBJECT

public:
    /// `outOfOrder` is how many subtitles start before the one above them.
    explicit SortProposalDialog(std::size_t outOfOrder, QWidget* parent = nullptr);

    /// What the dialog says, for a test to read it as a user would.
    [[nodiscard]] QString message() const;
};

} // namespace subedit::gui
