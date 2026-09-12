#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command> setLetterCase(const Project& project,
                                       const Selection& selection,
                                       Document document,
                                       LetterCase wanted) {
    const SubtitleFormat format = project.sourceFile().format;
    std::vector<std::unique_ptr<Command>> commands;

    // Built before any is applied, so that each captures the text as it stands
    // now rather than the one its predecessor left.
    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written = recased(text, wanted, format);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(CommandKind::ChangeCase, std::move(commands));
}

std::size_t recasedCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
