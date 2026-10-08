#pragma once

#include <subedit/core/edit/translation.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>

#include <optional>
#include <span>
#include <vector>

namespace subedit::core {

/// A translation that sits a constant time away from the main document, and what
/// opening it shifted back would say — decision D10 of the phase-14 spec.
struct ConstantShift {
    /// How much later than the main document the lines are. Negative when they
    /// are early.
    Duration lateBy = Duration::zero();

    /// What opening the lines as they are says.
    TranslationOutcome asOpened{};

    /// What opening them moved by `lateBy` says. Always clean.
    TranslationOutcome ifShifted{};

    friend bool operator==(const ConstantShift&, const ConstantShift&) = default;
};

/// Looks for one constant shift that would make the opening of `lines` clean.
///
/// **The candidates are the gaps between the start of each line and the starts of the
/// subtitles on either side of it**, gathered to the ten milliseconds; a gap
/// that only one line shows is noise. Each candidate is judged by **the matching the
/// opening goes through** (`previewTranslation`, by position), so the confidence is
/// what the opening would report and never a number made up for the purpose.
///
/// **Nothing is returned** when the opening is clean already, when no candidate makes
/// it clean, and when two candidates do — in which case none has been shown to be the
/// one. A drift, or a shift that holds for only some of the lines, leaves no candidate
/// that cleans the opening, and so says nothing: it is not told apart from a shift,
/// it simply is not one.
///
/// `lateBy` is the median of the gaps of its group, so a shift laid on a file is found
/// to the millisecond whatever the rounding of the group.
[[nodiscard]] std::optional<ConstantShift> findConstantShift(const Project& project,
                                                             std::span<const Subtitle> lines);

/// `lines` moved `lateBy` earlier — the shift undone.
[[nodiscard]] std::vector<Subtitle> shiftedBack(std::span<const Subtitle> lines, Duration lateBy);

} // namespace subedit::core
