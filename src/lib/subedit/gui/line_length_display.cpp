#include <subedit/gui/ems_line_measure.hpp>
#include <subedit/gui/line_length_display.hpp>

#include <QFont>

#include <algorithm>
#include <memory>
#include <string_view>
#include <utility>

namespace subedit::gui {

namespace {

constexpr double kSmallerBy = 0.8;

/// A `LineMeasure` and the table that remembers its answers, owned together:
/// `CachedLineMeasure` only borrows what it wraps.
class OwnedCachedMeasure final : public core::LineMeasure {

public:
    explicit OwnedCachedMeasure(std::unique_ptr<const core::LineMeasure> underlying)
        : m_underlying(std::move(underlying)), m_cache(*m_underlying) {}

    [[nodiscard]] double lengthOf(std::string_view text) const override {
        return m_cache.lengthOf(text);
    }

private:
    std::unique_ptr<const core::LineMeasure> m_underlying;
    core::CachedLineMeasure m_cache;
};

} // namespace

std::shared_ptr<const core::LineMeasure> LengthMeasures::measureFor(core::LengthUnit unit,
                                                                    const QFont& font) {
    if (m_measure != nullptr && unit == m_unit && font == m_font)
        return m_measure;

    std::unique_ptr<const core::LineMeasure> base;
    if (unit == core::LengthUnit::Ems)
        base = std::make_unique<const EmsLineMeasure>(font);
    else
        base = std::make_unique<const core::CharacterLineMeasure>();

    m_measure = std::make_shared<const OwnedCachedMeasure>(std::move(base));
    m_unit = unit;
    m_font = font;
    return m_measure;
}

QFont smallerFont(const QFont& font) {
    QFont smaller = font;
    if (font.pixelSize() > 0)
        smaller.setPixelSize(std::max(1, static_cast<int>(font.pixelSize() * kSmallerBy)));
    else
        smaller.setPointSizeF(font.pointSizeF() * kSmallerBy);
    return smaller;
}

} // namespace subedit::gui
