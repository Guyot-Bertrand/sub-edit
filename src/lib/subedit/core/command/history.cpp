#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/history.hpp>
#include <subedit/core/model/document.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::array kDocuments = {Document::Main, Document::Translation};

} // namespace

void History::apply(std::unique_ptr<Command> command, Project& project) {
    command->apply(project);
    shiftCounters(*command, 1);

    m_redoable.clear();
    m_undoable.push_back(std::move(command));

    // The oldest entry goes first. Erasing from the front of a vector shifts
    // the rest, which is fine: it happens once per action at most, on a
    // container of pointers.
    if (m_undoable.size() > m_maximumEntries)
        m_undoable.erase(m_undoable.begin());
}

void History::undo(Project& project) {
    if (m_undoable.empty())
        return;

    std::unique_ptr<Command> command = std::move(m_undoable.back());
    m_undoable.pop_back();

    command->revert(project);
    shiftCounters(*command, -1);

    m_redoable.push_back(std::move(command));
}

void History::redo(Project& project) {
    if (m_redoable.empty())
        return;

    std::unique_ptr<Command> command = std::move(m_redoable.back());
    m_redoable.pop_back();

    command->apply(project);
    shiftCounters(*command, 1);

    m_undoable.push_back(std::move(command));
}

void History::clear() {
    m_undoable.clear();
    m_redoable.clear();
}

void History::shiftCounters(const Command& command, int shift) {
    const std::vector<Change> changes = command.describe();

    for (const Document document : kDocuments) {
        const bool concerned = std::ranges::any_of(
            changes, [document](const Change& change) { return affects(change.kind, document); });

        if (concerned)
            m_modificationCounts.at(static_cast<std::size_t>(document)) += shift;
    }
}

} // namespace subedit::core
