#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/dialogue_dashes.hpp>

#include <algorithm>
#include <string>

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

    return rewriteTexts(project,
                        selection,
                        document,
                        dashed ? CommandKind::AddDialogueDashes : CommandKind::RemoveDialogueDashes,
                        [dashed, format](const std::string& text) {
                            return dashed ? withDialogueDashes(text, format)
                                          : withoutDialogueDashes(text, format);
                        });
}

} // namespace subedit::core
