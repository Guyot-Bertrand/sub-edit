#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <memory>

namespace subedit::core {

class Project;

/// Builds the command that merges the subtitles of `run` into one.
///
/// The merged subtitle **starts where the first started and ends where the
/// last ended**, and its texts are those of the run glued by a line break,
/// empty ones skipped — `merge_subtitles`, retained as it is. It takes the
/// place of the first, and keeps what the first carried beyond positions and
/// text: a style, a layer, coordinates. Gaupol builds a new subtitle and loses
/// them; keeping the first's is the answer that invents nothing.
///
/// **A run and not a selection**, and the type is the rule: merging lines one
/// and three would swallow line two under an overlap. Gaupol refuses a
/// discontinuous selection too, only further up, in the window.
///
/// Built as a removal followed by an insertion, **inside one history entry**:
/// both already undo themselves exactly, so the group does too.
///
/// Returns **nothing for a run of one** — merging a subtitle into itself is not
/// an operation to undo.
[[nodiscard]] std::unique_ptr<Command> mergeSubtitles(const Project& project, IndexRange run);

/// Builds the command that splits the subtitle at `index` in two.
///
/// **The cut falls at the middle of the duration**, the first half keeps the
/// whole text and the second is born blank — `split_subtitle`, retained as it
/// is. Cutting the text at its line break was ruled out by the phase-10 spec: a
/// rule that works on two lines and invents on one or three is worse than one
/// that can be predicted.
///
/// The first half is the original subtitle, extras included, ending earlier;
/// the second is a blank subtitle like an insertion lays down.
[[nodiscard]] std::unique_ptr<Command> splitSubtitle(const Project& project, SubtitleIndex index);

} // namespace subedit::core
