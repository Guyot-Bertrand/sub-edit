#pragma once

#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/time/duration.hpp>

#include <cstddef>
#include <optional>

namespace subedit::core {

class Project;
class Selection;

/// What reaches past the end of the video.
struct BeyondEnd {
    /// How many subtitles end after the video does.
    std::size_t count = 0;

    /// By how much the furthest of them overshoots.
    Duration overshoot = Duration::zero();

    friend bool operator==(const BeyondEnd&, const BeyondEnd&) = default;
};

/// Reports what of `selection` ends after the video does, or nothing.
///
/// **A notice, never a refusal** — decision D4 of the phase-6 spec. A subtitle
/// landing after the closing credits may be exactly what the user meant; a
/// refusal that is wrong costs more than a warning that is ignored. This is the
/// opposite reading from `firstBeforeOrigin`, which refuses, and for a reason
/// that does not apply here: a negative position is one no subtitle file can
/// hold, whereas a position past the end of a film is perfectly writable.
///
/// **Nothing, when the length is unknown**, which is the ordinary state of the
/// program: no video associated, or no video open. What the notice would have
/// said is simply not said, and no operation behaves differently for it.
/// Nothing too for a length of zero or less, which is not a film.
///
/// **Read after the operation, on the state it produced.** That is what the
/// window shows, and it is what keeps this one function from being three:
/// shifting, transforming and converting a frame rate reach past the end in
/// three different ways, and all three arrive here as subtitles that now end
/// too late. Re-deriving each operation's arithmetic to say so beforehand
/// would be three copies of what the commands already compute.
///
/// **`selection` is what the operation touched.** A subtitle nobody moved,
/// already past the end because the associated film is the wrong one, is not
/// this operation's doing and is not counted against it.
///
/// The overshoot is the furthest one, not the last in file order: a document
/// out of order is an ordinary thing to be handed, and what the user needs is
/// how far past the end it reaches.
///
/// A subtitle ending exactly with the video is inside it. Refusing the exact
/// end would turn a bound into a prohibition.
[[nodiscard]] std::optional<BeyondEnd>
beyondEnd(const Project& project, const Selection& selection, std::optional<Duration> length);

/// Whether an operation of this kind can put a subtitle past the end.
///
/// **The other half of « the selection is what the operation touched ».**
/// `beyondEnd` reads the state an operation produced, and reads it over what it
/// was applied to; it cannot tell, from that alone, whether the operation moved
/// anything. Removing hearing-impaired mentions from subtitles that were
/// already past the end would otherwise be reported as having put them there.
///
/// Three kinds answer yes, and decision D4 names the same three: shifting,
/// transforming, converting a frame rate. They overshoot in three different
/// ways and arrive at the same place, which is what `noticeOf` says in three
/// sentences.
///
/// **Editing a start or an end by hand is not among them**, though it moves
/// one: that is a user typing a position they can see, and telling them it
/// lands after the end of the film would be remarking on what they just did.
[[nodiscard]] bool movesPositions(CommandKind kind);

} // namespace subedit::core
