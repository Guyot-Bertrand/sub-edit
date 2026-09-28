#pragma once

// The hearing-impaired mentions the phase 4 scan does not reach — decision D7
// of the spec of phase 12: the four patterns of song lyrics and speaker names
// that the pattern engine plays, followed by Gaupol's seven clean-up passes.
//
// **The scan of ADR 0017 still owns brackets and parentheses**, unchanged: it
// runs first, on its own rule. What follows is new — a text the scan already
// shortened is corrected again by whichever of the four patterns the cascade
// gives, in the cascade's order, and only a text the cascade itself changed
// earns the clean-up: the scan's own output already keeps exactly one space
// where a mention stood, and owes nothing to Gaupol's global rule.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

/// The result of correcting hearing-impaired mentions: one answer per text,
/// `nullopt` where the subtitle does not survive — the scan's own removal, or
/// a text the cascade and the clean-up left empty.
struct HearingImpairedCorrection {
    std::vector<std::optional<std::string>> texts;
    std::vector<PatternFailure> failures;
};

/// Removes the hearing-impaired mentions of `texts`, in the order the
/// decision fixes: the scan, then the patterns of kind `HearingImpaired`
/// among `patterns` — other kinds are not this function's — each once, not
/// repeated, then the seven clean-ups if any of them changed the text.
///
/// **`Sound in brackets` and `Sound in parentheses` are the scan's alone**:
/// giving them here would run their expression a second time, on text the
/// scan already resolved, so they are skipped rather than compiled.
///
/// **`scanBracketsAndParentheses` is the assistant's, not the scan's own
/// rule** — decision D7, precised by #504: the direct command of phase 4
/// always wants the scan, and always gets it through `withoutHearingImpaired`
/// directly; this function is the assistant's mentions task alone, where the
/// two checkboxes named above may be unchecked like the other four.
[[nodiscard]] HearingImpairedCorrection
correctHearingImpaired(const PatternEngine& engine,
                       std::span<const CorrectionPattern* const> patterns,
                       std::span<const std::string> texts,
                       SubtitleFormat format,
                       bool scanBracketsAndParentheses);

} // namespace subedit::core
