#include <subedit/core/wording/alignment.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/synchronization_preview.hpp>
#include <subedit/gui/synchronization_preview_dialog.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cstddef>
#include <utility>

namespace subedit::gui {

namespace {

constexpr int kMinimumWidth = 900;
constexpr int kMinimumHeight = 480;
constexpr int kTimeColumnWidth = 110;

constexpr int kNumberColumn = 0;
constexpr int kTextColumn = 1;
constexpr int kWasColumn = 2;
constexpr int kNowColumn = 3;
constexpr int kShiftColumn = 4;
constexpr int kCheckColumn = 5;
constexpr int kColumns = 6;

[[nodiscard]] QString timeText(core::Timestamp moment) {
    return QString::fromStdString(moment.format(
        core::DecimalMark::Period, core::HourField::Always, core::Decimals::Milliseconds));
}

[[nodiscard]] QString shiftText(std::int64_t milliseconds) {
    return QString::fromStdString(core::secondsOf(core::Duration::fromMilliseconds(milliseconds)));
}

/// What there is to check about the subtitle, in words, or nothing.
[[nodiscard]] QString checkText(const core::LineVerdict& verdict,
                                const core::RetimedLine& retimed) {
    QStringList notes;
    if (verdict.doubt != core::LineDoubt::None)
        notes.append(QString::fromStdString(core::doubtText(verdict.doubt)));
    if (retimed.trimmed)
        notes.append(QString::fromStdString(core::lineCutBackText()));
    if (retimed.overlapping)
        notes.append(QString::fromStdString(core::lineOverlapsText()));
    return notes.join(QStringLiteral("; "));
}

[[nodiscard]] QTableWidgetItem* cell(const QString& text) {
    auto* item = new QTableWidgetItem{text};
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

} // namespace

SynchronizationPreviewDialog::SynchronizationPreviewDialog(SynchronizationPreview preview,
                                                           QWidget* parent)
    : QDialog(parent),
      m_preview(std::make_unique<SynchronizationPreview>(std::move(preview))),
      m_summary(new QLabel{this}),
      m_onlyToCheck(new QCheckBox{translate("Only the subtitles to check"), this}),
      m_table(new QTableWidget{0, kColumns, this}),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Cancel, this}) {
    setWindowTitle(translate("Synchronize with audio"));
    setMinimumSize(kMinimumWidth, kMinimumHeight);

    m_summary->setWordWrap(true);
    m_summary->setText(QString::fromStdString(core::noticeOfAlignment(m_preview->plan.report)));

    m_table->setHorizontalHeaderLabels(QStringList{translate("No."),
                                                   translate("Text"),
                                                   translate("Was"),
                                                   translate("Now"),
                                                   translate("Shift"),
                                                   translate("To check")});
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setWordWrap(false);
    m_table->horizontalHeader()->setSectionResizeMode(kTextColumn, QHeaderView::Stretch);
    m_table->setColumnWidth(kWasColumn, kTimeColumnWidth);
    m_table->setColumnWidth(kNowColumn, kTimeColumnWidth);

    const core::SynchronizationPlan& plan = m_preview->plan;
    m_table->setRowCount(static_cast<int>(plan.subtitles.size()));
    for (std::size_t line = 0; line < plan.subtitles.size(); ++line) {
        const auto row = static_cast<int>(line);
        const core::LineVerdict& verdict = plan.report.lines[line];
        m_table->setItem(row, kNumberColumn, cell(QString::number(plan.subtitles[line] + 1)));
        m_table->setItem(row, kTextColumn, cell(QString::fromStdString(m_preview->texts[line])));
        m_table->setItem(row, kWasColumn, cell(timeText(plan.originals[line].start)));
        m_table->setItem(row, kNowColumn, cell(timeText(plan.retimed[line].times.start)));
        m_table->setItem(
            row,
            kShiftColumn,
            cell(verdict.shiftMilliseconds ? shiftText(*verdict.shiftMilliseconds) : QString{}));
        m_table->setItem(row, kCheckColumn, cell(checkText(verdict, plan.retimed[line])));
    }

    // Applied only when something moves.
    QPushButton* apply = m_buttons->addButton(translate("Apply"), QDialogButtonBox::AcceptRole);
    const bool moves = plan.changed() > 0;
    apply->setEnabled(moves);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_onlyToCheck, &QCheckBox::toggled, this, [this](bool only) { showOnlyToCheck(only); });

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(m_summary);
    stack->addWidget(m_onlyToCheck);
    stack->addWidget(m_table);
    stack->addWidget(m_buttons);
}

SynchronizationPreviewDialog::~SynchronizationPreviewDialog() = default;

void SynchronizationPreviewDialog::showOnlyToCheck(bool only) {
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem* check = m_table->item(row, kCheckColumn);
        m_table->setRowHidden(row, only && (check == nullptr || check->text().isEmpty()));
    }
}

} // namespace subedit::gui
