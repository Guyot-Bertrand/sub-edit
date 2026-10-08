#include <subedit/core/edit/translation_drift.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <utility>

namespace subedit::core {

namespace {

/// Gaps are gathered to this many milliseconds.
constexpr std::int64_t kGroupMilliseconds = 10;

/// A gap shown by fewer lines than this is noise.
constexpr std::size_t kSmallestSupport = 2;

/// How many groups are judged, the best supported first. Each judgement is a whole
/// matching, and a file that needs more than this to find its shift has none.
constexpr std::size_t kMostCandidates = 8;

/// A shift is only ever looked for among two lines or more.
constexpr std::size_t kFewestLines = 2;

[[nodiscard]] std::int64_t groupOf(std::int64_t gap) {
    return std::llround(static_cast<double>(gap) / static_cast<double>(kGroupMilliseconds));
}

[[nodiscard]] std::int64_t medianOf(std::vector<std::int64_t> gaps) {
    std::ranges::sort(gaps);
    return gaps[gaps.size() / 2];
}

} // namespace

std::vector<Subtitle> shiftedBack(std::span<const Subtitle> lines, Duration lateBy) {
    std::vector<Subtitle> moved{lines.begin(), lines.end()};
    for (Subtitle& line : moved) {
        line.start -= lateBy;
        line.end -= lateBy;
    }
    return moved;
}

std::optional<ConstantShift> findConstantShift(const Project& project,
                                               std::span<const Subtitle> lines) {
    const std::span<const Subtitle> subtitles = project.subtitles();
    if (subtitles.empty() || lines.size() < kFewestLines)
        return std::nullopt;

    const TranslationOutcome asOpened =
        previewTranslation(project, lines, TranslationMethod::Position);
    if (asOpened.isClean())
        return std::nullopt;

    std::vector<std::int64_t> starts;
    starts.reserve(subtitles.size());
    for (const Subtitle& subtitle : subtitles)
        starts.push_back(subtitle.start.milliseconds());
    std::ranges::sort(starts);

    // The gaps to the subtitle starting at or before a line and to the one starting
    // after it. Only the nearest one could be wrong where two subtitles are as near.
    std::map<std::int64_t, std::vector<std::int64_t>> groups;
    for (const Subtitle& line : lines) {
        const std::int64_t at = line.start.milliseconds();
        const auto after = std::ranges::upper_bound(starts, at);
        if (after != starts.begin())
            groups[groupOf(at - *std::prev(after))].push_back(at - *std::prev(after));
        if (after != starts.end())
            groups[groupOf(at - *after)].push_back(at - *after);
    }

    // The zero group is the opening as it is.
    groups.erase(0);

    std::vector<std::vector<std::int64_t>> candidates;
    for (auto& entry : groups)
        if (entry.second.size() >= kSmallestSupport)
            candidates.push_back(std::move(entry.second));
    std::ranges::stable_sort(
        candidates, [](const auto& left, const auto& right) { return left.size() > right.size(); });

    std::optional<ConstantShift> found;
    const std::size_t judged = std::min(candidates.size(), kMostCandidates);
    for (std::size_t rank = 0; rank < judged; ++rank) {
        const Duration lateBy = Duration::fromMilliseconds(medianOf(candidates[rank]));
        const TranslationOutcome outcome =
            previewTranslation(project, shiftedBack(lines, lateBy), TranslationMethod::Position);
        if (!outcome.isClean())
            continue;

        // Two shifts that both clean the opening: none is the one.
        if (found.has_value())
            return std::nullopt;
        found = ConstantShift{.lateBy = lateBy, .asOpened = asOpened, .ifShifted = outcome};
    }
    return found;
}

} // namespace subedit::core
