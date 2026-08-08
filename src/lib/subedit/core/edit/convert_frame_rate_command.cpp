#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <cstddef>
#include <span>
#include <utility>

namespace subedit::core {

ConvertFrameRateCommand::ConvertFrameRateCommand(const Project& project,
                                                 Selection selection,
                                                 FrameRate input,
                                                 FrameRate output)
    : m_selection(std::move(selection)),
      m_factor(input.conversionTo(output)),
      m_output(output),
      m_previousFrameRate(project.frameRate()) {
    m_previous.reserve(m_selection.count());
    for (const SubtitleIndex index : m_selection.indices()) {
        const Subtitle& subtitle = project.subtitleAt(index);
        m_previous.push_back(Positions{.start = subtitle.start, .end = subtitle.end});
    }
}

void ConvertFrameRateCommand::apply(Project& project) {
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start = subtitle.start.scaledBy(m_factor);
        subtitle.end = subtitle.end.scaledBy(m_factor);
    }

    project.setFrameRate(m_output);
}

void ConvertFrameRateCommand::revert(Project& project) {
    const std::span<const SubtitleIndex> indices = m_selection.indices();
    for (std::size_t rank = 0; rank < indices.size(); ++rank) {
        Subtitle& subtitle = project.subtitleAt(indices[rank]);
        subtitle.start = m_previous[rank].start;
        subtitle.end = m_previous[rank].end;
    }

    project.setFrameRate(m_previousFrameRate);
}

} // namespace subedit::core
