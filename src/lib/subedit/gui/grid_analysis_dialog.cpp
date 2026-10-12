#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_repair.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/messages.hpp>

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
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

    const auto number = [](std::size_t value) { return QString::number(value); };
    const auto text = [](const std::string& value) { return QString::fromStdString(value); };

    if (!deduction.enoughStarts) {
        // TRANSLATORS: %1 is the number of subtitles the file has, too few to measure.
        lines << substitute(translate("Too few subtitles to tell: %1 of them."),
                            number(deduction.starts));
        return lines.join(QStringLiteral("\n"));
    }

    if (deduction.verdict == core::GridVerdict::Silent) {
        // TRANSLATORS: %1 is a share, « 41.2 % ».
        lines << substitute(translate("No frame rate grid explains these positions. "
                                      "The closest candidate reaches %1."),
                            text(core::percentOf(deduction.ranked.front().concentration)));
    } else {
        // TRANSLATORS: %1 is a frame rate, %2 how clean the grid is (« clean »), %3 a share.
        lines << substitute(translate("Written on a %1 fps grid — %2, %3."),
                            text(nameOf(deduction.retained.rate)),
                            text(std::string{nameOf(deduction.verdict)}),
                            text(core::percentOf(deduction.retained.concentration)));
    }

    // TRANSLATORS: %1 is a number of subtitle starts, %2 a length of time, « 1:32:05 ».
    lines << substitute(translatePlural("Measured on %1 start, over %2.",
                                        "Measured on %1 starts, over %2.",
                                        deduction.starts),
                        number(deduction.starts),
                        text(core::secondsOf(deduction.span)));

    if (deduction.retained.phase != core::Duration::zero())
        // TRANSLATORS: %1 is a length of time, « 0.020 s ».
        lines << substitute(translate("Shifted off the grid by %1."),
                            text(core::secondsOf(deduction.retained.phase)));

    if (deduction.harmonic.has_value())
        // TRANSLATORS: %1 is a frame rate.
        lines << substitute(translate("%1 fps fits just as well, being a whole multiple of it."),
                            text(nameOf(*deduction.harmonic)));

    for (const core::FrameRate other : deduction.notSeparated)
        // TRANSLATORS: %1 is a frame rate.
        lines << substitute(translate("This span is too short to tell it from %1 fps."),
                            text(nameOf(other)));

    if (!deduction.strays.empty())
        // TRANSLATORS: %1 is a number of starts, %2 a number of runs, « 3 runs ».
        lines << substitute(translatePlural("%1 of these starts leaves the grid, in %2.",
                                            "%1 of these starts leave the grid, in %2.",
                                            deduction.strays.size()),
                            number(deduction.strays.size()),
                            text(core::countOf(core::runsOfStrays(deduction), core::Counted::Run)));

    return lines.join(QStringLiteral("\n"));
}

} // namespace

GridAnalysisDialog::GridAnalysisDialog(const core::FrameRateDeduction& deduction, QWidget* parent)
    : GridAnalysisDialog(deduction, core::GridRepair{}, parent) {}

GridAnalysisDialog::GridAnalysisDialog(const core::FrameRateDeduction& deduction,
                                       const core::GridRepair& repair,
                                       QWidget* parent)
    : QDialog(parent),
      m_summary(new QLabel{summaryOf(deduction), this}),
      m_ranking(new QTableWidget{static_cast<int>(deduction.ranked.size()), 2, this}) {
    setWindowTitle(translate("Frame Rate Analysis"));

    // What the search for a conversion found goes under the deduction's own account, in the
    // words the command line will use.
    if (const std::string repairing = core::repairNoticeOf(repair); !repairing.empty())
        m_summary->setText(m_summary->text() + QStringLiteral("\n") +
                           QString::fromStdString(repairing));
    m_summary->setWordWrap(true);

    m_ranking->setHorizontalHeaderLabels(QStringList{translate("Frame rate"), translate("Fit")});
    m_ranking->verticalHeader()->setVisible(false);
    m_ranking->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ranking->setSelectionMode(QAbstractItemView::NoSelection);
    m_ranking->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (std::size_t rank = 0; rank < deduction.ranked.size(); ++rank) {
        const int row = static_cast<int>(rank);
        const core::GridFit& fit = deduction.ranked[rank];
        m_ranking->setItem(row,
                           0,
                           // TRANSLATORS: %1 is a frame rate, « 25 ».
                           new QTableWidgetItem{substitute(
                               translate("%1 fps"), QString::fromStdString(nameOf(fit.rate)))});
        m_ranking->setItem(
            row,
            1,
            new QTableWidgetItem{QString::fromStdString(core::percentOf(fit.concentration))});
    }

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // **Only for a conversion that stands alone**: two that fit equally are named above, and
    // choosing between them is not for a button.
    if (repair.outcome == core::RepairOutcome::Found && repair.conversion.has_value()) {
        m_proposedInput = repair.conversion->input;
        m_proposedOutput = repair.conversion->output;
        m_convert =
            buttons->addButton(translate("Convert Frame Rate…"), QDialogButtonBox::AcceptRole);
        connect(m_convert, &QPushButton::clicked, this, [this] {
            m_input = m_proposedInput;
            m_output = m_proposedOutput;
            accept();
        });
    }

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
