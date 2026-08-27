#include <subedit/gui/frame_rate_box.hpp>
#include <subedit/gui/snap_dialog.hpp>

#include <QFormLayout>
#include <QString>

#include <cstddef>
#include <optional>

namespace subedit::gui {

SnapDialog::SnapDialog(std::size_t targetCount,
                       core::FrameRate current,
                       std::optional<core::FrameRate> declared,
                       QWidget* parent)
    : OperationDialog(targetCount, parent), m_rate(new FrameRateBox{this}) {
    setWindowTitle(QStringLiteral("Snap to frame rate"));

    // In two steps, because the second may find nothing — see `FrameRateBox`,
    // which is where that contract is written.
    m_rate->pick(current);
    if (declared.has_value())
        m_rate->pick(*declared);

    fields()->addRow(QStringLiteral("Lay positions on"), m_rate);

    finish();
}

core::FrameRate SnapDialog::rate() const {
    return m_rate->rate();
}

void SnapDialog::setRate(core::FrameRate rate) {
    m_rate->pick(rate);
}

} // namespace subedit::gui
