#pragma once

#include <subedit/core/text/text_diff.hpp>

#include <QString>

#include <vector>

namespace subedit::gui {

/// `spans` as rich text, the changed runs in bold — a rendering of `TextDiff`,
/// never a computation of one: `core::diffTexts` decides what changed, this
/// only decides how to show it.
[[nodiscard]] QString correctionDiffHtml(const std::vector<core::DiffSpan>& spans);

} // namespace subedit::gui
