#pragma once

#include <subedit/core/model/selection.hpp>

class QItemSelectionModel;

namespace subedit::core {
class Project;
} // namespace subedit::core

namespace subedit::gui {

/// The rows the user actually selected, and nothing more.
///
/// **Empty means empty**, which is the whole difference with `targetOf` below:
/// a removal reads « nothing selected » as « nothing to do », where a shift
/// reads it as « the whole file ». One convention could not serve both, and
/// letting a removal go through `targetOf` would empty a document on a stray
/// `Suppr`.
[[nodiscard]] core::Selection selectionOf(const QItemSelectionModel& selection);

/// What an operation is about to touch: the selected rows, or the whole file.
///
/// **Nothing selected means everything**, which is what makes the dialogs
/// usable: nobody selects four thousand rows to shift a whole file. The two
/// paths meet — selecting every row gives the same answer — and neither is a
/// special case for whoever applies the command.
///
/// Written once and shared by the four dialogs of this phase; the target is
/// the one thing they all need and none of them should decide differently.
[[nodiscard]] core::Selection targetOf(const QItemSelectionModel& selection,
                                       const core::Project& project);

} // namespace subedit::gui
