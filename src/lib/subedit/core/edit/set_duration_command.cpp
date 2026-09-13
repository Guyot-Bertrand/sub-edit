#include <subedit/core/edit/set_duration_command.hpp>
#include <subedit/core/model/project.hpp>

namespace subedit::core {

SetDurationCommand::SetDurationCommand(const Project& project,
                                       SubtitleIndex index,
                                       Duration duration)
    : m_index(index),
      m_newEnd(project.subtitleAt(index).start + duration),
      m_oldEnd(project.subtitleAt(index).end) {}

void SetDurationCommand::apply(Project& project) {
    project.subtitleAt(m_index).end = m_newEnd;
}

void SetDurationCommand::revert(Project& project) {
    project.subtitleAt(m_index).end = m_oldEnd;
}

} // namespace subedit::core
