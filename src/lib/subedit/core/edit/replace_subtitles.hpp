#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <memory>
#include <vector>

namespace subedit::core {

class Project;

/// Plans replacing **every** subtitle of `project` by `subtitles`, as one entry of the
/// history.
///
/// What a transcription does to the project it was run for: the subtitle section is
/// replaced whole, and a single undo gives the old one back, translation texts
/// included — they are the old subtitles' own. Nothing when there is nothing to do:
/// an empty project given nothing.
///
/// The new subtitles are inserted where the old ones were, at the start; the order they
/// come in is kept.
[[nodiscard]] std::unique_ptr<Command> replaceAllSubtitles(const Project& project,
                                                           std::vector<Subtitle> subtitles);

} // namespace subedit::core
