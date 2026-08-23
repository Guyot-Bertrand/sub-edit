#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/history.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>

#include <filesystem>
#include <memory>
#include <optional>
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

    /// Records that the document now lives somewhere else, or in another
    /// format.
    ///
    /// **Not a command, and deliberately so.** « Save as » changes where a
    /// document lives, not what it holds; nobody would want to undo it, and
    /// the history stays a faithful account of what was done to the subtitles.
    /// The rule it looks like an exception to — inside a session the only road
    /// to a change is a command — is about the document, and this is not the
    /// document.
    void setSourceFile(SourceFile source) { m_project.setSourceFile(std::move(source)); }

    /// Associates the video the user named, whatever was there before.
    ///
    /// **Not a command either, and for the same reason as `setSourceFile`.**
    /// Which film a document is watched against is not the document: no
    /// subtitle changes, the file on disk does not start differing from what
    /// is held, and undoing a shift has no business un-associating a film.
    void chooseVideo(std::filesystem::path path) { m_project.chooseVideo(std::move(path)); }

    /// Offers the video the naming convention found, and says whether it was
    /// taken. A choice is never replaced by a guess — decision D5.
    bool proposeVideo(std::filesystem::path path) {
        return m_project.proposeVideo(std::move(path));
    }

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

    /// Names what undoing would defeat, and what redoing would replay.
    ///
    /// Relayed rather than recomputed: the history is the authority, and the
    /// session is only the door a window reaches it through.
    [[nodiscard]] std::optional<CommandKind> nextUndoKind() const {
        return m_history.nextUndoKind();
    }

    [[nodiscard]] std::optional<CommandKind> nextRedoKind() const {
        return m_history.nextRedoKind();
    }

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
