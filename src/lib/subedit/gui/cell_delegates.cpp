#include <subedit/core/text/line_lengths.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/spell_highlighter.hpp>
#include <subedit/gui/subtitle_editor.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPlainTextEdit>
#include <QRect>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QWidget>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

/// The shape a timestamp may be typed in, and nothing more.
///
/// **It constrains the characters, not the bounds.** `00:70:00,000` goes
/// through, and deliberately: saying that a minute stops at sixty is
/// `Timestamp::parse`'s work, which it already does, and repeating it here
/// would leave two definitions to keep in agreement. What the shape refuses is
/// what the reading would never usefully catch — a letter in the middle of a
/// timestamp.
///
/// Permissive as the reading is: hours optional, one or two digits per field,
/// one to three decimals or none, comma or period.
constexpr auto kPositionPattern = R"(\s*-?\d{1,2}:\d{1,2}(:\d{1,2})?([.,]\d{1,3})?\s*)";

/// The shape of a position with no sign: what a duration may be typed in.
constexpr auto kDurationPattern = R"(\s*\d{1,2}:\d{1,2}(:\d{1,2})?([.,]\d{1,3})?\s*)";

/// The shape of a position counted in frames: a whole number, and the sign a start may have before
/// the film. Nine digits is more than any film has, and keeps a number of a size `toLongLong`
/// reads.
constexpr auto kFramePattern = R"(\s*-?\d{1,9}\s*)";

/// A one-line field that accepts `pattern`, and nothing more.
[[nodiscard]] QWidget* constrainedField(QWidget* parent, const char* pattern) {
    auto* editor = new QLineEdit{parent};
    editor->setValidator(
        new QRegularExpressionValidator{QRegularExpression{QString::fromUtf8(pattern)}, editor});
    return editor;
}

} // namespace

QWidget* TextDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& /*option*/,
                                    const QModelIndex& /*index*/) const {
    // `plainText` is a `QPlainTextEdit`'s USER property, which is all the
    // inherited `setEditorData` and `setModelData` need: they read and write
    // that one. Nothing to override to reach the text.
    auto* editor = new SubtitleEditor{parent};

    // Otherwise a tab would put a character in the text rather than move to
    // the next cell, which is not what anyone expects of a table.
    editor->setTabChangesFocus(true);

    // The lengths of the lines, when the window says they are to be shown.
    if (m_editorLengths)
        editor->showLengths(m_editorLengths());

    // Underlined as one types, when the window has a checker to give — the
    // highlighter is a child of the document and goes with the editor.
    if (m_spellChecker) {
        if (auto checker = m_spellChecker(); checker != nullptr)
            new SpellHighlighter{editor->document(), std::move(checker)};
    }

    // **The editor grows with what is typed into it**, and that closes the one
    // case the row height cannot: the row is two lines tall, the editor with
    // it, and a `Shift+Enter` making a third line would bring back the
    // scrollbar until the edit is validated. The row catches up afterwards, by
    // way of `dataChanged`; this is what holds until then.
    //
    // It grows downwards over the row beneath, which is what an editor is
    // allowed to do and a cell is not.
    connect(editor->document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            editor,
            [editor](const QSizeF&) {
                if (editor->height() < editor->heightOfItsLines())
                    editor->resize(editor->width(), editor->heightOfItsLines());
            });

    return editor;
}

namespace {

/// Between the end of a line and the length that follows it.
constexpr int kGapBeforeLength = 4;

/// The opacity of a length beside the line it belongs to, out of 255.
constexpr int kDimmedAlpha = 150;

/// What a line of a cell carries: its text, and its length once it has one.
///
/// **Gaupol writes no length after an empty line** (`_text_to_markup`), and
/// neither does this: a bare `[0]` in the middle of a blank row says nothing.
struct CellLine {
    QString text;
    QString length; // "[12]", or empty
};

[[nodiscard]] std::vector<CellLine> cellLinesOf(const QString& text,
                                                const LineLengthDisplay& display) {
    const std::vector<int> lengths =
        core::lineLengths(*display.measure, text.toStdString(), display.vocabulary);
    const QStringList lines = text.split(QLatin1Char('\n'));

    std::vector<CellLine> cells;
    for (qsizetype at = 0; at < lines.size(); ++at) {
        const auto position = static_cast<std::size_t>(at);
        const bool measured = !lines[at].isEmpty() && position < lengths.size();
        cells.push_back(
            {lines[at], measured ? QStringLiteral("[%1]").arg(lengths[position]) : QString{}});
    }
    return cells;
}

/// How wide a cell has to be to hold its lines and their lengths.
[[nodiscard]] int widthOfLines(const std::vector<CellLine>& lines,
                               const QFontMetrics& metrics,
                               const QFontMetrics& small) {
    int widest = 0;
    for (const CellLine& line : lines) {
        int width = metrics.horizontalAdvance(line.text);
        if (!line.length.isEmpty())
            width += kGapBeforeLength + small.horizontalAdvance(line.length);
        widest = std::max(widest, width);
    }
    return widest;
}

/// Which set of colours a cell is painted with, by what its state says.
[[nodiscard]] QPalette::ColorGroup colorGroupOf(QStyle::State state) {
    if (!state.testFlag(QStyle::State_Enabled))
        return QPalette::Disabled;
    return state.testFlag(QStyle::State_Active) ? QPalette::Active : QPalette::Inactive;
}

} // namespace

QSize TextDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.rheight() += roomAroundTheLines(option.widget);

    // **The lengths widen the cell and never make it taller**: they are set in
    // a smaller type on the baseline of their line. Only the width the style
    // measured for the bare text is replaced by the one that includes them.
    const auto display = m_cellLengths ? m_cellLengths() : std::nullopt;
    if (display.has_value()) {
        QStyleOptionViewItem filled = option;
        initStyleOption(&filled, index);
        const std::vector<CellLine> lines =
            cellLinesOf(index.data(Qt::DisplayRole).toString(), *display);
        const QFontMetrics metrics{filled.font};
        const QFontMetrics small{smallerFont(filled.font)};
        int bare = 0;
        for (const CellLine& line : lines)
            bare = std::max(bare, metrics.horizontalAdvance(line.text));
        size.rwidth() += widthOfLines(lines, metrics, small) - bare;
    }
    return size;
}

void TextDelegate::paint(QPainter* painter,
                         const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    const auto display = m_cellLengths ? m_cellLengths() : std::nullopt;
    if (!display.has_value()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // **The style paints everything but the text** — the background, the
    // tint of an anomaly, the selection, the focus — and this paints the text,
    // because a style writes one string in one font and the lengths are in
    // another.
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    // **The model's own text, and not `opt.text`**: `displayText` has turned its
    // line breaks into U+2028 by the time the style is given it.
    const QString text = index.data(Qt::DisplayRole).toString();
    const QWidget* widget = opt.widget;
    const QStyle* style = widget != nullptr ? widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt, widget) + 1;
    // **The cell, less the margin the style keeps on each side** — and not
    // `SE_ItemViewItemText`, which answers the rectangle the text *takes* and so
    // hugs it. A text cell has neither icon nor check box to make room for.
    const QRect area = opt.rect.adjusted(margin, 0, -margin, 0);

    opt.text.clear();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    const QColor ink = opt.palette.color(
        colorGroupOf(opt.state),
        opt.state.testFlag(QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);

    const std::vector<CellLine> lines = cellLinesOf(text, *display);
    const QFontMetrics metrics{opt.font};
    const QFont smaller = smallerFont(opt.font);
    const QFontMetrics small{smaller};

    painter->save();
    painter->setClipRect(area);
    const int used = static_cast<int>(lines.size()) * metrics.lineSpacing();
    const int spare = std::max(0, (area.height() - used) / 2);
    int baseline = area.top() + spare + metrics.ascent();
    for (const CellLine& line : lines) {
        const int lengthWidth =
            line.length.isEmpty() ? 0 : kGapBeforeLength + small.horizontalAdvance(line.length);
        const QString shown =
            metrics.elidedText(line.text, Qt::ElideRight, area.width() - lengthWidth);

        painter->setFont(opt.font);
        painter->setPen(ink);
        painter->drawText(QPoint{area.left(), baseline}, shown);

        if (!line.length.isEmpty()) {
            QColor dimmed = ink;
            dimmed.setAlpha(kDimmedAlpha);
            painter->setFont(smaller);
            painter->setPen(dimmed);
            painter->drawText(
                QPoint{area.left() + metrics.horizontalAdvance(shown) + kGapBeforeLength, baseline},
                line.length);
        }
        baseline += metrics.lineSpacing();
    }
    painter->restore();
}

bool TextDelegate::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        // `dynamic_cast` where the type of the event would do: the project's
        // rule refuses to walk down a hierarchy unchecked, and checking a
        // keystroke twice costs nothing measurable.
        const auto* key = dynamic_cast<const QKeyEvent*>(event);
        if (key != nullptr && (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)) {
            // **The line break belongs to the editor.** Handing the keystroke
            // back without validating anything is all there is to do: it is the
            // `QPlainTextEdit` that will make another line of it.
            if (key->modifiers().testFlag(Qt::ShiftModifier))
                return false;

            // Swallowed, and that is the point. The inherited filter
            // validates on `Enter` but hands the keystroke back to the editor —
            // right for a one-line field, disastrous here: the validated text
            // would carry the line break the validation had just refused.
            if (auto* editor = qobject_cast<QWidget*>(object); editor != nullptr) {
                emit commitData(editor);
                emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
                return true;
            }
        }
    }

    // Escape cancels, losing the focus validates: both come from Qt, and
    // nothing here touches them.
    return QStyledItemDelegate::eventFilter(object, event);
}

QWidget* PositionDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& /*option*/,
                                        const QModelIndex& index) const {
    // **The shape follows what the column shows**: when positions are frame numbers, a number is
    // what the cell takes, and a colon in it would be a timestamp the reading refuses.
    const auto* model = qobject_cast<const SubtitleTableModel*>(index.model());
    const bool frames = model != nullptr && model->frameRate().has_value();
    return constrainedField(parent, frames ? kFramePattern : kPositionPattern);
}

QWidget* DurationDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& /*option*/,
                                        const QModelIndex& /*index*/) const {
    return constrainedField(parent, kDurationPattern);
}

} // namespace subedit::gui
