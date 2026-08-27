#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/frame_rate_box.hpp>

#include <QString>
#include <QVariant>

namespace subedit::gui {

namespace {

/// The rate an entry carries.
///
/// The enumeration travels as an `int` because that is what `QVariant` holds
/// without a registered type, and it comes back the same way.
[[nodiscard]] core::FrameRate rateOf(const QVariant& data) {
    return core::FrameRate{static_cast<core::StandardFrameRate>(data.toInt())};
}

} // namespace

FrameRateBox::FrameRateBox(QWidget* parent) : QComboBox(parent) {
    for (const core::StandardFrameRate standard : core::kStandardFrameRates) {
        addItem(QString::fromStdString(core::nameOf(core::FrameRate{standard})),
                QVariant::fromValue(static_cast<int>(standard)));
    }
}

core::FrameRate FrameRateBox::rate() const {
    return rateOf(currentData());
}

void FrameRateBox::pick(core::FrameRate wanted) {
    for (int row = 0; row < count(); ++row) {
        if (rateOf(itemData(row)) == wanted) {
            setCurrentIndex(row);
            return;
        }
    }
}

} // namespace subedit::gui
