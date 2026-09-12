#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>

#include <memory>

namespace subedit::core {

class Project;
class Selection;

/// Tells whether the single entry should put dialogue dashes on `selection`
/// rather than take them off.
///
/// **True as soon as one line of one subtitle has none**, which is Gaupol's
/// rule and the one the italic toggle already follows: the target goes one way
/// whole, so pressing twice leaves it as it was found.
[[nodiscard]] bool
wouldAddDialogueDashes(const Project& project, const Selection& selection, Document document);

/// Builds the command that puts dialogue dashes on, or takes them off.
///
/// Returns nothing when no text changes.
[[nodiscard]] std::unique_ptr<Command> setDialogueDashes(const Project& project,
                                                         const Selection& selection,
                                                         Document document,
                                                         bool dashed);

} // namespace subedit::core
