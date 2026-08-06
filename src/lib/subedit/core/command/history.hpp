#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace subedit::core {

class Project;

/// Two stacks of commands, and one modification counter per document.
///
/// Lives in the core and knows nothing of the interface: the command line of
/// phase 3 needs an undo stack just as much as the window of phase 5, and
/// neither should own it.
class History {

public:
    /// How many entries the history keeps before dropping its oldest.
    ///
    /// **Provisional.** ADR 0010 promises to measure the real footprint before
    /// settling on a figure, and no concrete command exists yet to measure.
    /// The bound is here so that memory stays bounded in the meantime; the
    /// number will be revisited in phase 2, with operations to weigh.
    static constexpr std::size_t kDefaultMaximumEntries = 1000;

    explicit History(std::size_t maximumEntries = kDefaultMaximumEntries)
        : m_maximumEntries(maximumEntries) {}

    /// Carries `command` out on `project` and makes it undoable.
    ///
    /// Drops whatever could have been redone: after a new action, redo would
    /// replay a command whose starting state no longer exists.
    void apply(std::unique_ptr<Command> command, Project& project);

    [[nodiscard]] bool canUndo() const { return !m_undoable.empty(); }

    [[nodiscard]] bool canRedo() const { return !m_redoable.empty(); }

    /// Undoes the most recent command. Does nothing if there is none.
    void undo(Project& project);

    /// Redoes the most recently undone command. Does nothing if there is none.
    void redo(Project& project);

    /// Returns how far `document` stands from the file on disk.
    ///
    /// Zero means « identical to the file ». The value moves by one per action
    /// and back by one per undo, which is why it is an integer and not a
    /// boolean: undoing back to the save point says « unmodified » again, and
    /// a boolean could never take that back. It goes negative when undoing
    /// past a save, which is a difference too.
    [[nodiscard]] int modificationCount(Document document) const {
        return m_modificationCounts.at(static_cast<std::size_t>(document));
    }

    [[nodiscard]] bool hasUnsavedChanges(Document document) const {
        return modificationCount(document) != 0;
    }

    /// Records that `document` has just been written to disk.
    void markSaved(Document document) {
        m_modificationCounts.at(static_cast<std::size_t>(document)) = 0;
    }

    /// Forgets every command, leaving the project as it stands.
    void clear();

    [[nodiscard]] std::size_t undoableCount() const { return m_undoable.size(); }

    [[nodiscard]] std::size_t redoableCount() const { return m_redoable.size(); }

private:
    static constexpr std::size_t kDocumentCount = 2;

    /// Moves the counters of the documents `command` concerns by `shift`.
    void shiftCounters(const Command& command, int shift);

    /// Most recent last, so that the top of the stack is the back.
    std::vector<std::unique_ptr<Command>> m_undoable;
    std::vector<std::unique_ptr<Command>> m_redoable;

    std::array<int, kDocumentCount> m_modificationCounts{};

    std::size_t m_maximumEntries;
};

} // namespace subedit::core
