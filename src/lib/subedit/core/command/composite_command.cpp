#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/composite_command.hpp>

#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace subedit::core {

CompositeCommand::CompositeCommand(CommandKind kind, std::vector<std::unique_ptr<Command>> commands)
    : m_kind(kind), m_commands(std::move(commands)) {}

void CompositeCommand::apply(Project& project) {
    for (const std::unique_ptr<Command>& command : m_commands)
        command->apply(project);
}

void CompositeCommand::revert(Project& project) {
    for (const std::unique_ptr<Command>& command : std::views::reverse(m_commands))
        command->revert(project);
}

std::vector<Change> CompositeCommand::describe() const {
    std::vector<Change> changes;
    changes.reserve(m_commands.size());
    for (const std::unique_ptr<Command>& command : m_commands) {
        std::vector<Change> own = command->describe();
        changes.insert(changes.end(),
                       std::make_move_iterator(own.begin()),
                       std::make_move_iterator(own.end()));
    }
    return changes;
}

} // namespace subedit::core
