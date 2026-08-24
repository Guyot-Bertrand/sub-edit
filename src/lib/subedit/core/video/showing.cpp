#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/showing.hpp>

#include <cstddef>
#include <optional>
#include <span>

namespace subedit::core {

std::optional<SubtitleIndex> showingAt(const Project& project, Timestamp when) {
    const std::span<const Subtitle> subtitles = project.subtitles();

    std::optional<SubtitleIndex> showing;
    for (std::size_t index = 0; index < subtitles.size(); ++index) {
        const Subtitle& subtitle = subtitles[index];
        if (subtitle.start <= when && when <= subtitle.end)
            showing = SubtitleIndex::fromValue(index);
    }

    return showing;
}

} // namespace subedit::core
