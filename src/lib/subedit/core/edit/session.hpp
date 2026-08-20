#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/history.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>

#include <memory>
#include <utility>

namespace subedit::core {

/// A project, its history, and the order policy that binds them.
///
/// What an application holds per open file, and what both the command line of
/// phase 3 and the window of phase 5 will manipulate.
///
/// The three are brought together without being mixed: ADR 0010 ruled out
/// putting a `History` inside a `Project` — `model` would depend on `command`,
/// and a project would stop being copyable.
///
/// **`project()` is constant.** Inside a session the only road to a change is
/// a command, and the compiler holds it. A bare `Project` stays mutable —
/// a reader has to fill one — but whoever holds one is building it, not
/// editing it.
class Session {

public:
    /// Opens a session on `project`.
    ///
    /// Under `OrderPolicy::Strict` a project that arrives out of order is
    /// sorted at once, by a command like any other: it can be undone, and it
    /// marks the document as differing from its file — which it does, since
    /// the file is the disordered one.
    explicit Session(Project project = Project{}, OrderPolicy policy = OrderPolicy::Lenient);

    /// Returns the project, for reading only.
    [[nodiscard]] const Project& project() const { return m_project; }

    [[nodiscard]] OrderPolicy orderPolicy() const { return m_policy; }

    /// Changes the policy for the operations to come.
    ///
    /// Does not sort what is already there: a policy says what happens next,
    /// and reordering the project as a side effect of a setting would be a
    /// change nobody asked for. Apply a `SortCommand` for that.
    void setOrderPolicy(OrderPolicy policy) { m_policy = policy; }

    /// Carries `command` out, makes it undoable, and says what it changed.
    ///
    /// Under `OrderPolicy::Strict`, an operation that could have broken the
    /// order is followed by a sort **inside the same history entry**: one
    /// action for the user, one undo — and one report, holding both.
    std::vector<Change> apply(std::unique_ptr<Command> command);

    [[nodiscard]] bool canUndo() const { return m_history.canUndo(); }

    [[nodiscard]] bool canRedo() const { return m_history.canRedo(); }

    /// Undoes the most recent action, and says what that changed.
    std::vector<Change> undo() { return m_history.undo(m_project); }

    /// Redoes the most recently undone action, and says what that changed.
    std::vector<Change> redo() { return m_history.redo(m_project); }

    /// Returns how far `document` stands from the file on disk.
    [[nodiscard]] int modificationCount(Document document) const {
        return m_history.modificationCount(document);
    }

    [[nodiscard]] bool hasUnsavedChanges(Document document) const {
        return m_history.hasUnsavedChanges(document);
    }

    /// Records that `document` has just been written to disk.
    void markSaved(Document document) { m_history.markSaved(document); }

    [[nodiscard]] std::size_t undoableCount() const { return m_history.undoableCount(); }

    [[nodiscard]] std::size_t redoableCount() const { return m_history.redoableCount(); }

private:
    Project m_project;
    History m_history;
    OrderPolicy m_policy;
};

} // namespace subedit::core
