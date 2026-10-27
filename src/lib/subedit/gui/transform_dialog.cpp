#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/read_only_area.hpp>
#include <subedit/gui/timestamp_edit.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QFont>
#include <QFontMetricsF>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMargins>
#include <QPalette>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QSpinBox>
#include <QString>
#include <QTextDocument>
#include <QTextOption>
#include <QWidget>

#include <cmath>
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

/// A paragraph that wraps, and asks for the height its wrapped text needs.
///
/// **A wrapping `QLabel` is not enough on its own**: its height depends on its
/// width, and the form gives it the height of its size hint, worked out at a
/// width of the label's own choosing. Narrower than that — a smaller window, a
/// longer translation — the last lines were cut (#717). The minimum height is
/// therefore the one the text needs at the width the label was actually given,
/// and it follows every resize.
class WrappedParagraph final : public QLabel {
public:
    WrappedParagraph(const QString& text, QWidget* parent) : QLabel{text, parent} {
        setWordWrap(true);
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QLabel::resizeEvent(event);
        const int needed = heightForWidth(width());
        if (needed > 0 && needed != minimumHeight())
            setMinimumHeight(needed);
    }
};

/// How many lines of a subtitle the reference rows show before they scroll.
constexpr int kShownLines = 3;

/// The air between the text of a reference and the frame around it.
constexpr int kTextMargin = 3;

/// Where the text of a reference is shown: read-only, wrapped, and always the
/// height of three lines of the dialog's font.
///
/// **Not a label**, for two reasons: a wrapping label is as tall as the text it
/// shows, so the dialog jumped with every reference, and a long text had no
/// way to be read whole. **Not an input either**: it has the look of a read-only
/// area (`styleAsReadOnly`, a frame and a background of its own, so that one sees
/// where it ends) — only the new start is the user's to type in. A click still
/// gives it the focus, so that its text can be copied.
[[nodiscard]] QPlainTextEdit* textArea(QWidget* parent) {
    auto* area = new QPlainTextEdit{parent};
    styleAsReadOnly(*area);
    area->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    // A long address has no space to break at, and is cut anywhere.
    area->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // A little air between the text and the frame.
    area->document()->setDocumentMargin(kTextMargin);

    // Each line rounded up to a whole pixel, as the text layout lays them, the
    // document's margin above and below them, and the one pixel
    // `QPlainTextEdit` keeps clear before it counts a line as visible: one
    // pixel less and three lines already scroll.
    const QMargins margins = area->contentsMargins();
    const auto line = static_cast<int>(std::ceil(QFontMetricsF{area->font()}.lineSpacing()));
    const auto documentMargin = static_cast<int>(std::ceil(area->document()->documentMargin()));
    area->setFixedHeight((kShownLines * line) + (2 * documentMargin) + 1 +
                         (2 * area->frameWidth()) + margins.top() + margins.bottom());
    return area;
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

TransformDialog::TransformDialog(OperationScope scope, AnchorLookup lookup, QWidget* parent)
    : OperationDialog(scope, parent),
      m_lookup(std::move(lookup)),
      m_first(makeRow(scope.whole)),
      m_second(makeRow(scope.whole)) {
    setWindowTitle(translate("Transform positions"));

    // The second reference defaults to the last subtitle: two distant
    // references give a surer correction than two neighbouring ones, and that
    // is what the user wants nine times out of ten.
    m_second.number->setValue(m_second.number->maximum());

    // Named for the tests: the two rows are otherwise built alike.
    m_first.text->setObjectName(QStringLiteral("first-text"));
    m_second.text->setObjectName(QStringLiteral("second-text"));

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
        fields()->addRow(translate("Subtitle"), row.number);
        fields()->addRow(translate("Now starts"), row.current);
        fields()->addRow(translate("Should start"), row.target);
        fields()->addRow(translate("Text"), row.text);
    };
    auto* explanation =
        new WrappedParagraph{translate("Pick two subtitles and say when each one should start.\n"
                                       "All the other subtitles are moved and stretched to match."),
                             this};
    explanation->setObjectName(QStringLiteral("explanation"));
    fields()->addRow(explanation);

    heading(translate("Reference 1"));
    add(m_first);
    heading(translate("Reference 2"));
    add(m_second);
    finish();
}

TransformDialog::Row TransformDialog::makeRow(std::size_t subtitleCount) {
    // A label, not a read-only field: a greyed field still looks like one to
    // type in, and only the new start is the user's to change.
    auto* current = new QLabel{this};

    auto* text = textArea(this);

    auto* number = numberField(this, subtitleCount);
    auto* target = new TimestampEdit{{.negative = true}, this};

    return Row{.number = number, .current = current, .target = target, .text = text};
}

void TransformDialog::refresh(const Row& row) {
    const std::optional<AnchorView> view = m_lookup ? m_lookup(row.number->value()) : std::nullopt;
    row.current->setText(view ? view->start : QString{});
    row.text->setPlainText(view ? view->text : QString{});

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
    return m_first.text->toPlainText();
}

QString TransformDialog::secondText() const {
    return m_second.text->toPlainText();
}

} // namespace subedit::gui
