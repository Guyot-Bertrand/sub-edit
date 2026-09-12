#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/text/letter_case.hpp>

#include <cstddef>
#include <memory>

namespace subedit::core {

class Project;
class Selection;

/// Builds the command that re-cases the texts of `selection`.
///
/// Returns **nothing when no text changes** — a selection already in capitals
/// asked to go into capitals is not an operation to undo.
[[nodiscard]] std::unique_ptr<Command> setLetterCase(const Project& project,
                                                     const Selection& selection,
                                                     Document document,
                                                     LetterCase wanted);

/// How many subtitles `command` rewrites, read from the command rather than by
/// counting again.
[[nodiscard]] std::size_t recasedCount(const Command& command);

} // namespace subedit::core
