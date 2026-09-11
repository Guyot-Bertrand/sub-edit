#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/italics.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

bool wouldItalicise(const Project& project, const Selection& selection, Document document) {
    const SubtitleFormat format = project.sourceFile().format;

    return std::ranges::any_of(selection.indices(), [&](SubtitleIndex index) {
        const std::string& text = project.subtitleAt(index).text(document);
        // A subtitle with nothing in it says nothing about the question: it is
        // never put in italics, so counting it as « not italic yet » would make
        // a selection of italics ask to be italicised again.
        return !withoutItalics(text, format).empty() && !opensInItalics(text, format);
    });
}

std::unique_ptr<Command>
setItalics(const Project& project, const Selection& selection, Document document, bool italic) {
    const SubtitleFormat format = project.sourceFile().format;

    std::vector<std::unique_ptr<Command>> commands;

    // Every command is built before any is applied, so that each captures the
    // text as it stands now rather than the one its predecessor left.
    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written = italic ? inItalics(text, format) : withoutItalics(text, format);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;

    return std::make_unique<CompositeCommand>(
        italic ? CommandKind::Italicise : CommandKind::Unitalicise, std::move(commands));
}

std::size_t italicisedCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
