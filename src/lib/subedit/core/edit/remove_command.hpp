#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

class Project;

/// Takes the selected subtitles out of a project.
///
/// What it retains to undo itself is **the subtitles it removed and their
/// indices**. Both are needed: a discontinuous removal cannot be undone by
/// appending what was taken, each subtitle has to go back to its own place.
///
/// The subtitles are collected when the command is applied and not when it is
/// built — `Project::remove` hands them back, which is exactly why it does.
class RemoveCommand final : public Command {

public:
    explicit RemoveCommand(Selection selection) : m_selection(std::move(selection)) {}

    void apply(Project& project) override;

    /// Puts each subtitle back at the index it was taken from.
    ///
    /// Ascending, which is what makes it right: the indices still to come are
    /// all larger, so each insertion lands before them and leaves them where
    /// they were expected.
    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Remove; }

    /// Reports the indices the removal emptied.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Removal,
                       .indices = {m_selection.indices().begin(), m_selection.indices().end()}}};
    }

private:
    Selection m_selection;
    std::vector<Subtitle> m_removed;
};

} // namespace subedit::core
