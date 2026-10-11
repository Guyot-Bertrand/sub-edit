#pragma once

#include <QDialog>
#include <QString>

#include <cstddef>

class QCheckBox;

namespace subedit::gui {

/// What the window says when a file it has just opened has something wrong that
/// it can put right, and the choice it offers: any of the corrections, or none.
///
/// **One box for all of it**, and not one per problem: a file out of order that
/// also overlaps would otherwise ask twice in a row. Each problem found is a
/// line with its own case, ticked; a problem not found is not a line.
///
/// **A proposal and never a decision.** Gaupol sorts at opening, behind a
/// warning, and cannot take it back; here each correction is a command like any
/// other, so « Keep as is » costs nothing and the corrections can be undone. A
/// file edited by hand may be out of order or overlapping on purpose, or for a
/// reason the user wants to see first — the diagnostics list names every one.
///
/// **The ranges can be made disjoint only in order**, since the groups are runs
/// of neighbours: with the sort offered and unticked, the second case goes grey
/// and unticked too, and says so.
///
/// A dialog of ours and not a message box, so that a test reaches it through
/// `Prompts::run` like the others: a `QMessageBox` could only be answered by a
/// human.
class FixesProposalDialog final : public QDialog {
    Q_OBJECT

public:
    /// `repeated` is how many subtitles repeat another one that they touch,
    /// `outOfOrder` how many start before the one above them, `overlapping` how
    /// many start before the one above them ends. **Each is counted once the
    /// corrections above it are made**, since that is where it is made: the
    /// pieces of a text cut in three are the cause of some disorder, and joining
    /// them first may leave none. Zero leaves a line out.
    FixesProposalDialog(std::size_t repeated,
                        std::size_t outOfOrder,
                        std::size_t overlapping,
                        QWidget* parent = nullptr);

    /// What the dialog says, one problem per line, for a test to read it as a
    /// user would.
    [[nodiscard]] QString message() const;

    /// Whether the user wants the repeats joined, the sort, and the ranges made
    /// disjoint.
    [[nodiscard]] bool joinChosen() const;
    [[nodiscard]] bool sortChosen() const;
    [[nodiscard]] bool disjointChosen() const;

    /// The cases, for a test to tick: null when the problem was not found.
    [[nodiscard]] QCheckBox* joinCheck() const { return m_join; }

    [[nodiscard]] QCheckBox* sortCheck() const { return m_sort; }

    [[nodiscard]] QCheckBox* disjointCheck() const { return m_disjoint; }

private:
    QCheckBox* m_join = nullptr;
    QCheckBox* m_sort = nullptr;
    QCheckBox* m_disjoint = nullptr;
};

} // namespace subedit::gui
