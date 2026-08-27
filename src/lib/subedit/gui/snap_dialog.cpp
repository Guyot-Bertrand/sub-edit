#include <subedit/core/wording.hpp>
#include <subedit/gui/snap_dialog.hpp>

#include <QComboBox>
#include <QFormLayout>
#include <QString>
#include <QVariant>

#include <cstddef>
#include <optional>

namespace subedit::gui {

namespace {

/// The eight standards, named as a report names them.
[[nodiscard]] QComboBox* rateField(QWidget* parent) {
    auto* box = new QComboBox{parent};
    for (const core::StandardFrameRate standard : core::kStandardFrameRates) {
        const core::FrameRate rate{standard};
        box->addItem(QString::fromStdString(core::nameOf(rate)),
                     QVariant::fromValue(static_cast<int>(standard)));
    }
    return box;
}

void pick(QComboBox& box, core::FrameRate rate) {
    for (int row = 0; row < box.count(); ++row) {
        if (core::FrameRate{static_cast<core::StandardFrameRate>(box.itemData(row).toInt())} ==
            rate) {
            box.setCurrentIndex(row);
            return;
        }
    }
}

} // namespace

SnapDialog::SnapDialog(std::size_t targetCount,
                       core::FrameRate current,
                       std::optional<core::FrameRate> declared,
                       QWidget* parent)
    : OperationDialog(targetCount, parent), m_rate(rateField(this)) {
    setWindowTitle(QStringLiteral("Snap to frame rate"));

    // In two steps, and the second may find nothing: a film may declare a rate
    // this closed list does not carry, and the box then stays where the first
    // step put it rather than on whichever rate happened to be built first.
    pick(*m_rate, current);
    if (declared.has_value())
        pick(*m_rate, *declared);

    fields()->addRow(QStringLiteral("Lay positions on"), m_rate);

    finish();
}

core::FrameRate SnapDialog::rate() const {
    return core::FrameRate{static_cast<core::StandardFrameRate>(m_rate->currentData().toInt())};
}

void SnapDialog::setRate(core::FrameRate rate) {
    pick(*m_rate, rate);
}

} // namespace subedit::gui
