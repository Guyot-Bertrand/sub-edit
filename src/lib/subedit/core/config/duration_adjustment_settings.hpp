#pragma once

#include <subedit/core/edit/duration_adjustment.hpp>

#include <cstdint>

namespace subedit::core {

/// The form of `Adjust Durations…`, as the window keeps it from one opening to
/// the next and from one session to the next — decision of issue #409.
///
/// **Every value is always here, whether its case is checked or not.**
/// `DurationConstraints` turns an unchecked case into `std::nullopt`, which is
/// right for the core and wrong for a form: unchecking a field must not erase
/// the number it held, or checking it back on asks the user for it again.
struct DurationAdjustmentSettings {
    double charactersPerSecond = kDefaultReadingSpeed;
    bool lengthen = true;
    bool shorten = false;

    bool minimumEnabled = true;
    std::int64_t minimumMilliseconds = kDefaultMinimumMilliseconds;

    bool maximumEnabled = false;
    std::int64_t maximumMilliseconds = kDefaultMaximumMilliseconds;

    bool gapEnabled = true;
    std::int64_t gapMilliseconds = 0;

    friend bool operator==(const DurationAdjustmentSettings&,
                           const DurationAdjustmentSettings&) = default;
};

/// The request the core receives, built from what the form shows — an
/// unchecked field becomes absent, never zero by accident.
///
/// A reading speed that is not one — a settings file edited by hand can carry
/// zero — is read as the speed being off rather than refused outright: the
/// tolerance ADR 0022 already chooses for every other option.
[[nodiscard]] DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings);

} // namespace subedit::core
