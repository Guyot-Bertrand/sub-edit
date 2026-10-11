#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/duration.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>

namespace subedit::core {

class Project;

/// An interval shorter than this many milliseconds is absorbed by default: about
/// two frames at 25 images a second.
inline constexpr std::int64_t kDefaultShortestMilliseconds = 80;

/// What the operation is asked for, with the defaults the interfaces offer.
struct DisjointSettings {
    /// An interval shorter than this is absorbed by a neighbour — about two
    /// frames at 25 images a second.
    Duration shortest = Duration::fromMilliseconds(kDefaultShortestMilliseconds);

    /// The most lines a merged subtitle may hold; a merged interval with more is
    /// shown in windows of this many lines, one after the other. Zero lifts the
    /// limit.
    std::size_t mostLines = 2;

    /// Puts a dash before the text each subtitle contributes to a merged one.
    bool dashes = false;

    friend bool operator==(const DisjointSettings&, const DisjointSettings&) = default;
};

/// What the operation did and what it could not.
struct DisjointReport {
    /// Groups of subtitles that overlapped each other, replaced.
    std::size_t groups = 0;

    /// Subtitles taken out and subtitles laid down in their place.
    std::size_t replaced = 0;
    std::size_t produced = 0;

    /// Subtitles of the groups that ended up cut in several pieces.
    std::size_t split = 0;

    /// Pieces that hold the texts of several subtitles.
    std::size_t merged = 0;

    /// Merged pieces shown in windows because they held more lines than allowed.
    std::size_t windowed = 0;

    /// Intervals shorter than the limit, taken over by a neighbour.
    std::size_t absorbed = 0;

    /// Subtitles shorter than the limit that were left whole: absorbing one
    /// would drop its text.
    std::size_t tooShort = 0;

    /// Subtitles that end before they start: not guessed, and left where they
    /// are.
    std::size_t endingBeforeStart = 0;

    /// Merged pieces whose subtitles carried different format extras (a style,
    /// a position): the first one's are kept.
    std::size_t extrasDropped = 0;

    friend bool operator==(const DisjointReport&, const DisjointReport&) = default;
};

/// What `makeDisjoint` builds.
struct DisjointRanges {
    /// Nothing when no subtitle overlaps another.
    std::unique_ptr<Command> command;
    DisjointReport report;

    /// From the first row the operation wrote to the last, once applied: what
    /// an interface selects to show where it worked. Nothing when nothing was
    /// done.
    std::optional<IndexRange> touched{};
};

/// Why the operation was not made.
enum class DisjointRefusal {
    /// The subtitles are not in the order of their starts: groups are runs of
    /// neighbours, and out of order the neighbours are not the ones that
    /// overlap. Sort first.
    NotInOrder,
};

/// Builds the command that makes the ranges of `project` disjoint.
///
/// **Nothing is shortened or lengthened.** The timeline is cut at every start
/// and every end inside a group of overlapping subtitles; an interval covered by
/// one subtitle stays that subtitle, an interval covered by several becomes one
/// subtitle whose lines are theirs in the order of their starts. At every
/// instant the same texts are shown — apart from the cases `DisjointReport`
/// counts.
///
/// - **A line is never duplicated**: covering subtitles with the same text give
///   one line, and two neighbouring pieces that end up with the same texts are
///   one subtitle.
/// - **A very short interval is absorbed**: the neighbour whose lines already
///   contain it, else the longer one, extends its range over it.
/// - **More lines than allowed** are shown in windows: three lines become two
///   successive halves of the interval, the first two lines and then the last
///   two.
/// - Texts, translations and the extras of the first covering subtitle follow
///   the same cuts.
///
/// One history entry; a second run finds nothing to do.
[[nodiscard]] std::expected<DisjointRanges, DisjointRefusal>
makeDisjoint(const Project& project, const DisjointSettings& settings);

} // namespace subedit::core
