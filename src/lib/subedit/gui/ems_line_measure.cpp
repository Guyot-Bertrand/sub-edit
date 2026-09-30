#include <subedit/gui/ems_line_measure.hpp>

#include <QString>

#include <memory>
#include <string_view>

namespace subedit::gui {

namespace {

// `gaupol/ruler.py`'s own calibration: the natural width of the lowercase
// alphabet, rendered once, stands for 0,55 em per letter — an em that is not
// the typographic one, and never claims to be.
constexpr std::string_view kCalibrationAlphabet = "abcdefghijklmnopqrstuvwxyz";
constexpr double kAverageEmPerLetter = 0.55;

[[nodiscard]] QString toQString(std::string_view text) {
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

} // namespace

EmsLineMeasure::EmsLineMeasure(const QFont& font)
    : m_metrics(font),
      m_emLength(m_metrics.horizontalAdvance(toQString(kCalibrationAlphabet)) /
                 (kAverageEmPerLetter * static_cast<double>(kCalibrationAlphabet.size()))) {}

double EmsLineMeasure::lengthOf(std::string_view text) const {
    return m_metrics.horizontalAdvance(toQString(text)) / m_emLength;
}

CachedEmsLineMeasure::CachedEmsLineMeasure(const QFont& font) : m_ems(font), m_cached(m_ems) {}

std::shared_ptr<const core::LineMeasure> assistantLineMeasure(bool inEms, const QFont& font) {
    if (inEms)
        return std::make_shared<CachedEmsLineMeasure>(font);
    return std::make_shared<core::CharacterLineMeasure>();
}

} // namespace subedit::gui
