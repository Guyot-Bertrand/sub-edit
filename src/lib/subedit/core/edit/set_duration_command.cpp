#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/set_duration_command.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration) {
    const Timestamp newEnd = project.subtitleAt(index).start + duration;
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<SetPositionCommand>(project, index, Boundary::End, newEnd));
    return std::make_unique<CompositeCommand>(CommandKind::SetDuration, std::move(commands));
}

} // namespace subedit::core
