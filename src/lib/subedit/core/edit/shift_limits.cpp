#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <optional>

namespace subedit::core {

std::optional<SubtitleIndex>
firstBeforeOrigin(const Project& project, const Selection& selection, Duration by) {
    for (const SubtitleIndex index : selection.indices()) {
        if ((project.subtitleAt(index).start + by).milliseconds() < 0)
            return index;
    }

    return std::nullopt;
}

} // namespace subedit::core
