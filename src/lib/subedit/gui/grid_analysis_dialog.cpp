#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

#include <cstddef>
#include <string>

namespace subedit::gui {

namespace {

/// Everything the deduction concluded, in the order a reader needs it.
///
/// The answer first, then what qualifies it. Each sentence after the first
/// appears only when it has something to say: a file on a clean grid with no
/// ambiguity gets one line, which is all there is to know about it.
[[nodiscard]] QString summaryOf(const core::FrameRateDeduction& deduction) {
    QStringList lines;

    if (!deduction.enoughStarts) {
        lines << QObject::tr("Too few subtitles to tell: %1 of them.").arg(deduction.starts);
        return lines.join(QStringLiteral("\n"));
    }

    if (deduction.verdict == core::GridVerdict::Silent) {
        lines << QObject::tr("No frame rate grid explains these positions. "
                             "The closest candidate reaches %1.")
                     .arg(QString::fromStdString(
                         core::percentOf(deduction.ranked.front().concentration)));
    } else {
        lines << QObject::tr("Written on a %1 fps grid — %2, %3.")
                     .arg(
                         QString::fromStdString(nameOf(deduction.retained.rate)),
                         QString::fromStdString(std::string{nameOf(deduction.verdict)}),
                         QString::fromStdString(core::percentOf(deduction.retained.concentration)));
    }

    lines << QObject::tr("Measured on %1 starts, over %2.")
                 .arg(deduction.starts)
                 .arg(QString::fromStdString(core::secondsOf(deduction.span)));

    if (deduction.retained.phase != core::Duration::zero())
        lines << QObject::tr("Shifted off the grid by %1.")
                     .arg(QString::fromStdString(core::secondsOf(deduction.retained.phase)));

    if (deduction.harmonic.has_value())
        lines << QObject::tr("%1 fps fits just as well, being a whole multiple of it.")
                     .arg(QString::fromStdString(nameOf(*deduction.harmonic)));

    for (const core::FrameRate other : deduction.notSeparated)
        lines << QObject::tr("This span is too short to tell it from %1 fps.")
                     .arg(QString::fromStdString(nameOf(other)));

    if (!deduction.strays.empty())
        lines << QObject::tr("%1 of these starts leave the grid, in %2 runs.")
                     .arg(deduction.strays.size())
                     .arg(core::runsOfStrays(deduction));

    return lines.join(QStringLiteral("\n"));
}

} // namespace

GridAnalysisDialog::GridAnalysisDialog(const core::FrameRateDeduction& deduction, QWidget* parent)
    : QDialog(parent),
      m_summary(new QLabel{summaryOf(deduction), this}),
      m_ranking(new QTableWidget{static_cast<int>(deduction.ranked.size()), 2, this}) {
    setWindowTitle(QStringLiteral("Frame Rate Analysis"));

    m_summary->setWordWrap(true);

    m_ranking->setHorizontalHeaderLabels(
        QStringList{QStringLiteral("Frame rate"), QStringLiteral("Fit")});
    m_ranking->verticalHeader()->setVisible(false);
    m_ranking->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ranking->setSelectionMode(QAbstractItemView::NoSelection);
    m_ranking->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (std::size_t rank = 0; rank < deduction.ranked.size(); ++rank) {
        const int row = static_cast<int>(rank);
        const core::GridFit& fit = deduction.ranked[rank];
        m_ranking->setItem(row,
                           0,
                           new QTableWidgetItem{QStringLiteral("%1 fps").arg(
                               QString::fromStdString(nameOf(fit.rate)))});
        m_ranking->setItem(
            row,
            1,
            new QTableWidgetItem{QString::fromStdString(core::percentOf(fit.concentration))});
    }

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_summary);
    layout->addWidget(m_ranking);
    layout->addWidget(buttons);
}

QString GridAnalysisDialog::summary() const {
    return m_summary->text();
}

int GridAnalysisDialog::candidateCount() const {
    return m_ranking->rowCount();
}

QString GridAnalysisDialog::candidateAt(int row) const {
    return m_ranking->item(row, 0)->text() + QStringLiteral(" ") + m_ranking->item(row, 1)->text();
}

} // namespace subedit::gui
