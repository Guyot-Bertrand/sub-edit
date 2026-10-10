#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>

#include <cstddef>
#include <optional>
#include <utility>

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

std::optional<AnchorView> anchorIn(const core::Project& project, int number) {
    if (number < 1 || static_cast<std::size_t>(number) > project.count())
        return std::nullopt;

    const core::Subtitle& subtitle =
        project.subtitleAt(core::SubtitleIndex::fromNumber(static_cast<std::size_t>(number)));
    return AnchorView{
        .start = QString::fromStdString(subtitle.start.format(core::DecimalMark::Comma)),
        .text = QString::fromStdString(subtitle.mainText),
    };
}

TransformDialog::TransformDialog(std::size_t targetCount,
                                 std::size_t subtitleCount,
                                 AnchorLookup lookup,
                                 QWidget* parent)
    : OperationDialog(targetCount, parent),
      m_lookup(std::move(lookup)),
      m_first(makeRow(subtitleCount)),
      m_second(makeRow(subtitleCount)) {
    setWindowTitle(QStringLiteral("Transform positions"));

    // The second reference defaults to the last subtitle: two distant
    // references give a surer correction than two neighbouring ones, and that
    // is what the user wants nine times out of ten.
    m_second.number->setValue(m_second.number->maximum());

    for (const Row* row : {&m_first, &m_second}) {
        connect(row->target, &QLineEdit::textChanged, this, [this] { revalidate(); });
        connect(row->number, &QSpinBox::valueChanged, this, [this, row](int) {
            refresh(*row);
            revalidate();
        });
        refresh(*row);
    }

    const auto heading = [this](const QString& title) {
        auto* label = new QLabel{title, this};
        QFont bold = label->font();
        bold.setBold(true);
        label->setFont(bold);
        fields()->addRow(label);
    };
    const auto add = [this](const Row& row) {
        fields()->addRow(QStringLiteral("Subtitle"), row.number);
        fields()->addRow(QStringLiteral("Now starts"), row.current);
        fields()->addRow(QStringLiteral("Should start"), row.target);
        fields()->addRow(QStringLiteral("Text"), row.text);
    };
    auto* explanation =
        new QLabel{QStringLiteral("Pick two subtitles and say when each one should start.\n"
                                  "All the other subtitles are moved and stretched to match."),
                   this};
    explanation->setWordWrap(true);
    fields()->addRow(explanation);

    heading(QStringLiteral("Reference 1"));
    add(m_first);
    heading(QStringLiteral("Reference 2"));
    add(m_second);
    finish();
}

TransformDialog::Row TransformDialog::makeRow(std::size_t subtitleCount) {
    // A label, not a read-only field: a greyed field still looks like one to
    // type in, and only the new start is the user's to change.
    auto* current = new QLabel{this};

    auto* text = new QLabel{this};
    text->setWordWrap(true);

    auto* number = numberField(this, subtitleCount);
    auto* target = new QLineEdit{this};

    return Row{.number = number, .current = current, .target = target, .text = text};
}

void TransformDialog::refresh(const Row& row) {
    const std::optional<AnchorView> view = m_lookup ? m_lookup(row.number->value()) : std::nullopt;
    row.current->setText(view ? view->start : QString{});
    row.text->setText(view ? view->text : QString{});

    // Offered, not imposed: the field is the user's to overwrite, and a number
    // with nothing behind it leaves what was typed alone.
    if (view.has_value())
        row.target->setText(view->start);
}

std::optional<TypedReference> TransformDialog::referenceOf(const Row& row) {
    const std::optional<core::Timestamp> position =
        core::Timestamp::parse(row.target->text().toStdString());
    if (!position.has_value())
        return std::nullopt;

    return TypedReference{.number = row.number->value(), .target = *position};
}

std::optional<TypedReference> TransformDialog::first() const {
    return referenceOf(m_first);
}

std::optional<TypedReference> TransformDialog::second() const {
    return referenceOf(m_second);
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
                               const QString& secondTarget) const {
    m_first.number->setValue(firstNumber);
    m_first.target->setText(firstTarget);
    m_second.number->setValue(secondNumber);
    m_second.target->setText(secondTarget);
}

QString TransformDialog::firstCurrent() const {
    return m_first.current->text();
}

QString TransformDialog::secondCurrent() const {
    return m_second.current->text();
}

QString TransformDialog::firstText() const {
    return m_first.text->text();
}

QString TransformDialog::secondText() const {
    return m_second.text->text();
}

} // namespace subedit::gui
