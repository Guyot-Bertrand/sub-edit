#pragma once

#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <optional>

namespace subedit::core {

class Project;

/// Which subtitle is on screen at `when`, or nothing.
///
/// What the window asks ten times a second while a film plays: the replica
/// drawn over the picture is the answer to this question, and so is the row
/// that follows playback. It lives here rather than in the window because it
/// is a question about a document and a position, and nothing about a widget.
///
/// **A subtitle shows from its start to its end, both included.** The same
/// reading as `beyondEnd`, which counts a subtitle ending exactly with the
/// video as inside it: a bound that excluded its own edge would make the last
/// millisecond of every subtitle a hole.
///
/// **The last of those that match, in file order.** Overlapping subtitles are
/// ordinary — two speakers, or a document nobody has cleaned — and the one
/// written last is the one a player draws over the other. Gaupol reads it the
/// same way.
///
/// **A walk, and not a search.** A project may be out of order, which is a
/// state this model holds on purpose since ADR 0008; a binary search over a
/// disordered document would answer confidently and wrongly. What it costs is
/// measured rather than assumed — see `docs/mesures/performances.md`.
[[nodiscard]] std::optional<SubtitleIndex> showingAt(const Project& project, Timestamp when);

} // namespace subedit::core
