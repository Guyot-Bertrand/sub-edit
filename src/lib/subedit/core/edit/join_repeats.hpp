#pragma once

#include <subedit/core/command/command.hpp>

#include <cstddef>
#include <memory>

namespace subedit::core {

class Project;

/// What joining the repeated subtitles did.
struct JoinReport {
    /// Runs of repeats, each now one subtitle.
    std::size_t chains = 0;

    /// Subtitles taken out because another one now covers them.
    std::size_t joined = 0;

    friend bool operator==(const JoinReport&, const JoinReport&) = default;
};

/// What `joinRepeats` builds.
struct JoinedRepeats {
    /// Nothing when no subtitle repeats another.
    std::unique_ptr<Command> command;
    JoinReport report;
};

/// Builds the command that joins the subtitles that show the same thing one
/// after the other into one.
///
/// **A repeat** is a subtitle with the same text, the same translation and the
/// same format extras — a style, a position — as another, **starting no later
/// than that one ends**: they touch or overlap. A text cut in three pieces that
/// follow each other by a re-segmentation is the typical case. The run becomes
/// the earliest of them with its end moved to the furthest end of the run; the
/// others are taken out.
///
/// **It does not depend on the order of the file**, and does not change it: the
/// subtitle kept is the one that starts first, wherever the others sat, so a
/// file in order stays in order. Run this **before** sorting and before making
/// the ranges disjoint, which both read a clean sequence better.
///
/// A subtitle with no text in either document, or that ends before it starts, is
/// never joined. One history entry; a second run finds nothing to do.
[[nodiscard]] JoinedRepeats joinRepeats(const Project& project);

} // namespace subedit::core
