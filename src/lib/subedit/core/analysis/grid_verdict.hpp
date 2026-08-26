#pragma once

namespace subedit::core {

/// What the deduction concludes about a document as a whole.
///
/// **The thresholds that separate these three live in the implementation, and
/// deliberately so.** A caller that could read them could re-implement the
/// verdict, and four surfaces re-implementing the same two numbers end up
/// disagreeing. Ask for the verdict; do not recompute it.
enum class GridVerdict {
    Clean,   ///< the positions are on a grid
    Partial, ///< a part of them is, and the rest is not
    Silent,  ///< no candidate explains anything, which is an answer
};

} // namespace subedit::core
