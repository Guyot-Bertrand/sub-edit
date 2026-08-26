#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/video_bounds.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <optional>
#include <utility>

namespace subedit::core {

std::optional<BeyondEnd>
beyondEnd(const Project& project, const Selection& selection, std::optional<Duration> length) {
    if (!length.has_value() || length->milliseconds() <= 0)
        return std::nullopt;

    const Timestamp end = Timestamp::origin() + *length;

    BeyondEnd beyond;
    for (const SubtitleIndex index : selection.indices()) {
        const Timestamp last = project.subtitleAt(index).end;
        if (last <= end)
            continue;

        ++beyond.count;
        beyond.overshoot = std::max(beyond.overshoot, last - end);
    }

    if (beyond.count == 0)
        return std::nullopt;
    return beyond;
}

bool movesPositions(CommandKind kind) {
    switch (kind) {
    case CommandKind::Shift:
    case CommandKind::Transform:
    case CommandKind::ConvertFrameRate:
    case CommandKind::Snap:
        return true;
    case CommandKind::SetText:
    case CommandKind::SetStart:
    case CommandKind::SetEnd:
    case CommandKind::Insert:
    case CommandKind::Remove:
    case CommandKind::Sort:
    case CommandKind::RemoveHearingImpaired:
        return false;
    }

    std::unreachable();
}

} // namespace subedit::core
