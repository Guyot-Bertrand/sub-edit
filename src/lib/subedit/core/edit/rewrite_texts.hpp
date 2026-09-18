#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <concepts>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace subedit::core {

/// Rewrites the text of `document` for every subtitle of `selection` through
/// `transform`, and groups what changed under `kind`.
///
/// **The one loop every text-rewriting operation shared** — italics, letter
/// case, dialogue dashes, cut. Every command is built before any is applied,
/// so that each captures the text as it stands now rather than the one its
/// predecessor left. A subtitle `transform` leaves unchanged contributes
/// neither a command nor a count.
///
/// Returns **nothing when no text changes**. An empty group would apply
/// without doing anything and still push an entry the user would meet in
/// "undo" without understanding it.
///
/// A template rather than a `std::function` parameter: type erasure
/// instantiated a copy path for the closure that nothing executes, counted as
/// an uncovered line.
template<typename Transform>
    requires std::convertible_to<std::invoke_result_t<Transform&, const std::string&>, std::string>
[[nodiscard]] std::unique_ptr<Command> rewriteTexts(const Project& project,
                                                    const Selection& selection,
                                                    Document document,
                                                    CommandKind kind,
                                                    Transform&& transform) {
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

/// How many subtitles `command` rewrote, read from what it describes rather
/// than counted again — decision D8: the count a caller reports is the count
/// the history would show if the command were undone.
[[nodiscard]] std::size_t rewrittenCount(const Command& command);

} // namespace subedit::core
