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

    /// What the person must not take for granted of this result, shown under the summary: the
    /// limits of laying the file on the speech, and a frame rate put right. Empty for nothing.
    std::string warning;
};

} // namespace subedit::gui
