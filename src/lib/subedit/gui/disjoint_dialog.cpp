#include <subedit/gui/disjoint_dialog.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

namespace subedit::gui {

namespace {

constexpr int kLongestShortest = 10000;
constexpr int kMostLinesAllowed = 10;

} // namespace

DisjointDialog::DisjointDialog(const DisjointChoice& initial, QWidget* parent)
    : QDialog(parent),
      m_shortest(new QSpinBox{this}),
      m_lines(new QSpinBox{this}),
      m_dashes(new QCheckBox{QStringLiteral("Put a dash before each subtitle's text"), this}) {
    setWindowTitle(QStringLiteral("Make ranges disjoint"));

    m_shortest->setRange(0, kLongestShortest);
    m_shortest->setSuffix(QStringLiteral(" ms"));
    m_shortest->setValue(initial.shortestMs);
    m_shortest->setToolTip(QStringLiteral("An interval shorter than this is taken over by a "
                                          "neighbour. Zero keeps every interval."));

    m_lines->setRange(0, kMostLinesAllowed);
    m_lines->setSpecialValueText(QStringLiteral("No limit"));
    m_lines->setValue(initial.mostLines);
    m_lines->setToolTip(QStringLiteral("A merged subtitle with more lines is shown in windows of "
                                       "this many lines, one after the other."));

    m_dashes->setChecked(initial.dashes);

    auto* explanation = new QLabel{
        QStringLiteral("Overlapping subtitles are cut where they overlap, and what shows "
                       "together is merged line by line, the earliest first. Nothing is "
                       "shortened or lengthened. The change can be undone."),
        this};
    explanation->setWordWrap(true);

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Shortest interval kept"), m_shortest);
    fields->addRow(QStringLiteral("Most lines in a subtitle"), m_lines);
    fields->addRow(QString{}, m_dashes);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(explanation);
    stack->addLayout(fields);
    stack->addWidget(buttons);
}

DisjointChoice DisjointDialog::choice() const {
    return DisjointChoice{.shortestMs = m_shortest->value(),
                          .mostLines = m_lines->value(),
                          .dashes = m_dashes->isChecked()};
}

} // namespace subedit::gui
