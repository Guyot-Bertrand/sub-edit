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

#include <cstddef>
#include <memory>
#include <string_view>

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

/// `EmsLineMeasure` with the cache D5 asked for, both owned together — issue
/// #527.
///
/// **One per operation**, never shared: the table lives as long as the object
/// does, and a `CachedLineMeasure` is not safe to ask from two threads. The
/// assistant builds one for each computation and the background thread that
/// runs it is the only one to ask; the window's own measure for the table
/// (`LengthMeasures`) is another object, on the interface thread.
class CachedEmsLineMeasure final : public core::LineMeasure {

public:
    explicit CachedEmsLineMeasure(const QFont& font);

    [[nodiscard]] double lengthOf(std::string_view text) const override {
        return m_cached.lengthOf(text);
    }

    [[nodiscard]] std::size_t cachedCount() const { return m_cached.cachedCount(); }

private:
    EmsLineMeasure m_ems;
    core::CachedLineMeasure m_cached;
};

/// The measure the correction assistant breaks lines with: ems under `font`,
/// cached, or characters — which are cheap enough, next to counting code
/// points, that a table would cost more than it saves (the bench of #502).
[[nodiscard]] std::shared_ptr<const core::LineMeasure> assistantLineMeasure(bool inEms,
                                                                            const QFont& font);

} // namespace subedit::gui
