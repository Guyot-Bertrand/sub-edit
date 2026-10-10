#pragma once

#include <QTableView>

#include <utility>
#include <vector>

namespace subedit::gui {

/// The table of subtitles, and one question Qt keeps to itself.
///
/// **It exists for a single line**, and that line is the one that keeps a film
/// from eating a correction: `QAbstractItemView::state()` is protected, so the
/// only way to ask a view whether a cell is being edited is to be one. Qt
/// means it that way — the state is the view's own business — and deriving is
/// the door it leaves open.
///
/// **It also sets the height of its rows**, and that is the one thing it
/// overrides (`setModel`). Qt's way — `ResizeToContents` — asks the delegate
/// for the size of every cell of every row whenever the model is set or reset,
/// and a project of three thousand subtitles froze the window for most of a
/// second at every change of tab. The height of a row depends on nothing but
/// its number of lines, so it is measured once per number of lines, on a row
/// that has it, and given to the others. No `Q_OBJECT`: it declares neither
/// signal nor slot, and a macro that buys nothing costs a generated file.
class SubtitleTable final : public QTableView {

public:
    using QTableView::QTableView;

    /// Follows `model`: its rows get their heights now, and again whenever the
    /// text of one changes or rows come and go.
    void setModel(QAbstractItemModel* model) override;

    /// Gives every row its height again — for a change the model does not
    /// announce: a column shown or hidden changes which text the rows are as
    /// tall as.
    void refreshRowHeights();

    /// Whether an editor is open on one of the cells.
    ///
    /// What the playback follower asks before moving the current row: moving
    /// it closes whatever editor is open, and a user halfway through typing a
    /// timestamp would watch it vanish.
    [[nodiscard]] bool isEditing() const { return state() == EditingState; }

protected:
    /// A new font or style measures differently: what was learnt is forgotten.
    void changeEvent(QEvent* event) override;

private:
    /// Gives rows `first` to `last` their heights.
    void fitRows(int first, int last);

    /// What a row of `lines` lines measures, by asking the delegate on `row`
    /// the first time and remembering it after.
    [[nodiscard]] int heightOfLines(int lines, int row);

    /// The measured height of a row, by its number of lines.
    std::vector<std::pair<int, int>> m_heights;
    /// Which text columns were shown at the last pass, as a bit per column.
    unsigned m_shownColumns = 0;
};

} // namespace subedit::gui
