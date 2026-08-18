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
    /// **Measured, as ADR 0010 asked.** On a file of four thousand subtitles,
    /// a full history of a thousand entries cost, before issue #45:
    ///
    /// | what filled it | per entry | for a thousand |
    /// | :------------- | --------: | -------------: |
    /// | editing one text | 270 B | 264 KB |
    /// | shifting the whole file | 32 KB | 31 MB |
    /// | transforming the whole file | 64 KB | 62 MB |
    ///
    /// The first line is what a history really holds — typing in a cell. The
    /// other two are what a user would have to do a thousand times in a row,
    /// each on the entire file, to reach them; and even then the figure was
    /// bounded and survivable. A thousand entries stands.
    ///
    /// **Half of that is gone since issue #45.** The two other lines were the
    /// `Selection` a command carries — one index per subtitle, thirty-two
    /// kilobytes of them — and a whole-file selection is now two bounds. What
    /// is left is the state a command keeps to undo itself: nothing at all for
    /// a shift, which retains a `Duration`; one pair of positions per subtitle
    /// for a transform and for a frame-rate conversion, which cannot be
    /// inverted without them. Shrinking *that* would be an optimisation of
    /// those two commands, not of this bound.
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
