#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/model/project.hpp>

#include <cstddef>
#include <span>

namespace subedit::core {

void RemoveCommand::apply(Project& project) {
    m_removed = project.remove(m_selection);
}

void RemoveCommand::revert(Project& project) {
    const std::span<const SubtitleIndex> indices = m_selection.indices();
    for (std::size_t rank = 0; rank < m_removed.size(); ++rank)
        project.insert(indices[rank], std::span{&m_removed[rank], 1});

    m_removed.clear();
}

} // namespace subedit::core
