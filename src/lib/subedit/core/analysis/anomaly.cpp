#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace subedit::core {

std::vector<Anomaly> scanAnomalies(const Project& project) {
    std::vector<Anomaly> found;

    const std::span<const Subtitle> subtitles = project.subtitles();
    for (std::size_t position = 0; position < subtitles.size(); ++position) {
        const SubtitleIndex index = SubtitleIndex::fromValue(position);
        const Subtitle& subtitle = subtitles[position];

        // The only kind that needs no predecessor, so the only one the first
        // subtitle can carry.
        if (subtitle.end < subtitle.start)
            found.push_back(Anomaly{.kind = AnomalyKind::EndBeforeStart, .index = index});

        if (position == 0)
            continue;

        const Subtitle& previous = subtitles[position - 1];

        if (subtitle.start < previous.end)
            found.push_back(Anomaly{.kind = AnomalyKind::OverlappingSubtitles, .index = index});

        if (subtitle.start < previous.start)
            found.push_back(Anomaly{.kind = AnomalyKind::OutOfOrder, .index = index});
    }

    return found;
}

std::vector<AnomalyCount> countAnomalies(std::span<const Anomaly> anomalies) {
    constexpr std::array kAllKinds{
        AnomalyKind::EndBeforeStart, AnomalyKind::OverlappingSubtitles, AnomalyKind::OutOfOrder};

    std::vector<AnomalyCount> counts;
    for (const AnomalyKind kind : kAllKinds) {
        std::size_t count = 0;
        SubtitleIndex first = SubtitleIndex::fromValue(0);
        for (const Anomaly& anomaly : anomalies) {
            if (anomaly.kind != kind)
                continue;
            if (count == 0)
                first = anomaly.index;
            ++count;
        }
        if (count != 0)
            counts.push_back(AnomalyCount{.kind = kind, .count = count, .first = first});
    }
    return counts;
}

} // namespace subedit::core
