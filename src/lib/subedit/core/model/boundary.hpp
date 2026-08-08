#pragma once

namespace subedit::core {

/// One of the two ends of a subtitle on the timeline.
///
/// The counterpart of `Document` for positions: an operation that moves one
/// end or the other names it with this enumeration rather than with a flag,
/// and the compiler holds the two apart.
enum class Boundary {
    Start,
    End,
};

} // namespace subedit::core
