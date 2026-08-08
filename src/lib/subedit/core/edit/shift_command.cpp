#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>

namespace subedit::core {

void ShiftCommand::apply(Project& project) {
    moveBy(project, m_shift);
}

void ShiftCommand::revert(Project& project) {
    moveBy(project, -m_shift);
}

void ShiftCommand::moveBy(Project& project, Duration shift) const {
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start += shift;
        subtitle.end += shift;
    }
}

} // namespace subedit::core
