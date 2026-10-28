#include <subedit/core/transcription/alignment_doubt.hpp>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::size_t kDriftFraction = 10;

/// The lower of the two middle values for an even count, so that one outlier among few neighbours
/// cannot pull the median onto itself.
[[nodiscard]] std::int64_t medianOf(std::vector<std::int64_t> values) {
    if (values.empty())
        return 0;
    std::ranges::sort(values);
    return values[(values.size() - 1) / 2];
}

/// The median shift of the positioned lines around `line`, itself left out; nothing when none has.
[[nodiscard]] std::optional<std::int64_t> neighboursShift(const std::vector<LineVerdict>& lines,
                                                          std::size_t line) {
    const std::size_t from = line > kDoubtNeighbours ? line - kDoubtNeighbours : 0;
    const std::size_t to = std::min(lines.size(), line + kDoubtNeighbours + 1);
    std::vector<std::int64_t> around;
    for (std::size_t at = from; at < to; ++at)
        if (at != line && lines[at].shiftMilliseconds.has_value())
            around.push_back(lines[at].shiftMilliseconds.value_or(0));
    if (around.empty())
        return std::nullopt;
    return medianOf(std::move(around));
}

[[nodiscard]] LineVerdict firstLook(const LineMatch& match,
                                    const std::optional<Timestamp>& original,
                                    const DoubtLimits& limits) {
    if (!match.place.start.has_value() || !original.has_value())
        return {.doubt = LineDoubt::NotFound, .shiftMilliseconds = std::nullopt};
    const Timestamp from = original.value_or(Timestamp::fromMilliseconds(0));
    const std::int64_t shift =
        match.place.start.value_or(from).milliseconds() - from.milliseconds();
    const bool heardEnough = static_cast<double>(match.matched) >=
                             limits.partialBelow * static_cast<double>(match.words);
    return {.doubt = heardEnough ? LineDoubt::None : LineDoubt::Partial,
            .shiftMilliseconds = shift};
}

void count(AlignmentReport& report) {
    for (const LineVerdict& verdict : report.lines) {
        switch (verdict.doubt) {
        case LineDoubt::None:
            ++report.sure;
            break;
        case LineDoubt::NotFound:
            ++report.notFound;
            break;
        case LineDoubt::Partial:
            ++report.partial;
            break;
        case LineDoubt::OutOfLine:
            ++report.outOfLine;
            break;
        }
    }
}

void measureShifts(AlignmentReport& report) {
    std::vector<std::int64_t> shifts;
    for (const LineVerdict& verdict : report.lines)
        if (verdict.shiftMilliseconds.has_value())
            shifts.push_back(verdict.shiftMilliseconds.value_or(0));
    report.medianShiftMilliseconds = medianOf(shifts);
    if (shifts.size() < 2)
        return;
    const std::size_t tenth = std::max<std::size_t>(1, shifts.size() / kDriftFraction);
    const std::vector<std::int64_t> first(shifts.begin(),
                                          shifts.begin() + static_cast<long>(tenth));
    const std::vector<std::int64_t> last(shifts.end() - static_cast<long>(tenth), shifts.end());
    report.driftMilliseconds = medianOf(last) - medianOf(first);
}

} // namespace

AlignmentReport judgeAlignment(std::span<const LineMatch> matches,
                               std::span<const Timestamp> originalStarts,
                               const DoubtLimits& limits) {
    AlignmentReport report;
    report.lines.reserve(matches.size());
    for (std::size_t line = 0; line < matches.size(); ++line) {
        const std::optional<Timestamp> original =
            line < originalStarts.size() ? std::optional{originalStarts[line]} : std::nullopt;
        report.lines.push_back(firstLook(matches[line], original, limits));
    }

    // The neighbours are judged on the shifts as found, before any line is called out of line.
    const std::vector<LineVerdict> found = report.lines;
    for (std::size_t line = 0; line < found.size(); ++line) {
        if (found[line].doubt == LineDoubt::NotFound)
            continue;
        const std::optional<std::int64_t> around = neighboursShift(found, line);
        const std::int64_t gap =
            std::abs(found[line].shiftMilliseconds.value_or(0) - around.value_or(0));
        if (found[line].doubt == LineDoubt::None && around.has_value() &&
            gap > limits.gapMilliseconds)
            report.lines[line].doubt = LineDoubt::OutOfLine;
    }

    count(report);
    measureShifts(report);
    return report;
}

} // namespace subedit::core
