#pragma once

#include <QTableView>

namespace subedit::gui {

/// The table of subtitles, and one question Qt keeps to itself.
///
/// **It exists for a single line**, and that line is the one that keeps a film
/// from eating a correction: `QAbstractItemView::state()` is protected, so the
/// only way to ask a view whether a cell is being edited is to be one. Qt
/// means it that way — the state is the view's own business — and deriving is
/// the door it leaves open.
///
/// Nothing else is added, and nothing is overridden. No `Q_OBJECT` either: it
/// declares neither signal nor slot, and a macro that buys nothing costs a
/// generated file.
class SubtitleTable final : public QTableView {

public:
    using QTableView::QTableView;

    /// Whether an editor is open on one of the cells.
    ///
    /// What the playback follower asks before moving the current row: moving
    /// it closes whatever editor is open, and a user halfway through typing a
    /// timestamp would watch it vanish.
    [[nodiscard]] bool isEditing() const { return state() == EditingState; }
};

} // namespace subedit::gui
