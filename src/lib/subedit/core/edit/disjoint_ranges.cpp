#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/disjoint_ranges.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// A stretch of the timeline between two marks, and who covers it.
struct Piece {
    Timestamp start;
    Timestamp end;
    /// Positions in the group of the subtitles that show during the piece, in
    /// the order of their starts.
    std::vector<std::size_t> covers;
    std::string mainText;
    std::string translationText;

    [[nodiscard]] Duration duration() const { return end - start; }
};

[[nodiscard]] std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t from = 0;
    while (true) {
        const std::size_t at = text.find('\n', from);
        if (at == std::string::npos) {
            lines.push_back(text.substr(from));
            return lines;
        }
        lines.push_back(text.substr(from, at - from));
        from = at + 1;
    }
}

[[nodiscard]] std::string joined(std::span<const std::string> lines) {
    std::string text;
    for (std::size_t at = 0; at < lines.size(); ++at) {
        if (at > 0)
            text += '\n';
        text += lines[at];
    }
    return text;
}

/// The text of `document` for the subtitles at `covers`: theirs in order, an
/// empty one skipped, a repeated one said once.
[[nodiscard]] std::string textOf(std::span<const Subtitle> group,
                                 const std::vector<std::size_t>& covers,
                                 Document document,
                                 bool dashes) {
    std::vector<std::string> kept;
    for (const std::size_t position : covers) {
        const std::string& text = group[position].text(document);
        if (!text.empty() && std::ranges::find(kept, text) == kept.end())
            kept.push_back(text);
    }

    if (dashes && kept.size() > 1) {
        for (std::string& text : kept) {
            if (!text.starts_with('-'))
                text.insert(0, "- ");
        }
    }
    return joined(kept);
}

[[nodiscard]] bool sameTexts(const Piece& left, const Piece& right) {
    return left.mainText == right.mainText && left.translationText == right.translationText;
}

/// Whether every line of `inner` is a line of `outer`, in both texts.
[[nodiscard]] bool holds(const Piece& outer, const Piece& inner) {
    const auto contained = [](const std::string& whole, const std::string& part) {
        const std::vector<std::string> lines = linesOf(whole);
        return std::ranges::all_of(linesOf(part), [&lines](const std::string& line) {
            return std::ranges::find(lines, line) != lines.end();
        });
    };
    return contained(outer.mainText, inner.mainText) &&
           contained(outer.translationText, inner.translationText);
}

/// Cuts the timeline of `group` at every start and end.
[[nodiscard]] std::vector<Piece> piecesOf(std::span<const Subtitle> group, bool dashes) {
    std::vector<Timestamp> marks;
    for (const Subtitle& subtitle : group) {
        marks.push_back(subtitle.start);
        marks.push_back(subtitle.end);
    }
    std::ranges::sort(marks);
    const auto repeated = std::ranges::unique(marks);
    marks.erase(repeated.begin(), repeated.end());

    // The group is in the order of its starts, so the subtitles showing at a
    // mark are found by a sweep: added as they start, dropped as they end.
    std::vector<Piece> pieces;
    std::vector<std::size_t> active;
    std::size_t next = 0;
    for (std::size_t mark = 0; mark + 1 < marks.size(); ++mark) {
        const Timestamp from = marks[mark];
        while (next < group.size() && group[next].start <= from)
            active.push_back(next++);
        std::erase_if(active, [&](std::size_t position) { return group[position].end <= from; });

        // Never empty: a group is a run of subtitles each of which starts before
        // the furthest end so far, so no stretch of its timeline is bare.
        pieces.push_back(
            Piece{.start = from,
                  .end = marks[mark + 1],
                  .covers = active,
                  .mainText = textOf(group, active, Document::Main, dashes),
                  .translationText = textOf(group, active, Document::Translation, dashes)});
    }
    return pieces;
}

/// Joins neighbours that show the same texts, and says whether any did.
[[nodiscard]] bool joinedSame(std::vector<Piece>& pieces) {
    bool changed = false;
    for (std::size_t at = 0; at + 1 < pieces.size();) {
        if (!sameTexts(pieces[at], pieces[at + 1])) {
            ++at;
            continue;
        }
        pieces[at].end = pieces[at + 1].end;
        for (const std::size_t cover : pieces[at + 1].covers) {
            if (std::ranges::find(pieces[at].covers, cover) == pieces[at].covers.end())
                pieces[at].covers.push_back(cover);
        }
        std::ranges::sort(pieces[at].covers);
        pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(at) + 1);
        changed = true;
    }
    return changed;
}

/// What happened to one group, to be added to the report.
struct GroupOutcome {
    std::vector<Subtitle> subtitles;
    DisjointReport report;
};

/// Takes over the first interval shorter than the limit that can be, and says
/// whether there was one.
[[nodiscard]] bool
absorbedOne(std::vector<Piece>& pieces, std::span<const Subtitle> group, Duration shortest) {
    for (std::size_t at = 0; at < pieces.size() && pieces.size() > 1; ++at) {
        const Piece& piece = pieces[at];
        if (!(piece.duration() < shortest))
            continue;

        // A subtitle shorter than the limit is shown whole: absorbing its
        // interval would drop its text.
        const bool wholeShort = std::ranges::any_of(
            piece.covers, [&](std::size_t cover) { return group[cover].duration() < shortest; });
        if (wholeShort)
            continue;

        const bool hasPrevious = at > 0;
        const bool hasNext = at + 1 < pieces.size();
        const auto holding = [&](std::size_t neighbour) { return holds(pieces[neighbour], piece); };
        std::size_t kept = 0;
        if (hasPrevious && hasNext) {
            const bool previousHolds = holding(at - 1);
            const bool nextHolds = holding(at + 1);
            const bool previousLonger = pieces[at - 1].duration() >= pieces[at + 1].duration();
            // The one that holds it; when both do or neither does, the longer.
            const bool previousWins = previousHolds != nextHolds ? previousHolds : previousLonger;
            kept = previousWins ? at - 1 : at + 1;
        } else {
            kept = hasPrevious ? at - 1 : at + 1;
        }

        pieces[kept].start = std::min(pieces[kept].start, piece.start);
        pieces[kept].end = std::max(pieces[kept].end, piece.end);
        pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(at));
        return true;
    }
    return false;
}

/// The part of `text` shown in window `which` of the lines `most` at a time.
[[nodiscard]] std::string windowOf(const std::string& text, std::size_t which, std::size_t most) {
    const std::vector<std::string> lines = linesOf(text);
    if (lines.size() <= most)
        return text;

    const std::size_t from = std::min(which, lines.size() - most);
    return joined(std::span{lines}.subspan(from, most));
}

/// Replaces one group of overlapping subtitles.
[[nodiscard]] GroupOutcome disjointedGroup(std::span<const Subtitle> group,
                                           const DisjointSettings& settings) {
    GroupOutcome outcome;
    DisjointReport& report = outcome.report;
    report.groups = 1;
    report.replaced = group.size();
    report.tooShort = static_cast<std::size_t>(std::ranges::count_if(
        group, [&](const Subtitle& subtitle) { return subtitle.duration() < settings.shortest; }));

    std::vector<Piece> pieces = piecesOf(group, settings.dashes);
    (void)joinedSame(pieces);
    while (absorbedOne(pieces, group, settings.shortest)) {
        ++report.absorbed;
        (void)joinedSame(pieces);
    }

    // How many pieces each subtitle shows in, before the windows: a subtitle in
    // several was cut. Windows cut an interval, not a subtitle.
    std::vector<std::size_t> appearances(group.size(), 0);
    for (const Piece& piece : pieces) {
        for (const std::size_t cover : piece.covers)
            ++appearances[cover];
    }
    report.split = static_cast<std::size_t>(
        std::ranges::count_if(appearances, [](std::size_t count) { return count > 1; }));

    std::vector<Piece> finals;
    for (const Piece& piece : pieces) {
        const bool several = piece.covers.size() > 1;
        const std::size_t lines =
            std::max(linesOf(piece.mainText).size(), linesOf(piece.translationText).size());
        const bool tooMany = several && settings.mostLines > 0 && lines > settings.mostLines;
        const std::size_t windows = tooMany ? lines - settings.mostLines + 1 : 1;

        if (several) {
            ++report.merged;
            const Subtitle& first = group[piece.covers.front()];
            const bool differ = std::ranges::any_of(piece.covers, [&](std::size_t cover) {
                return group[cover].extras != first.extras;
            });
            report.extrasDropped += differ ? 1U : 0U;
        }
        report.windowed += tooMany ? 1U : 0U;

        // Equal parts, the last one taking what the division leaves.
        const std::int64_t length = piece.duration().milliseconds();
        const auto cut = [&](std::size_t window) {
            return window == windows
                       ? piece.end
                       : piece.start +
                             Duration::fromMilliseconds(length * static_cast<std::int64_t>(window) /
                                                        static_cast<std::int64_t>(windows));
        };
        for (std::size_t window = 0; window < windows; ++window) {
            Piece shown = piece;
            shown.start = cut(window);
            shown.end = cut(window + 1);
            if (tooMany) {
                shown.mainText = windowOf(piece.mainText, window, settings.mostLines);
                shown.translationText = windowOf(piece.translationText, window, settings.mostLines);
            }
            finals.push_back(std::move(shown));
        }
    }

    // A window can end up showing what the interval after it shows: one subtitle.
    (void)joinedSame(finals);

    for (const Piece& piece : finals) {
        Subtitle subtitle = group[piece.covers.front()];
        subtitle.start = piece.start;
        subtitle.end = piece.end;
        subtitle.mainText = piece.mainText;
        subtitle.translationText = piece.translationText;
        outcome.subtitles.push_back(std::move(subtitle));
    }
    report.produced = outcome.subtitles.size();
    return outcome;
}

void add(DisjointReport& total, const DisjointReport& part) {
    total.groups += part.groups;
    total.replaced += part.replaced;
    total.produced += part.produced;
    total.split += part.split;
    total.merged += part.merged;
    total.windowed += part.windowed;
    total.absorbed += part.absorbed;
    total.tooShort += part.tooShort;
    total.extrasDropped += part.extrasDropped;
}

} // namespace

std::expected<DisjointRanges, DisjointRefusal> makeDisjoint(const Project& project,
                                                            const DisjointSettings& settings) {
    if (!project.isInOrder())
        return std::unexpected{DisjointRefusal::NotInOrder};

    const std::span<const Subtitle> subtitles = project.subtitles();
    DisjointRanges result;

    struct Replacement {
        std::size_t first;
        std::size_t last;
        std::vector<Subtitle> subtitles;
    };

    std::vector<Replacement> replacements;

    const auto usable = [](const Subtitle& subtitle) { return subtitle.end > subtitle.start; };

    std::size_t at = 0;
    while (at < subtitles.size()) {
        if (subtitles[at].end < subtitles[at].start)
            ++result.report.endingBeforeStart;
        if (!usable(subtitles[at])) {
            ++at;
            continue;
        }

        // The run of neighbours each of which starts before the furthest end so
        // far — the ones that overlap, directly or through another.
        std::size_t last = at;
        Timestamp furthest = subtitles[at].end;
        while (last + 1 < subtitles.size() && usable(subtitles[last + 1]) &&
               subtitles[last + 1].start < furthest) {
            ++last;
            furthest = std::max(furthest, subtitles[last].end);
        }

        if (last > at) {
            GroupOutcome outcome = disjointedGroup(subtitles.subspan(at, last - at + 1), settings);
            add(result.report, outcome.report);
            replacements.push_back(
                Replacement{.first = at, .last = last, .subtitles = std::move(outcome.subtitles)});
        }
        at = last + 1;
    }

    if (replacements.empty())
        return result;

    // Where the rows landed: each group moves the ones after it by what it
    // added or took away.
    std::ptrdiff_t shift = 0;
    SubtitleIndex firstRow = SubtitleIndex::fromValue(0);
    SubtitleIndex lastRow = SubtitleIndex::fromValue(0);
    for (std::size_t group = 0; group < replacements.size(); ++group) {
        const Replacement& replacement = replacements[group];
        const auto first =
            static_cast<std::size_t>(static_cast<std::ptrdiff_t>(replacement.first) + shift);
        if (group == 0)
            firstRow = SubtitleIndex::fromValue(first);
        lastRow = SubtitleIndex::fromValue(first + replacement.subtitles.size() - 1);
        shift += static_cast<std::ptrdiff_t>(replacement.subtitles.size()) -
                 static_cast<std::ptrdiff_t>(replacement.last - replacement.first + 1);
    }
    result.touched = IndexRange{.first = firstRow, .last = lastRow};

    // From the end, so that the indices of the groups still to come are the
    // ones they were read at.
    std::vector<std::unique_ptr<Command>> commands;
    for (Replacement& replacement : std::views::reverse(replacements)) {
        const SubtitleIndex first = SubtitleIndex::fromValue(replacement.first);
        commands.push_back(std::make_unique<RemoveCommand>(
            Selection::range(first, SubtitleIndex::fromValue(replacement.last))));
        commands.push_back(
            std::make_unique<InsertCommand>(first, std::move(replacement.subtitles)));
    }
    result.command =
        std::make_unique<CompositeCommand>(CommandKind::MakeDisjoint, std::move(commands));
    return result;
}

} // namespace subedit::core
