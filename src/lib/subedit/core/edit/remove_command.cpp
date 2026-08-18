#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/model/project.hpp>

#include <cstddef>
#include <span>

namespace subedit::core {

void RemoveCommand::apply(Project& project) {
    m_removed = project.remove(m_selection);
}

void RemoveCommand::revert(Project& project) {
    project.restore(m_selection, m_removed);
    m_removed.clear();
}

} // namespace subedit::core
