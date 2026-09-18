#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/italics.hpp>

#include <algorithm>
#include <string>

namespace subedit::core {

bool wouldItalicise(const Project& project, const Selection& selection, Document document) {
    const SubtitleFormat format = project.sourceFile().format;

    return std::ranges::any_of(selection.indices(), [&](SubtitleIndex index) {
        // A subtitle with nothing in it says nothing about the question: it is
        // never put in italics, so counting it as « not italic yet » would make
        // a selection of italics ask to be italicised again.
        const std::string& text = project.subtitleAt(index).text(document);
        return !withoutItalics(text, format).empty() && !opensInItalics(text, format);
    });
}

std::unique_ptr<Command>
setItalics(const Project& project, const Selection& selection, Document document, bool italic) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project,
                        selection,
                        document,
                        italic ? CommandKind::Italicise : CommandKind::Unitalicise,
                        [italic, format](const std::string& text) {
                            return italic ? inItalics(text, format) : withoutItalics(text, format);
                        });
}

} // namespace subedit::core
