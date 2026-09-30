#include <subedit/core/text/line_lengths.hpp>
#include <subedit/gui/subtitle_editor.hpp>

#include <QAbstractTextDocumentLayout>
#include <QColor>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStyle>
#include <QTextBlock>
#include <QTextDocument>
#include <QWidget>

#include <algorithm>

namespace subedit::gui {

namespace {

/// The margin a `QTextDocument` keeps on each side of its text.
///
/// **Read from a document rather than written here.** Qt's default is four
/// pixels today, and a four copied into this file would be a second truth that
/// nothing would keep in agreement. Read once and kept: a `QTextDocument` is
/// not free, and this is asked once per row of the table.
[[nodiscard]] int documentMargin() {
    static const int margin = static_cast<int>(QTextDocument{}.documentMargin());
    return margin;
}

/// The pixel the frame and the margin do not account for.
///
/// **Measured, and it is the same under both styles the window serves.** An
/// editor of `lines × spacing + 2 × (frame + margin)` keeps a vertical
/// scrollbar with two arrows and almost no travel, and hides the very line the
/// height was meant to show; one pixel more and it has nowhere to go. Where it
/// comes from inside `QPlainTextEdit` is not written here, because guessing at
/// it would be worse than measuring it — `cell_delegates_test.cpp` opens an
/// editor on one, two and three lines and asks its scrollbar the only question
/// that settles this.
constexpr int kRoundingPixel = 1;

/// Room either side of the numbers, in pixels — Gaupol's own 4 and 6.
constexpr int kGutterPadding = 4;

constexpr int kAlpha = 150;

} // namespace

int roomAroundTheLines(const QWidget* widget) {
    const QStyle* style = widget != nullptr ? widget->style() : nullptr;
    const int frame =
        style == nullptr ? 0 : style->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, widget);
    return (2 * (frame + documentMargin())) + kRoundingPixel;
}

/// The strip on the right of the field where the lengths are drawn.
///
/// A child of the field, laid over the room `setViewportMargins` leaves it, and
/// nothing more than a place to paint: what to write and where comes from the
/// field.
class SubtitleEditor::Gutter final : public QWidget {

public:
    explicit Gutter(SubtitleEditor& editor) : QWidget(&editor), m_editor(&editor) {}

protected:
    void paintEvent(QPaintEvent* event) override { m_editor->paintGutter(event); }

private:
    SubtitleEditor* m_editor;
};

SubtitleEditor::SubtitleEditor(QWidget* parent) : QPlainTextEdit(parent) {
    connect(this, &QPlainTextEdit::textChanged, this, [this] { refreshLengths(); });
    // Scrolling and any other repaint of the lines: the margin is redrawn with
    // the part of the viewport that moved.
    // A scrollbar that appears takes room from the viewport without the field
    // being resized, and the margin has to follow it.
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, [this] { placeGutter(); });
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect& area, int dy) {
        if (m_gutter == nullptr)
            return;
        if (dy != 0)
            m_gutter->scroll(0, dy);
        else
            m_gutter->update(0, area.y(), m_gutter->width(), area.height());
    });
}

QSize SubtitleEditor::sizeHint() const {
    return {QPlainTextEdit::sizeHint().width(), heightOfItsLines()};
}

int SubtitleEditor::heightOfItsLines() const {
    const auto lines = static_cast<int>(document()->documentLayout()->documentSize().height());
    return (std::max(1, lines) * fontMetrics().lineSpacing()) + roomAroundTheLines(this);
}

void SubtitleEditor::showLengths(std::optional<LineLengthDisplay> display) {
    m_display = std::move(display);
    if (m_display.has_value() && m_gutter == nullptr)
        m_gutter = new Gutter{*this};
    if (!m_display.has_value() && m_gutter != nullptr) {
        delete m_gutter;
        m_gutter = nullptr;
    }
    refreshLengths();
}

int SubtitleEditor::gutterWidth() const {
    if (!m_display.has_value())
        return 0;
    const int widest = m_lengths.empty() ? 0 : *std::ranges::max_element(m_lengths);
    const QFontMetrics metrics{smallerFont(font())};
    return metrics.horizontalAdvance(QString::number(widest)) + (2 * kGutterPadding);
}

void SubtitleEditor::refreshLengths() {
    m_lengths.clear();
    if (m_display.has_value() && !document()->isEmpty()) {
        m_lengths = core::lineLengths(
            *m_display->measure, toPlainText().toStdString(), m_display->vocabulary);
    }

    setViewportMargins(0, 0, gutterWidth(), 0);
    placeGutter();
    if (m_gutter != nullptr)
        m_gutter->update();
}

void SubtitleEditor::placeGutter() {
    if (m_gutter == nullptr)
        return;
    // Just right of the viewport, which is where `setViewportMargins` left the
    // room — and so left of the scrollbar, which the margin does not displace.
    const QRect view = viewport()->geometry();
    m_gutter->setGeometry(view.right() + 1, view.top(), gutterWidth(), view.height());
    m_gutter->show();
}

void SubtitleEditor::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    placeGutter();
}

void SubtitleEditor::paintGutter(QPaintEvent* event) {
    QPainter painter{m_gutter};
    painter.fillRect(event->rect(), palette().color(QPalette::Window));
    if (m_lengths.empty())
        return;

    QColor ink = palette().color(QPalette::WindowText);
    ink.setAlpha(kAlpha);
    painter.setPen(ink);
    painter.setFont(smallerFont(font()));

    // **Aligned on the baseline of the first row of each line**, not on the top
    // of it: the numbers are smaller than the text, and hung from the top they
    // would sit above the letters they belong to.
    const int ascent = fontMetrics().ascent();
    const int width = m_gutter->width();
    const QFontMetrics numbers{painter.font()};

    QTextBlock block = firstVisibleBlock();
    auto line = static_cast<std::size_t>(block.blockNumber());
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    while (block.isValid() && top <= event->rect().bottom() && line < m_lengths.size()) {
        if (block.isVisible()) {
            const QString number = QString::number(m_lengths[line]);
            painter.drawText(QPointF{static_cast<qreal>(width - kGutterPadding -
                                                        numbers.horizontalAdvance(number)),
                                     top + ascent},
                             number);
        }
        top += blockBoundingRect(block).height();
        block = block.next();
        ++line;
    }
}

} // namespace subedit::gui
