#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>

namespace subedit::core {

std::unique_ptr<Command> setLetterCase(const Project& project,
                                       const Selection& selection,
                                       Document document,
                                       LetterCase wanted) {
    const SubtitleFormat format = project.sourceFile(document).format;

    return rewriteTexts(
        project,
        selection,
        document,
        CommandKind::ChangeCase,
        [wanted, format](const std::string& text) { return recased(text, wanted, format); });
}

} // namespace subedit::core
