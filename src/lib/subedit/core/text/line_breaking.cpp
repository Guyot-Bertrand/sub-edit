#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/line_breaking.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/utf8.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

// --- Boxes and lines --------------------------------------------------------

/// `Liner._boxes_to_lines`: `boxes` joined into lines, cut after each box a
/// break in `breaks` names.
[[nodiscard]] std::vector<std::string> boxesToLines(std::span<const std::string> boxes,
                                                    std::span<const std::size_t> breaks) {
    std::vector<std::size_t> edges{0};
    edges.reserve(breaks.size() + 2);
    for (const std::size_t at : breaks)
        edges.push_back(at + 1);
    edges.push_back(boxes.size());

    std::vector<std::string> lines;
    lines.reserve(edges.size() - 1);
    for (std::size_t which = 0; which + 1 < edges.size(); ++which) {
        std::string line;
        for (std::size_t box = edges[which]; box < edges[which + 1]; ++box) {
            if (box > edges[which])
                line += ' ';
            line += boxes[box];
        }
        lines.push_back(std::move(line));
    }
    return lines;
}

/// `boxes`, space-joined, the one line a text makes with no break at all.
[[nodiscard]] std::string joinedOf(std::span<const std::string> boxes) {
    std::string joined;
    for (std::size_t which = 0; which < boxes.size(); ++which) {
        if (which > 0)
            joined += ' ';
        joined += boxes[which];
    }
    return joined;
}

// --- The demerit: Liner._calculate_demerit -----------------------------------

// Gaupol's own four weights, unexplained beyond "two subjective measures of
// badness" in its own comment — kept as it wrote them, since changing a
// number here changes which break a text gets, not just how it reads.
constexpr double kDeviationWeight = 50.0;
constexpr double kPyramidWeight = 50.0;
constexpr double kLineCountWeight = 100.0;
constexpr double kExcessLinesWeight = 1000.0;

[[nodiscard]] double demeritOf(std::span<const std::string> boxes,
                               std::span<const double> penalties,
                               std::span<const std::size_t> breaks,
                               double maxLength,
                               int maxLines,
                               const LineMeasure& measure) {
    const auto nlines = static_cast<double>(breaks.size() + 1);
    double chosen = 0.0;
    for (const std::size_t at : breaks)
        chosen += penalties[at];

    const std::vector<std::string> lines = boxesToLines(boxes, breaks);
    std::vector<double> lengths;
    lengths.reserve(lines.size());
    for (const std::string& line : lines)
        lengths.push_back(measure.lengthOf(line));

    double mean = 0.0;
    for (const double length : lengths)
        mean += length;
    mean /= static_cast<double>(lengths.size());

    double deviation = 0.0;
    for (const double length : lengths) {
        const double normalized = (length - mean) / maxLength;
        deviation += normalized * normalized;
    }

    double pyramid = 0.0;
    for (std::size_t which = 0; which + 1 < lengths.size(); ++which) {
        if (lengths[which] > lengths[which + 1]) {
            const double normalized = (lengths[which] - lengths[which + 1]) / maxLength;
            pyramid += normalized * normalized;
        }
    }

    const double lineCost = std::pow(nlines - 1.0, 3);
    const double overCost = std::pow(std::max(0.0, nlines - static_cast<double>(maxLines)), 3);
    return chosen + (kDeviationWeight * deviation) + (kPyramidWeight * pyramid) +
           (kLineCountWeight * lineCost) + (kExcessLinesWeight * overCost);
}

// --- The candidate breaks: Liner._list_possible_breaks, memoized ------------

/// What `possibleBreaksOf` remembers an answer by — `boxes` and `penalties`
/// read as views, `nlines` as itself.
using BreakKey = std::tuple<std::vector<std::string_view>, std::vector<double>, int>;
using BreakMemo = std::map<BreakKey, std::vector<std::size_t>>;

[[nodiscard]] std::vector<std::size_t> possibleBreaksOf(std::span<const std::string> boxes,
                                                        std::span<const double> penalties,
                                                        int nlines,
                                                        double maxLength,
                                                        const LineMeasure& measure,
                                                        BreakMemo& memo);

[[nodiscard]] std::vector<std::size_t> possibleBreaksRaw(std::span<const std::string> boxes,
                                                         std::span<const double> penalties,
                                                         int nlines,
                                                         double maxLength,
                                                         const LineMeasure& measure,
                                                         BreakMemo& memo) {
    if (nlines == 1)
        return {};

    const auto breakCount = boxes.size() - static_cast<std::size_t>(nlines - 1);
    std::vector<bool> keep(breakCount, false);

    if (nlines == 2) {
        for (std::size_t at = 0; at < breakCount; ++at) {
            const std::array<std::size_t, 1> single{at};
            const std::vector<std::string> lines = boxesToLines(boxes, single);
            double longest = 0.0;
            for (const std::string& line : lines)
                longest = std::max(longest, measure.lengthOf(line));
            keep[at] = longest <= maxLength;
        }
    } else {
        for (std::size_t at = 0; at < breakCount; ++at) {
            const std::array<std::size_t, 1> single{at};
            const std::vector<std::string> lines = boxesToLines(boxes, single);
            if (measure.lengthOf(lines.front()) > maxLength)
                break;
            const std::vector<std::size_t> later = possibleBreaksOf(boxes.subspan(at + 1),
                                                                    penalties.subspan(at + 1),
                                                                    nlines - 1,
                                                                    maxLength,
                                                                    measure,
                                                                    memo);
            keep[at] = !later.empty();
        }
    }

    std::vector<std::pair<double, std::size_t>> kept;
    for (std::size_t at = 0; at < breakCount; ++at) {
        if (keep[at])
            kept.emplace_back(penalties[at], at);
    }
    std::ranges::sort(kept);

    std::vector<std::size_t> result;
    result.reserve(kept.size());
    for (const auto& [penalty, at] : kept)
        result.push_back(at);
    return result;
}

[[nodiscard]] std::vector<std::size_t> possibleBreaksOf(std::span<const std::string> boxes,
                                                        std::span<const double> penalties,
                                                        int nlines,
                                                        double maxLength,
                                                        const LineMeasure& measure,
                                                        BreakMemo& memo) {
    BreakKey key{std::vector<std::string_view>(boxes.begin(), boxes.end()),
                 std::vector<double>(penalties.begin(), penalties.end()),
                 nlines};
    const auto found = memo.find(key);
    if (found != memo.end())
        return found->second;
    std::vector<std::size_t> result =
        possibleBreaksRaw(boxes, penalties, nlines, maxLength, measure, memo);
    return memo.emplace(std::move(key), std::move(result)).first->second;
}

// --- The search among candidates: Liner._break_lines -------------------------

struct BreakChoice {
    std::optional<std::vector<std::size_t>> breaks;
    double demerit = 0.0;
};

/// The most negative penalty that `count` later breaks could still add — a
/// bound on how much better a candidate still now in the running could get.
[[nodiscard]] double negativeSumBound(std::span<const double> penalties, int count) {
    std::vector<double> negative;
    for (const double penalty : penalties) {
        if (penalty < 0.0)
            negative.push_back(penalty);
    }
    std::ranges::sort(negative);
    const std::size_t take = std::min(negative.size(), static_cast<std::size_t>(count));
    double sum = 0.0;
    for (std::size_t which = 0; which < take; ++which)
        sum += negative[which];
    return sum;
}

/// `breakAmong`'s two-line case: the best single break among `candidates`, if
/// any beats what a caller already found without one.
[[nodiscard]] BreakChoice bestOverTwoLines(std::span<const std::string> boxes,
                                           std::span<const double> penalties,
                                           std::span<const std::size_t> candidates,
                                           double maxLength,
                                           int maxLines,
                                           const LineMeasure& measure,
                                           BreakChoice best) {
    for (const std::size_t at : candidates) {
        if (penalties[at] > best.demerit)
            break;
        const std::array<std::size_t, 1> single{at};
        const double demerit = demeritOf(boxes, penalties, single, maxLength, maxLines, measure);
        if (demerit < best.demerit)
            best = {.breaks = std::vector<std::size_t>{at}, .demerit = demerit};
    }
    return best;
}

[[nodiscard]] BreakChoice breakAmong(std::span<const std::string> boxes,
                                     std::span<const double> penalties,
                                     int nlines,
                                     double maxLength,
                                     int maxLines,
                                     const LineMeasure& measure,
                                     BreakMemo& memo) {
    const std::vector<std::size_t> candidates =
        possibleBreaksOf(boxes, penalties, nlines, maxLength, measure, memo);

    std::optional<std::vector<std::size_t>> best;
    double bestDemerit = std::numeric_limits<double>::max();

    if (measure.lengthOf(joinedOf(boxes)) <= maxLength) {
        // A single line, valid, is the standard any more-line answer must beat.
        best = std::vector<std::size_t>{};
        bestDemerit = demeritOf(boxes, penalties, {}, maxLength, maxLines, measure);
    }
    if (nlines == 1)
        return {.breaks = best, .demerit = bestDemerit};

    if (nlines == 2) {
        return bestOverTwoLines(boxes,
                                penalties,
                                candidates,
                                maxLength,
                                maxLines,
                                measure,
                                {.breaks = best, .demerit = bestDemerit});
    }

    for (const std::size_t at : candidates) {
        const double bound = negativeSumBound(penalties.subspan(at + 1), nlines - 2);
        if (penalties[at] + bound > bestDemerit)
            break;

        const BreakChoice later = breakAmong(boxes.subspan(at + 1),
                                             penalties.subspan(at + 1),
                                             nlines - 1,
                                             maxLength,
                                             maxLines,
                                             measure,
                                             memo);
        if (!later.breaks.has_value())
            continue;
        std::vector<std::size_t> candidate{at};
        for (const std::size_t x : *later.breaks)
            candidate.push_back(at + 1 + x);
        const double demerit = demeritOf(boxes, penalties, candidate, maxLength, maxLines, measure);
        if (demerit < bestDemerit) {
            best = candidate;
            bestDemerit = demerit;
        }
    }
    return {.breaks = best, .demerit = bestDemerit};
}

// --- Placing the penalties: Liner._detect_penalties ---------------------------

/// One record of kind `LineBreak`, compiled and read.
struct Penalty {
    const CorrectionPattern* pattern;
    int group;
    double value;
    std::unique_ptr<PatternMatcher> matcher;
};

[[nodiscard]] PatternFailure failureOf(FailureKind kind,
                                       const CorrectionPattern& pattern,
                                       std::optional<std::size_t> text,
                                       std::string detail) {
    return PatternFailure{.kind = kind,
                          .code = pattern.code,
                          .rank = pattern.rank,
                          .name = pattern.name,
                          .text = text,
                          .detail = std::move(detail)};
}

/// Adds `value` at the start of `groups[group]`, and does nothing where
/// `group` is out of range or took no part in the match — an unreachable
/// case for a shipped record, but not one a hand-written one is refused for.
void addPenaltyIfCaptured(std::span<const std::optional<MatchSpan>> groups,
                          std::size_t group,
                          double value,
                          std::vector<double>& found) {
    if (group >= groups.size())
        return;
    const std::optional<MatchSpan>& span = groups[group];
    if (!span.has_value())
        return;
    found[span->start] += value;
}

/// One pattern's own scan of `joined`, the penalty it places at each match's
/// chosen group — or the failure that stopped it partway.
[[nodiscard]] std::expected<std::vector<double>, SearchFailure>
scanPenalty(const Penalty& one, std::string_view joined) {
    // One slot past the end: a pattern that matches nothing can still match
    // the empty string right there, and a chosen group can start on it.
    std::vector<double> found(joined.size() + 1, 0.0);
    std::size_t pos = 0;
    std::optional<MatchSpan> previous;

    while (true) {
        const std::expected<std::optional<Match>, SearchFailure> searched =
            one.matcher->find(joined, pos);
        if (!searched)
            return std::unexpected{searched.error()};
        if (!searched->has_value())
            return found;

        const Match& match = **searched;
        const MatchSpan whole = match.whole();
        if (previous == whole && whole.start == pos && whole.end == pos) {
            if (pos >= joined.size())
                return found;
            pos = nextCodePoint(joined, pos);
            continue;
        }

        addPenaltyIfCaptured(match.groups, static_cast<std::size_t>(one.group), one.value, found);

        pos = whole.end;
        previous = whole;
        if (whole.start == whole.end)
            previous = MatchSpan{.start = pos, .end = pos};
    }
}

/// The penalty at the space between each pair of boxes — zero where no
/// pattern's chosen group started.
///
/// **A pattern that cannot be searched to completion contributes nothing for
/// this text**, rather than the partial score a half-finished scan would
/// leave: the others still place theirs.
[[nodiscard]] std::vector<double> detectPenalties(std::span<const Penalty> penalties,
                                                  std::span<const std::string> boxes,
                                                  std::vector<PatternFailure>& failures,
                                                  std::size_t textIndex) {
    const std::string joined = joinedOf(boxes);
    std::vector<double> atSpace(joined.size(), 0.0);

    for (const Penalty& one : penalties) {
        const std::expected<std::vector<double>, SearchFailure> scanned = scanPenalty(one, joined);
        if (!scanned) {
            failures.push_back(failureOf(
                FailureKind::TimedOut, *one.pattern, textIndex, "the search was given up"));
            continue;
        }
        for (std::size_t at = 0; at < joined.size(); ++at)
            atSpace[at] += (*scanned)[at];
    }

    std::vector<double> result(boxes.size(), 0.0);
    std::size_t boxStart = 0;
    for (std::size_t at = 0; at + 1 < boxes.size(); ++at) {
        const std::size_t spaceOffset = boxStart + boxes[at].size();
        result[at] = atSpace[spaceOffset];
        boxStart = spaceOffset + 1;
    }
    return result;
}

// --- Normalizing a text into boxes: Liner.break_lines's setup ----------------

constexpr std::string_view kPythonWhitespace = " \t\n\r\v\f";

[[nodiscard]] std::string_view stripped(std::string_view text) {
    const std::size_t begin = text.find_first_not_of(kPythonWhitespace);
    if (begin == std::string_view::npos)
        return {};
    const std::size_t end = text.find_last_not_of(kPythonWhitespace);
    return text.substr(begin, end - begin + 1);
}

/// `text.strip()`, `\n` folded to a space, runs of two or more spaces
/// collapsed to one — `Liner.break_lines`'s own `" {2,}"` pass, done directly
/// rather than through the pattern engine: the operation is fixed and
/// unambiguous, no Gaupol-authored expression to stay faithful to.
[[nodiscard]] std::string normalized(std::string_view text) {
    std::string out{stripped(text)};
    for (char& c : out) {
        if (c == '\n')
            c = ' ';
    }
    std::string collapsed;
    collapsed.reserve(out.size());
    bool inSpaces = false;
    for (const char c : out) {
        if (c == ' ') {
            if (!inSpaces)
                collapsed += ' ';
            inSpaces = true;
        } else {
            collapsed += c;
            inSpaces = false;
        }
    }
    return collapsed;
}

[[nodiscard]] std::vector<std::string> boxesOf(std::string_view text) {
    std::vector<std::string> boxes;
    std::size_t start = 0;
    while (true) {
        const std::size_t space = text.find(' ', start);
        if (space == std::string_view::npos) {
            boxes.emplace_back(text.substr(start));
            break;
        }
        boxes.emplace_back(text.substr(start, space - start));
        start = space + 1;
    }
    return boxes;
}

/// `text.split("\n")` — Python keeps an empty piece between two consecutive
/// separators, and this does too.
[[nodiscard]] std::vector<std::string_view> splitOnNewline(std::string_view text) {
    std::vector<std::string_view> lines;
    std::size_t start = 0;
    while (true) {
        const std::size_t at = text.find('\n', start);
        if (at == std::string_view::npos) {
            lines.push_back(text.substr(start));
            break;
        }
        lines.push_back(text.substr(start, at - start));
        start = at + 1;
    }
    return lines;
}

// --- One text: Liner.break_lines ----------------------------------------------

[[nodiscard]] std::string breakOneText(std::span<const Penalty> penalties,
                                       std::string_view text,
                                       const LineMeasure& measure,
                                       double maxLength,
                                       int maxLines,
                                       std::vector<PatternFailure>& failures,
                                       std::size_t textIndex) {
    std::string base = normalized(text);
    const std::vector<std::string> boxes = boxesOf(base);
    if (boxes.size() == 1)
        return base;

    const std::vector<double> boxPenalties = detectPenalties(penalties, boxes, failures, textIndex);

    BreakMemo memo;
    std::optional<std::vector<std::size_t>> best;
    double bestDemerit = std::numeric_limits<double>::max();

    const int minLines = std::min(2, maxLines);
    const auto maxTried = static_cast<int>(std::min<std::size_t>(10, boxes.size()));
    for (int nlines = minLines; nlines <= maxTried; ++nlines) {
        const BreakChoice choice =
            breakAmong(boxes, boxPenalties, nlines, maxLength, maxLines, measure, memo);
        if (!choice.breaks.has_value())
            continue;
        if (choice.demerit < bestDemerit) {
            best = choice.breaks;
            bestDemerit = choice.demerit;
        }
        if (nlines < maxLines)
            continue;

        std::string out = base;
        std::size_t boxStart = 0;
        for (std::size_t at = 0; at + 1 < boxes.size(); ++at) {
            const std::size_t spaceOffset = boxStart + boxes[at].size();
            if (best.has_value() && std::ranges::find(*best, at) != best->end())
                out[spaceOffset] = '\n';
            boxStart = spaceOffset + 1;
        }
        return out;
    }
    return base;
}

[[nodiscard]] std::vector<Penalty>
preparePenalties(const PatternEngine& engine,
                 std::span<const CorrectionPattern* const> patterns,
                 std::vector<PatternFailure>& failures) {
    std::vector<Penalty> prepared;
    for (const CorrectionPattern* pattern : patterns) {
        const auto* fields = std::get_if<LineBreakFields>(&pattern->fields);
        if (fields == nullptr)
            continue;

        std::expected<std::unique_ptr<PatternMatcher>, CompileError> matcher =
            engine.compile(pattern->expression, pattern->flags);
        if (!matcher) {
            failures.push_back(failureOf(matcher.error().kind == CompileFailure::Untranslatable
                                             ? FailureKind::Untranslatable
                                             : FailureKind::CompileError,
                                         *pattern,
                                         std::nullopt,
                                         matcher.error().reason));
            continue;
        }
        prepared.push_back(Penalty{.pattern = pattern,
                                   .group = fields->group,
                                   .value = fields->penalty,
                                   .matcher = std::move(*matcher)});
    }
    return prepared;
}

/// The longest line of `text`, split on `\n` — Gaupol's own reading of a
/// subtitle's shape, before or after a break.
[[nodiscard]] double longestLineOf(std::string_view text, const LineMeasure& measure) {
    double longest = 0.0;
    for (const std::string_view line : splitOnNewline(text))
        longest = std::max(longest, measure.lengthOf(line));
    return longest;
}

[[nodiscard]] int lineCountOf(std::string_view text) {
    return static_cast<int>(splitOnNewline(text).size());
}

} // namespace

BrokenTexts breakLines(const PatternEngine& engine,
                       std::span<const CorrectionPattern* const> patterns,
                       std::span<const std::string> texts,
                       const LineMeasure& measure,
                       double maxLength,
                       int maxLines,
                       bool skip) {
    BrokenTexts result;
    const std::vector<Penalty> prepared = preparePenalties(engine, patterns, result.failures);

    result.texts.reserve(texts.size());
    for (std::size_t index = 0; index < texts.size(); ++index) {
        const std::string& original = texts[index];

        if (skip) {
            const double length = longestLineOf(original, measure);
            const int lineCount = lineCountOf(original);
            if (length <= maxLength && lineCount <= maxLines) {
                result.texts.push_back(original);
                continue;
            }

            std::string broken = breakOneText(
                prepared, original, measure, maxLength, maxLines, result.failures, index);

            const bool lengthFixed = length > maxLength && longestLineOf(broken, measure) < length;
            const bool linesFixed = lineCount > maxLines && lineCountOf(broken) < lineCount;
            if (!lengthFixed && !linesFixed) {
                result.texts.push_back(original);
                continue;
            }
            result.texts.push_back(std::move(broken));
            continue;
        }

        result.texts.push_back(
            breakOneText(prepared, original, measure, maxLength, maxLines, result.failures, index));
    }
    return result;
}

} // namespace subedit::core
