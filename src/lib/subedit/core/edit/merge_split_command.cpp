#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/merge_split_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/ratio.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Glues the texts of `document` over `run`, one line break between two,
/// skipping the empty ones.
[[nodiscard]] std::string gluedTexts(const Project& project, IndexRange run, Document document) {
    std::string glued;
    for (std::size_t value = run.first.value(); value <= run.last.value(); ++value) {
        const std::string& text =
            project.subtitleAt(SubtitleIndex::fromValue(value)).text(document);
        if (text.empty())
            continue;

        if (!glued.empty())
            glued += '\n';
        glued += text;
    }
    return glued;
}

/// Returns the position halfway through `subtitle`.
[[nodiscard]] Timestamp middleOf(const Subtitle& subtitle) {
    // Through `Ratio` rather than by dividing here, as the insertion does: it
    // is a rounding decision, and the project takes them in one place. One half
    // is always a ratio, so the fallback is never taken — it only spares a
    // branch no test could reach.
    const Ratio half = Ratio::create(1, 2).value_or(Ratio::one());
    return subtitle.start + subtitle.duration().scaledBy(half);
}

/// Replaces `run` with `subtitles`, as one entry named `kind`.
[[nodiscard]] std::unique_ptr<Command>
replaced(CommandKind kind, IndexRange run, std::vector<Subtitle> subtitles) {
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<RemoveCommand>(Selection::range(run.first, run.last)));
    commands.push_back(std::make_unique<InsertCommand>(run.first, std::move(subtitles)));
    return std::make_unique<CompositeCommand>(kind, std::move(commands));
}

} // namespace

std::unique_ptr<Command> mergeSubtitles(const Project& project, IndexRange run) {
    if (run.count() < 2)
        return nullptr;

    // The first, and not a new subtitle: what it carries beyond positions and
    // text survives the merge.
    Subtitle merged = project.subtitleAt(run.first);
    merged.end = project.subtitleAt(run.last).end;
    merged.mainText = gluedTexts(project, run, Document::Main);
    merged.translationText = gluedTexts(project, run, Document::Translation);

    std::vector<Subtitle> subtitles;
    subtitles.push_back(std::move(merged));
    return replaced(CommandKind::Merge, run, std::move(subtitles));
}

std::unique_ptr<Command> splitSubtitle(const Project& project, SubtitleIndex index) {
    const Subtitle& whole = project.subtitleAt(index);
    const Timestamp middle = middleOf(whole);

    Subtitle first = whole;
    first.end = middle;

    std::vector<Subtitle> subtitles;
    subtitles.push_back(std::move(first));
    subtitles.push_back(Subtitle{.start = middle, .end = whole.end});
    return replaced(
        CommandKind::Split, IndexRange{.first = index, .last = index}, std::move(subtitles));
}

} // namespace subedit::core
