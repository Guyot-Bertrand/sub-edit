#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/sort_command.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace subedit::core {

Session::Session(Project project, OrderPolicy policy)
    : m_project(std::move(project)), m_policy(policy) {
    if (m_policy == OrderPolicy::Strict && !m_project.outOfOrder().empty())
        m_history.apply(std::make_unique<SortCommand>(), m_project);
}

void Session::apply(std::unique_ptr<Command> command) {
    if (m_policy != OrderPolicy::Strict || !mayBreakOrder(command->kind())) {
        m_history.apply(std::move(command), m_project);
        return;
    }

    // The group keeps the name of what was asked for: the sort is how the
    // policy is honoured, not a second operation the user performed.
    const CommandKind kind = command->kind();

    std::vector<std::unique_ptr<Command>> group;
    group.reserve(2);
    group.push_back(std::move(command));
    group.push_back(std::make_unique<SortCommand>());

    m_history.apply(std::make_unique<CompositeCommand>(kind, std::move(group)), m_project);
}

} // namespace subedit::core
