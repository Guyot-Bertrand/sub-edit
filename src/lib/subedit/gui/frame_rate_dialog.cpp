#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>

#include <QComboBox>
#include <QFormLayout>
#include <QString>
#include <QVariant>

#include <cstddef>

namespace subedit::gui {

namespace {

/// The eight standards, named as a report names them.
///
/// A closed list rather than free entry: a rate that is not one of these is a
/// rate no video carries, and the conversion of a whole file is not the place
/// to accept a typo.
[[nodiscard]] QComboBox* rateField(QWidget* parent) {
    auto* box = new QComboBox{parent};
    for (const core::StandardFrameRate standard : core::kStandardFrameRates) {
        const core::FrameRate rate{standard};
        box->addItem(QString::fromStdString(core::nameOf(rate)),
                     QVariant::fromValue(static_cast<int>(standard)));
    }
    return box;
}

[[nodiscard]] core::FrameRate rateOf(const QComboBox& box) {
    return core::FrameRate{static_cast<core::StandardFrameRate>(box.currentData().toInt())};
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

FrameRateDialog::FrameRateDialog(std::size_t targetCount, core::FrameRate current, QWidget* parent)
    : OperationDialog(targetCount, parent), m_input(rateField(this)), m_output(rateField(this)) {
    setWindowTitle(QStringLiteral("Convert frame rate"));

    pick(*m_input, current);
    pick(*m_output, current);

    for (QComboBox* box : {m_input, m_output})
        connect(box, &QComboBox::currentIndexChanged, this, [this] { revalidate(); });

    fields()->addRow(QStringLiteral("Timed against"), m_input);
    fields()->addRow(QStringLiteral("Should play at"), m_output);
    finish();
}

core::FrameRate FrameRateDialog::input() const {
    return rateOf(*m_input);
}

core::FrameRate FrameRateDialog::output() const {
    return rateOf(*m_output);
}

bool FrameRateDialog::isComplete() const {
    // Converting a rate into itself changes nothing: letting it be validated
    // would put an entry in the history for an unchanged file.
    return !(input() == output());
}

void FrameRateDialog::setRates(core::FrameRate from, core::FrameRate to) {
    pick(*m_input, from);
    pick(*m_output, to);
}

} // namespace subedit::gui
