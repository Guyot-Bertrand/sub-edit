#include <subedit/gui/disjoint_dialog.hpp>
#include <subedit/gui/messages.hpp>

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

DisjointDialog::DisjointDialog(const DisjointChoice& initial, QWidget* parent, bool repairing)
    : QDialog(parent),
      m_shortest(new QSpinBox{this}),
      m_lines(new QSpinBox{this}),
      m_dashes(new QCheckBox{translate("Put a dash before each subtitle's text"), this}) {
    setWindowTitle(repairing ? translate("Repair subtitles") : translate("Make ranges disjoint"));

    m_shortest->setRange(0, kLongestShortest);
    // TRANSLATORS: the unit after the number of a spin box, with its space: « 50 ms ».
    m_shortest->setSuffix(translate(" ms"));
    m_shortest->setValue(initial.shortestMs);
    m_shortest->setToolTip(translate("An interval shorter than this is taken over by a "
                                     "neighbour. Zero keeps every interval."));

    m_lines->setRange(0, kMostLinesAllowed);
    m_lines->setSpecialValueText(translate("No limit"));
    m_lines->setValue(initial.mostLines);
    m_lines->setToolTip(translate("A merged subtitle with more lines is shown in windows of "
                                  "this many lines, one after the other."));

    m_dashes->setChecked(initial.dashes);

    // Each paragraph whole, so that a translation is never assembled from pieces.
    auto* explanation = new QLabel{
        repairing ? translate("The subtitles that repeat each other are joined first, and the "
                              "rest put in order of their start. Overlapping subtitles are cut "
                              "where they overlap, and what shows together is merged line by "
                              "line, the earliest first. Nothing is shortened or lengthened. "
                              "Each change can be undone.")
                  : translate("Overlapping subtitles are cut where they overlap, and what shows "
                              "together is merged line by line, the earliest first. Nothing is "
                              "shortened or lengthened. Each change can be undone."),
        this};
    explanation->setWordWrap(true);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Shortest interval kept"), m_shortest);
    fields->addRow(translate("Most lines in a subtitle"), m_lines);
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
