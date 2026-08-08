#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>

#include <memory>
#include <vector>

namespace subedit::core {

class Project;

/// A sequence of commands that counts as a single entry in the history.
///
/// An operation built out of several others must not force the user to undo
/// seven times. Grouping is what makes « replace everywhere » one action
/// rather than one per occurrence.
class CompositeCommand final : public Command {

public:
    /// Builds a group named `kind`.
    ///
    /// The name is given rather than taken from the commands held, because a
    /// group is named after what the user asked for and not after everything
    /// it took: a shift that a strict order policy follows with a sort is
    /// still « shift » in the undo menu.
    CompositeCommand(CommandKind kind, std::vector<std::unique_ptr<Command>> commands);

    void apply(Project& project) override;

    /// Reverts the commands **in reverse order**.
    ///
    /// Not a detail: undoing « insert then shift » by undoing the insertion
    /// first would shift subtitles that are no longer there.
    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return m_kind; }

    /// Returns the changes of every command it holds, in order.
    [[nodiscard]] std::vector<Change> describe() const override;

private:
    CommandKind m_kind;
    std::vector<std::unique_ptr<Command>> m_commands;
};

} // namespace subedit::core
