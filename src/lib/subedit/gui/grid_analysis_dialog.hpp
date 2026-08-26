#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QTableWidget;

namespace subedit::core {
// Declared rather than included: `moc` parses this header, and the chain the
// deduction pulls in is more than it can read. `DiagnosticsPanel` does the same
// with `Diagnostic`, for the same reason.
struct FrameRateDeduction;
} // namespace subedit::core

namespace subedit::gui {

/// Shows, in full, what the positions say about the grid they were written on.
///
/// **It reports and changes nothing**, which is why it is not an
/// `OperationDialog`: there is no target, no selection, and no button that
/// applies anything.
///
/// The status bar carries the answer; this carries the working. Two things it
/// says that a single line cannot:
///
/// - **the whole ranking**, so that a reader sees what was close. A file that
///   fits 25 at a hundred and 50 at a hundred is telling them something a
///   verdict alone hides;
/// - **which starts left the grid, and in how many runs.** Many runs of one are
///   positions corrected by hand, one at a time; a few long runs are a section
///   that was retimed. The two read the same by their count and not at all by
///   their cause.
///
/// **It is the one place the deduction speaks of individual subtitles.**
/// Everywhere else it speaks of the document, and phase 5's rule holds: the
/// grid never marks a row of the table, because the moment a user corrects a
/// position by hand it stops being aligned, and a naive detector would accuse
/// them of their own work. Here the user asked.
class GridAnalysisDialog final : public QDialog {
    Q_OBJECT

public:
    GridAnalysisDialog(const core::FrameRateDeduction& deduction, QWidget* parent = nullptr);

    /// What the dialog says above the table, for a test to read what a user
    /// would.
    [[nodiscard]] QString summary() const;

    /// How many candidates the table holds — always the eight.
    [[nodiscard]] int candidateCount() const;

    /// One row of the ranking, as `24 fps` and `99.9%` joined by a space.
    [[nodiscard]] QString candidateAt(int row) const;

private:
    QLabel* m_summary = nullptr;
    QTableWidget* m_ranking = nullptr;
};

} // namespace subedit::gui
