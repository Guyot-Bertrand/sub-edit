#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>

#include <memory>

namespace subedit::core {

class Project;

/// Builds the command that removes the hearing-impaired mentions of `document`.
///
/// Returns **nothing when no text bites**. An empty group would apply without
/// doing anything and still push an entry the user would meet in « undo »
/// without understanding it.
[[nodiscard]] std::unique_ptr<Command> removeHearingImpaired(const Project& project,
                                                             Document document);

} // namespace subedit::core
