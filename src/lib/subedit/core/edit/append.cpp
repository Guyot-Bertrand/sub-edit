#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace subedit::core {

AppendedFile appendFile(const Project& project, const Project& appended) {
    ConvertedProject converted =
        convertProjectFor(appended, project.sourceFile().format, project.frameRate());

    AppendedFile result{.loss = converted.loss};
    if (converted.subtitles.empty())
        return result;

    // Nothing is inserted between the two files: the appended one starts
    // exactly where the project's own last subtitle ends. An empty project has
    // no end to shift from, and the appended file lands as it stands.
    const Duration offset = project.subtitles().empty()
                                ? Duration::zero()
                                : project.subtitles().back().end - Timestamp::origin();

    for (Subtitle& subtitle : converted.subtitles) {
        subtitle.start += offset;
        subtitle.end += offset;
        // A file opened on its own has no translation to give — D6.
        subtitle.translationText.clear();
    }

    result.inserted = converted.subtitles.size();

    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<InsertCommand>(SubtitleIndex::fromValue(project.count()),
                                                       std::move(converted.subtitles)));
    result.command = std::make_unique<CompositeCommand>(CommandKind::Append, std::move(commands));
    return result;
}

} // namespace subedit::core
