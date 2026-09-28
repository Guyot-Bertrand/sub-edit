#pragma once

// The em-based `LineMeasure` — decision D5 of the spec of phase 12, issue
// #503.
//
// Gaupol's own unit, calibrated the way `gaupol/ruler.py` calibrates it: not
// the em of typography, but one where the lowercase alphabet averages 0,55 em
// a letter, rendered under whatever font the caller gives.

#include <subedit/core/text/line_measure.hpp>

#include <QFont>
#include <QFontMetricsF>

namespace subedit::gui {

/// `LineMeasure` in ems, on a font read once at construction.
///
/// **Injected with its font, never `QApplication::font()` on its own** — the
/// same rule D5 gives `CachedLineMeasure`: a caller passes the application's
/// current font when that is what it means, and a test passes DejaVu Sans, so
/// a length never depends on which font happened to be current when this was
/// built.
class EmsLineMeasure final : public core::LineMeasure {

public:
    explicit EmsLineMeasure(const QFont& font);

    /// Tags already stripped, `\n` already folded — the same contract every
    /// `LineMeasure` keeps.
    [[nodiscard]] double lengthOf(std::string_view text) const override;

private:
    QFontMetricsF m_metrics;
    double m_emLength;
};

} // namespace subedit::gui
