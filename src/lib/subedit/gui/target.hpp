#pragma once

#include <subedit/core/model/selection.hpp>

class QItemSelectionModel;

namespace subedit::core {
class Project;
} // namespace subedit::core

namespace subedit::gui {

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
