#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <memory>

namespace subedit::core {

class Project;
class Selection;

/// Tells whether a single button pressed over `selection` should put it in
/// italics rather than take its italics out.
///
/// **True as soon as one subtitle is not italic**, which is Gaupol's rule and
/// the only one that makes a single button usable: a mixed selection goes to
/// italics whole, so pressing twice leaves it plain rather than inverted
/// subtitle by subtitle.
[[nodiscard]] bool
wouldItalicise(const Project& project, const Selection& selection, Document document);

/// Builds the command that puts the texts of `selection` in italics, or takes
/// their italics out.
///
/// **Told which way rather than deciding**, unlike the toggle above. The window
/// has one button and asks `wouldItalicise` what it means; a command line will
/// have two words and will not ask. One operation, two doors, and the decision
/// stays where it belongs — with whoever has a user in front of them.
///
/// Returns **nothing when no text changes**. An empty group would apply without
/// doing anything and still push an entry the user would meet in « undo »
/// without understanding it — the same reason as for the removal of mentions.
[[nodiscard]] std::unique_ptr<Command>
setItalics(const Project& project, const Selection& selection, Document document, bool italic);

/// How many subtitles `command` rewrites, read from the command rather than by
/// counting again.
///
/// `describe()` already says it: one change of text per subtitle rewritten.
/// Comparing the texts before and after would give a second answer to one
/// question, and the one shown would be the one no test compares — the same
/// reason `tallyOf` exists beside the removal of mentions.
[[nodiscard]] std::size_t italicisedCount(const Command& command);

} // namespace subedit::core
