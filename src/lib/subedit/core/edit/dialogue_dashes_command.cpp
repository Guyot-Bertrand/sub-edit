#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/dialogue_dashes.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

bool wouldAddDialogueDashes(const Project& project, const Selection& selection, Document document) {
    const SubtitleFormat format = project.sourceFile().format;

    return std::ranges::any_of(selection.indices(), [&](SubtitleIndex index) {
        return wantsDialogueDashes(project.subtitleAt(index).text(document), format);
    });
}

std::unique_ptr<Command> setDialogueDashes(const Project& project,
                                           const Selection& selection,
                                           Document document,
                                           bool dashed) {
    const SubtitleFormat format = project.sourceFile().format;
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written =
            dashed ? withDialogueDashes(text, format) : withoutDialogueDashes(text, format);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(dashed ? CommandKind::AddDialogueDashes
                                                     : CommandKind::RemoveDialogueDashes,
                                              std::move(commands));
}

} // namespace subedit::core
