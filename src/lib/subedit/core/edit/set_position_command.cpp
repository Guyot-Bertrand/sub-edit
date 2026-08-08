#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/project.hpp>

namespace subedit::core {

SetPositionCommand::SetPositionCommand(const Project& project,
                                       SubtitleIndex index,
                                       Boundary boundary,
                                       Timestamp position)
    : m_index(index),
      m_boundary(boundary),
      m_newPosition(position),
      m_oldPosition(project.subtitleAt(index).position(boundary)) {}

void SetPositionCommand::apply(Project& project) {
    project.subtitleAt(m_index).position(m_boundary) = m_newPosition;
}

void SetPositionCommand::revert(Project& project) {
    project.subtitleAt(m_index).position(m_boundary) = m_oldPosition;
}

} // namespace subedit::core
