#pragma once

#include <subedit/core/transcription/synchronization.hpp>

#include <string>
#include <vector>

namespace subedit::gui {

/// What the preview shows of a synchronization.
struct SynchronizationPreview {
    /// The texts of the subtitles that were looked for, in the order of the plan.
    std::vector<std::string> texts;

    core::SynchronizationPlan plan;
};

} // namespace subedit::gui
