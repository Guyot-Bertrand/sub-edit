#pragma once

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

} // namespace subedit::core
