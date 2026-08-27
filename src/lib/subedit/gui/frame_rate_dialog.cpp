#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/frame_rate_box.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>

#include <cstddef>
#include <optional>

namespace subedit::gui {

FrameRateDialog::FrameRateDialog(std::size_t targetCount,
                                 core::FrameRate current,
                                 std::optional<core::FrameRate> declared,
                                 std::optional<core::FrameRate> deduced,
                                 QWidget* parent)
    : OperationDialog(targetCount, parent),
      m_input(new FrameRateBox{this}),
      m_output(new FrameRateBox{this}) {
    setWindowTitle(QStringLiteral("Convert frame rate"));

    // **The positions first, the project's assumption second.** Before phase 16
    // this field could only open on what the project carried, and the manual
    // said as much: the file does not declare its rate and nobody can guess it.
    // Now something can — not a guess but a measurement, and only when it is
    // clean.
    m_input->pick(current);
    if (deduced.has_value())
        m_input->pick(*deduced);

    // **What the film declares lands on « should play at », and nowhere else.**
    // A document is timed against something the file does not carry — that is
    // the field above, and only the user knows it. What the container names is
    // the rate the film actually runs at, which is what the subtitles have to
    // be brought to.
    //
    // **In two steps, because the second may find nothing** — see
    // `FrameRateBox`, which is where that contract is written. The declared
    // rate is still said below when the list does not carry it: knowing that
    // the film runs at something unusual is the information, and there is
    // simply nothing here to convert it to.
    m_output->pick(current);
    if (declared.has_value())
        m_output->pick(*declared);

    for (FrameRateBox* box : {m_input, m_output})
        connect(box, &QComboBox::currentIndexChanged, this, [this] { revalidate(); });

    fields()->addRow(QStringLiteral("Timed against"), m_input);
    fields()->addRow(QStringLiteral("Should play at"), m_output);

    // Added only when there is something to say. Without a film, without
    // `ffprobe`, and without a grid, this dialog is exactly the one that came
    // before it.
    //
    // **Both rows may show at once, saying different numbers**, and that is not
    // a fault to resolve: the film runs at one rate, the file was written on
    // another grid, and the difference is the reason the alignment exists.
    if (deduced.has_value()) {
        m_deduced = new QLabel{QString::fromStdString(core::nameOf(*deduced)), this};
        fields()->addRow(QStringLiteral("The positions say"), m_deduced);
    }

    if (declared.has_value()) {
        m_declared = new QLabel{QString::fromStdString(core::nameOf(*declared)), this};
        fields()->addRow(QStringLiteral("The video declares"), m_declared);
    }

    finish();
}

QString FrameRateDialog::declaredLabel() const {
    return m_declared == nullptr ? QString{} : m_declared->text();
}

QString FrameRateDialog::deducedLabel() const {
    return m_deduced == nullptr ? QString{} : m_deduced->text();
}

core::FrameRate FrameRateDialog::input() const {
    return m_input->rate();
}

core::FrameRate FrameRateDialog::output() const {
    return m_output->rate();
}

bool FrameRateDialog::isComplete() const {
    // Converting a rate into itself changes nothing: letting it be validated
    // would put an entry in the history for an unchanged file.
    return !(input() == output());
}

void FrameRateDialog::setRates(core::FrameRate from, core::FrameRate to) {
    m_input->pick(from);
    m_output->pick(to);
}

} // namespace subedit::gui
