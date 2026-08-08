#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <vector>

namespace subedit::core {

class Project;

/// Adds subtitles at one place in a project.
///
/// What it retains to undo itself is **where it put them and how many** — the
/// subtitles it inserted are its own, given to it, so it already holds them.
class InsertCommand final : public Command {

public:
    /// Inserts `subtitles` before `index`, in the order given.
    InsertCommand(SubtitleIndex index, std::vector<Subtitle> subtitles);

    /// Builds a command inserting `count` blank subtitles before `index`.
    ///
    /// They share the room between the subtitle before them and the one after,
    /// in equal parts; with nothing after them, they last three seconds each.
    /// This is Gaupol's rule, and it is the one users expect from « insert »
    /// on a full file.
    ///
    /// The positions are computed **now**, from `project` as it stands, which
    /// is what every other command does with the state it captures.
    [[nodiscard]] static InsertCommand
    blank(const Project& project, SubtitleIndex index, std::size_t count);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Insert; }

    /// Reports the indices the insertion fills.
    [[nodiscard]] std::vector<Change> describe() const override;

private:
    SubtitleIndex m_index;
    std::vector<Subtitle> m_subtitles;
};

} // namespace subedit::core
