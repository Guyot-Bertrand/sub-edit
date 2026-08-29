#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/gui/target.hpp>

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QModelIndexList>

#include <cstddef>
#include <vector>

namespace subedit::gui {

core::Selection selectionOf(const QItemSelectionModel& selection) {
    const QModelIndexList rows = selection.selectedRows();

    std::vector<core::SubtitleIndex> indices;
    indices.reserve(static_cast<std::size_t>(rows.size()));
    for (const QModelIndex& row : rows)
        indices.push_back(core::SubtitleIndex::fromValue(static_cast<std::size_t>(row.row())));

    // `Selection::of` sorts, deduplicates and glues into runs: what Qt hands
    // back follows the order of the clicks, and nobody downstream has to put up
    // with that.
    return core::Selection::of(indices);
}

core::Selection targetOf(const QItemSelectionModel& selection, const core::Project& project) {
    const core::Selection selected = selectionOf(selection);
    return selected.isEmpty() ? core::Selection::all(project) : selected;
}

} // namespace subedit::gui
