#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/ratio.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// How long a blank subtitle lasts when nothing follows it to bound it.
constexpr Duration kDefaultBlankDuration = Duration::fromMilliseconds(3000);

/// Where blank subtitles inserted before `index` begin.
///
/// The end of the subtitle they follow, or the origin at the head of a file.
/// A file whose first subtitle already sits before the origin leaves no room
/// in front of it, so the room is taken from further back instead — which is
/// what Gaupol does, and the only alternative to overlapping.
[[nodiscard]] Timestamp firstStartFor(const Project& project, SubtitleIndex index) {
    if (project.count() == 0)
        return Timestamp::origin();

    if (index.value() > 0)
        return project.subtitles()[index.value() - 1].end;

    const Timestamp first = project.subtitles()[0].start;
    return first < Timestamp::origin() ? first - kDefaultBlankDuration : Timestamp::origin();
}

/// How long each of `count` blank subtitles lasts, starting at `firstStart`.
///
/// They share the window before the subtitle at `index` in equal parts. With
/// nothing after them there is no window, and they take the default duration.
///
/// A window can be negative, when the file overlaps itself at that point:
/// there is no room to share, and inventing one would push the new subtitles
/// over their neighbour. They get no duration at all, which the user sees.
[[nodiscard]] Duration blankDurationFor(const Project& project,
                                        SubtitleIndex index,
                                        std::size_t count,
                                        Timestamp firstStart) {
    if (index.value() >= project.count())
        return kDefaultBlankDuration;

    const Duration window = project.subtitles()[index.value()].start - firstStart;
    if (window <= Duration::zero())
        return Duration::zero();

    // Through `Ratio` rather than by dividing here: sharing a window is a
    // rounding decision, and the project takes them in one place.
    const std::optional<Ratio> share = Ratio::create(1, static_cast<std::int64_t>(count));
    if (!share.has_value())
        return kDefaultBlankDuration;

    return window.scaledBy(*share);
}

} // namespace

InsertCommand::InsertCommand(SubtitleIndex index, std::vector<Subtitle> subtitles)
    : m_index(index), m_subtitles(std::move(subtitles)) {}

InsertCommand InsertCommand::blank(const Project& project, SubtitleIndex index, std::size_t count) {
    const Timestamp firstStart = firstStartFor(project, index);
    const Duration duration = blankDurationFor(project, index, count, firstStart);

    std::vector<Subtitle> subtitles;
    subtitles.reserve(count);
    for (std::size_t rank = 0; rank < count; ++rank) {
        const Timestamp start = firstStart + (duration * static_cast<std::int64_t>(rank));
        subtitles.push_back(Subtitle{.start = start, .end = start + duration});
    }

    return InsertCommand{index, std::move(subtitles)};
}

void InsertCommand::apply(Project& project) {
    project.insert(m_index, m_subtitles);
}

void InsertCommand::revert(Project& project) {
    if (m_subtitles.empty())
        return;

    const SubtitleIndex last = SubtitleIndex::fromValue(m_index.value() + m_subtitles.size() - 1);
    static_cast<void>(project.remove(Selection::range(m_index, last)));
}

std::vector<Change> InsertCommand::describe() const {
    std::vector<SubtitleIndex> indices;
    indices.reserve(m_subtitles.size());
    for (std::size_t rank = 0; rank < m_subtitles.size(); ++rank)
        indices.push_back(SubtitleIndex::fromValue(m_index.value() + rank));

    return {Change{.kind = ChangeKind::Insertion, .indices = std::move(indices)}};
}

} // namespace subedit::core
