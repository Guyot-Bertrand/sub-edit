#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <utility>

namespace subedit::core {

namespace {

/// The position of the frame `moment` falls in, at `rate`.
///
/// Idempotent by construction: a position already on the grid falls in the
/// frame it starts, and comes back as itself.
[[nodiscard]] Timestamp onGrid(Timestamp moment, FrameRate rate) {
    return Timestamp::fromFrame(moment.toFrame(rate), rate);
}

} // namespace

SnapCommand::SnapCommand(const Project& project, Selection selection, FrameRate rate)
    : m_selection(std::move(selection)), m_rate(rate), m_previousFrameRate(project.frameRate()) {
    m_previous.reserve(m_selection.count());
    for (const SubtitleIndex index : m_selection.indices()) {
        const Subtitle& subtitle = project.subtitleAt(index);
        m_previous.push_back(Positions{.start = subtitle.start, .end = subtitle.end});
    }
}

void SnapCommand::apply(Project& project) {
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start = onGrid(subtitle.start, m_rate);
        subtitle.end = onGrid(subtitle.end, m_rate);
    }

    project.setFrameRate(m_rate);
}

void SnapCommand::revert(Project& project) {
    std::size_t rank = 0;
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start = m_previous[rank].start;
        subtitle.end = m_previous[rank].end;
        ++rank;
    }

    project.setFrameRate(m_previousFrameRate);
}

} // namespace subedit::core
