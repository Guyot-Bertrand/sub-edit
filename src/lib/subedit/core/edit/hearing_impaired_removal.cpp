#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/hearing_impaired.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command> removeHearingImpaired(const Project& project, Document document) {
    std::vector<std::unique_ptr<Command>> commands;
    std::vector<SubtitleIndex> emptied;

    // Every command is built **before** any is applied, which is what makes
    // the group undoable: each `SetTextCommand` captures the text as it stands
    // now, and the indices the removal will use are those of the project as it
    // stands now. Applying as we go would have each command capture the state
    // the previous one left.
    for (std::size_t rank = 0; rank < project.count(); ++rank) {
        const SubtitleIndex index = SubtitleIndex::fromValue(rank);
        const std::string& text = project.subtitleAt(index).text(document);

        const std::optional<std::string> cleaned = withoutHearingImpaired(text);
        if (!cleaned.has_value()) {
            emptied.push_back(index);
            continue;
        }
        if (*cleaned == text)
            continue;

        commands.push_back(std::make_unique<SetTextCommand>(project, index, document, *cleaned));
    }

    if (commands.empty() && emptied.empty())
        return nullptr;

    // The texts first, the removal last. The rewrites move no index, so the
    // ones the removal names are still the right ones when its turn comes —
    // and `CompositeCommand` undoes the group backwards, which puts the
    // subtitles back before their texts are restored.
    //
    // A subtitle the rule emptied is never rewritten: it keeps the text it
    // came with until the removal takes it, so undoing brings it back whole.
    if (!emptied.empty())
        commands.push_back(std::make_unique<RemoveCommand>(Selection::of(emptied)));

    return std::make_unique<CompositeCommand>(CommandKind::RemoveHearingImpaired,
                                              std::move(commands));
}

} // namespace subedit::core
