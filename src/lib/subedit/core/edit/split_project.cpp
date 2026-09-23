#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <expected>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace subedit::core {

std::expected<SplitProject, SplitRefusal> splitProject(const Project& project, SubtitleIndex from) {
    if (from.value() == 0 || from.value() >= project.count())
        throw std::out_of_range{"splitProject: nothing would stay, or nothing would leave"};

    const SubtitleIndex last = SubtitleIndex::fromValue(project.count() - 1);
    const Selection tail = Selection::range(from, last);

    // Minus the end of the last subtitle that stays — the append's offset, with
    // the sign the other way round.
    const SubtitleIndex kept = SubtitleIndex::fromValue(from.value() - 1);
    const Duration by = Timestamp::origin() - project.subtitleAt(kept).end;

    if (const auto before = firstBeforeOrigin(project, tail, by); before.has_value())
        return std::unexpected{SplitRefusal{.before = *before}};

    std::vector<Subtitle> moved{project.subtitles().begin() +
                                    static_cast<std::ptrdiff_t>(from.value()),
                                project.subtitles().end()};
    for (Subtitle& subtitle : moved) {
        subtitle.start += by;
        subtitle.end += by;
    }

    SplitProject result;
    result.tail.setSubtitles(std::move(moved));
    result.tail.setFrameRate(project.frameRate());

    SourceFile main = project.sourceFile();
    main.path.reset();
    result.tail.setSourceFile(std::move(main));
    if (project.translationFile().has_value()) {
        SourceFile translation = *project.translationFile();
        translation.path.reset();
        result.tail.setSourceFile(Document::Translation, std::move(translation));
    }

    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<RemoveCommand>(tail));
    result.command =
        std::make_unique<CompositeCommand>(CommandKind::SplitProject, std::move(commands));
    return result;
}

} // namespace subedit::core
