#include <subedit/core/command/change.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform) {
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written = transform(text);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(kind, std::move(commands));
}

std::size_t rewrittenCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
