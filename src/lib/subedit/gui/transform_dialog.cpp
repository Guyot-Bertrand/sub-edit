#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>

#include <cstddef>
#include <optional>

namespace subedit::gui {

namespace {

/// The one-based number of a subtitle, as the table shows it.
[[nodiscard]] QSpinBox* numberField(QWidget* parent, std::size_t subtitleCount) {
    auto* box = new QSpinBox{parent};
    box->setMinimum(1);
    box->setMaximum(subtitleCount == 0 ? 1 : static_cast<int>(subtitleCount));
    return box;
}

} // namespace

TransformDialog::TransformDialog(std::size_t targetCount,
                                 std::size_t subtitleCount,
                                 QWidget* parent)
    : OperationDialog(targetCount, parent),
      m_firstNumber(numberField(this, subtitleCount)),
      m_firstTarget(new QLineEdit{this}),
      m_secondNumber(numberField(this, subtitleCount)),
      m_secondTarget(new QLineEdit{this}) {
    setWindowTitle(QStringLiteral("Transform positions"));

    // The second reference defaults to the last subtitle: two distant
    // references give a surer correction than two neighbouring ones, and that
    // is what the user wants nine times out of ten.
    m_secondNumber->setValue(m_secondNumber->maximum());

    for (QLineEdit* target : {m_firstTarget, m_secondTarget}) {
        target->setPlaceholderText(QStringLiteral("00:00:01,000"));
        connect(target, &QLineEdit::textChanged, this, [this] { revalidate(); });
    }
    for (QSpinBox* number : {m_firstNumber, m_secondNumber})
        connect(number, &QSpinBox::valueChanged, this, [this] { revalidate(); });

    fields()->addRow(QStringLiteral("Subtitle"), m_firstNumber);
    fields()->addRow(QStringLiteral("really starts at"), m_firstTarget);
    fields()->addRow(QStringLiteral("Subtitle"), m_secondNumber);
    fields()->addRow(QStringLiteral("really starts at"), m_secondTarget);
    finish();
}

std::optional<TypedReference> TransformDialog::referenceOf(const QSpinBox& number,
                                                           const QLineEdit& target) {
    const std::optional<core::Timestamp> position =
        core::Timestamp::parse(target.text().toStdString());
    if (!position.has_value())
        return std::nullopt;

    return TypedReference{.number = number.value(), .target = *position};
}

std::optional<TypedReference> TransformDialog::first() const {
    return referenceOf(*m_firstNumber, *m_firstTarget);
}

std::optional<TypedReference> TransformDialog::second() const {
    return referenceOf(*m_secondNumber, *m_secondTarget);
}

bool TransformDialog::isComplete() const {
    const std::optional<TypedReference> one = first();
    const std::optional<TypedReference> other = second();
    if (!one.has_value() || !other.has_value())
        return false;

    // Two references on one subtitle define no correction: the core refuses it
    // by a zero denominator, and saying so here saves letting it be validated
    // for nothing.
    return one->number != other->number;
}

void TransformDialog::setTyped(int firstNumber,
                               const QString& firstTarget,
                               int secondNumber,
                               const QString& secondTarget) {
    m_firstNumber->setValue(firstNumber);
    m_firstTarget->setText(firstTarget);
    m_secondNumber->setValue(secondNumber);
    m_secondTarget->setText(secondTarget);
}

} // namespace subedit::gui
