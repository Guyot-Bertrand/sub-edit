#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QVariant>

#include <cstddef>
#include <optional>

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

FrameRateDialog::FrameRateDialog(std::size_t targetCount,
                                 core::FrameRate current,
                                 std::optional<core::FrameRate> declared,
                                 QWidget* parent)
    : OperationDialog(targetCount, parent), m_input(rateField(this)), m_output(rateField(this)) {
    setWindowTitle(QStringLiteral("Convert frame rate"));

    pick(*m_input, current);

    // **What the film declares lands on « should play at », and nowhere else.**
    // A document is timed against something the file does not carry — that is
    // the field above, and only the user knows it. What the container names is
    // the rate the film actually runs at, which is what the subtitles have to
    // be brought to.
    //
    // **In two steps, and the second may find nothing.** A film may declare a
    // rate this list does not carry — the eight standards, closed on purpose —
    // and `pick` then leaves the box where the first step put it. Which is why
    // the first step exists: without it the box would stay where it was built,
    // on the first of the eight, and the dialog would open on a rate nobody
    // named. The declared rate is still said below, because knowing that the
    // film runs at something unusual is the information; there is simply
    // nothing here to convert it to.
    pick(*m_output, current);
    if (declared.has_value())
        pick(*m_output, *declared);

    for (QComboBox* box : {m_input, m_output})
        connect(box, &QComboBox::currentIndexChanged, this, [this] { revalidate(); });

    fields()->addRow(QStringLiteral("Timed against"), m_input);
    fields()->addRow(QStringLiteral("Should play at"), m_output);

    // Added only when there is something to say. Without a film, or without
    // `ffprobe`, this dialog is exactly the one that came before it.
    if (declared.has_value()) {
        m_declared = new QLabel{QString::fromStdString(core::nameOf(*declared)), this};
        fields()->addRow(QStringLiteral("The video declares"), m_declared);
    }

    finish();
}

QString FrameRateDialog::declaredLabel() const {
    return m_declared == nullptr ? QString{} : m_declared->text();
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
