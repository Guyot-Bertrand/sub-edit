#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/transcription_preview_dialog.hpp>

#include <QBrush>
#include <QColor>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cstddef>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// Wide enough for two tables of three columns side by side, and tall enough to read a screenful.
constexpr int kMinimumWidth = 1000;
constexpr int kMinimumHeight = 520;
constexpr int kTimeColumnWidth = 110;

/// How much lighter than the selection colour the sources are painted.
constexpr int kHighlightLightness = 150;

constexpr int kStartColumn = 0;
constexpr int kEndColumn = 1;
constexpr int kTextColumn = 2;

[[nodiscard]] QString timeText(core::Timestamp moment) {
    return QString::fromStdString(moment.format(
        core::DecimalMark::Period, core::HourField::Always, core::Decimals::Milliseconds));
}

[[nodiscard]] const std::string& textOf(const core::TranscribedSegment& piece) {
    return piece.text;
}

[[nodiscard]] const std::string& textOf(const core::Subtitle& subtitle) {
    return subtitle.mainText;
}

/// Fills `table` with rows of (start, end, text).
template<typename Row>
void fill(QTableWidget& table, const std::vector<Row>& rows) {
    table.setRowCount(static_cast<int>(rows.size()));
    int row = 0;
    for (const Row& one : rows) {
        const auto cell = [](const QString& text) {
            auto* item = new QTableWidgetItem{text};
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            return item;
        };
        table.setItem(row, kStartColumn, cell(timeText(one.start)));
        table.setItem(row, kEndColumn, cell(timeText(one.end)));
        table.setItem(row, kTextColumn, cell(QString::fromStdString(textOf(one))));
        ++row;
    }
}

/// A table of three read-only columns.
[[nodiscard]] QTableWidget* tableOf(QWidget* parent) {
    auto* table = new QTableWidget{0, 3, parent};
    table->setHorizontalHeaderLabels(
        QStringList{translate("Start"), translate("End"), translate("Text")});
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setWordWrap(false);
    table->horizontalHeader()->setSectionResizeMode(kTextColumn, QHeaderView::Stretch);
    table->setColumnWidth(kStartColumn, kTimeColumnWidth);
    table->setColumnWidth(kEndColumn, kTimeColumnWidth);
    return table;
}

} // namespace

TranscriptionPreviewDialog::TranscriptionPreviewDialog(TranscriptionPreview preview,
                                                       QWidget* parent)
    : QDialog(parent),
      m_preview(std::move(preview)),
      m_summary(new QLabel{this}),
      m_pieces(tableOf(this)),
      m_subtitles(tableOf(this)),
      m_apply(new QPushButton{translate("Apply"), this}),
      m_ignore(new QPushButton{translate("Ignore"), this}) {
    setWindowTitle(translate("Transcription preview"));
    setMinimumSize(kMinimumWidth, kMinimumHeight);

    // The left side is for reading: nothing is selected there, the highlight is the only mark.
    m_pieces->setSelectionMode(QAbstractItemView::NoSelection);
    m_pieces->setFocusPolicy(Qt::NoFocus);
    m_subtitles->setSelectionMode(QAbstractItemView::SingleSelection);
    m_subtitles->setSelectionBehavior(QAbstractItemView::SelectRows);

    fill(*m_pieces, m_preview.pieces);
    fill(*m_subtitles, m_preview.assembly.subtitles);

    if (m_preview.partialUpTo) {
        m_summary->setText(substitute(
            translate("Partial result: the work stopped at %1. What was obtained up to there "
                      "can be applied or ignored."),
            QString::fromStdString(core::clockText(*m_preview.partialUpTo))));
    } else {
        m_summary->setText(
            translate("Select a subtitle on the right to see the pieces it was made from."));
    }
    m_summary->setWordWrap(true);

    connect(m_subtitles, &QTableWidget::currentCellChanged, this, [this](int row) {
        showSources(row);
    });
    connect(m_apply, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_ignore, &QPushButton::clicked, this, &QDialog::reject);
    m_apply->setDefault(true);
    // Nothing to apply, nothing to replace the project's subtitles with.
    m_apply->setEnabled(!m_preview.assembly.subtitles.empty());

    auto* left = new QGroupBox{substitute(translate("As the engine wrote it (%1)"),
                                          QString::number(m_preview.pieces.size())),
                               this};
    auto* leftLayout = new QVBoxLayout{left};
    leftLayout->addWidget(m_pieces);
    auto* right = new QGroupBox{substitute(translate("As they will be applied (%1)"),
                                           QString::number(m_preview.assembly.subtitles.size())),
                                this};
    auto* rightLayout = new QVBoxLayout{right};
    rightLayout->addWidget(m_subtitles);

    auto* tables = new QHBoxLayout;
    tables->addWidget(left);
    tables->addWidget(right);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch(1);
    buttons->addWidget(m_ignore);
    buttons->addWidget(m_apply);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(m_summary);
    stack->addLayout(tables, 1);
    stack->addLayout(buttons);
}

void TranscriptionPreviewDialog::showSources(int row) {
    const QColor mark = palette().color(QPalette::Highlight).lighter(kHighlightLightness);
    const auto paint = [this](int piece, const QBrush& brush) {
        for (int column = 0; column < m_pieces->columnCount(); ++column)
            if (QTableWidgetItem* item = m_pieces->item(piece, column))
                item->setBackground(brush);
    };
    for (const int piece : m_highlighted)
        paint(piece, QBrush{});
    m_highlighted.clear();

    const auto& sources = m_preview.assembly.sources;
    if (row < 0 || std::cmp_greater_equal(row, sources.size()))
        return;
    for (const std::size_t piece : sources[static_cast<std::size_t>(row)]) {
        m_highlighted.push_back(static_cast<int>(piece));
        paint(static_cast<int>(piece), QBrush{mark});
    }
    if (!m_highlighted.empty())
        m_pieces->scrollToItem(m_pieces->item(m_highlighted.front(), kTextColumn),
                               QAbstractItemView::PositionAtCenter);
}

} // namespace subedit::gui
