#include <subedit/gui/ems_line_measure.hpp>

#include <QString>

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

} // namespace subedit::gui
