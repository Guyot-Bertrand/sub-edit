#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <QDialog>
#include <QString>

#include <optional>

class QLabel;
class QPushButton;
class QTableWidget;

namespace subedit::core {
// Declared rather than included: `moc` parses this header, and the chain the
// deduction pulls in is more than it can read. `DiagnosticsButton` does the same
// with `Diagnostic`, for the same reason.
struct FrameRateDeduction;
struct GridRepair;
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
/// **And, since #386, it says which conversion would put the file back on a grid** — the one a
/// conversion made at the wrong rate left it needing — and offers `Convert Frame Rate…` filled
/// with it. Offering is all it does: nothing is applied from here, the conversion dialog still
/// asks, and the history undoes it. Two conversions that fit equally are named and neither is
/// offered.
///
/// **It is the one place the deduction speaks of individual subtitles.**
/// Everywhere else it speaks of the document, and phase 5's rule holds: the
/// grid never marks a row of the table, because the moment a user corrects a
/// position by hand it stops being aligned, and a naive detector would accuse
/// them of their own work. Here the user asked.
class GridAnalysisDialog final : public QDialog {
    Q_OBJECT

public:
    /// Without a conversion to offer: what the deduction says, and nothing else.
    explicit GridAnalysisDialog(const core::FrameRateDeduction& deduction,
                                QWidget* parent = nullptr);

    GridAnalysisDialog(const core::FrameRateDeduction& deduction,
                       const core::GridRepair& repair,
                       QWidget* parent = nullptr);

    /// The button that closes this and asks for `Convert Frame Rate…` to open on the conversion
    /// found — absent when none was.
    [[nodiscard]] QPushButton* convertButton() const { return m_convert; }

    /// The rates of the conversion found, once the button has been pressed, and nothing before.
    [[nodiscard]] std::optional<core::FrameRate> requestedInput() const { return m_input; }

    [[nodiscard]] std::optional<core::FrameRate> requestedOutput() const { return m_output; }

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
    QPushButton* m_convert = nullptr;
    std::optional<core::FrameRate> m_proposedInput{};
    std::optional<core::FrameRate> m_proposedOutput{};
    std::optional<core::FrameRate> m_input{};
    std::optional<core::FrameRate> m_output{};
};

} // namespace subedit::gui
