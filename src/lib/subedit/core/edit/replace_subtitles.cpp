#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/replace_subtitles.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <utility>

namespace subedit::core {

std::unique_ptr<Command> replaceAllSubtitles(const Project& project,
                                             std::vector<Subtitle> subtitles) {
    std::vector<std::unique_ptr<Command>> commands;
    if (project.count() > 0)
        commands.push_back(std::make_unique<RemoveCommand>(Selection::all(project)));
    if (!subtitles.empty())
        commands.push_back(
            std::make_unique<InsertCommand>(SubtitleIndex::fromValue(0), std::move(subtitles)));
    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(CommandKind::Transcribe, std::move(commands));
}

} // namespace subedit::core
