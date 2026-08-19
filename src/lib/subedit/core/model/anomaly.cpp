#include <subedit/core/model/anomaly.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>

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

} // namespace subedit::core
