#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace subedit::core {

class Project;
class Selection;

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
[[nodiscard]] std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform);

/// How many subtitles `command` rewrote, read from what it describes rather
/// than counted again — decision D8: the count a caller reports is the count
/// the history would show if the command were undone.
[[nodiscard]] std::size_t rewrittenCount(const Command& command);

} // namespace subedit::core
