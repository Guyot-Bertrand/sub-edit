#include <subedit/core/edit/command_preview.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QAbstractScrollArea>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cstddef>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// How a pair of positions reads in a preview.
[[nodiscard]] QString spanOf(const core::Subtitle& subtitle) {
    return QString::fromStdString(subtitle.start.format(core::DecimalMark::Comma)) +
           QStringLiteral(" → ") +
           QString::fromStdString(subtitle.end.format(core::DecimalMark::Comma));
}

/// The first line of a text: a preview is a glance, not a reading.
[[nodiscard]] QString firstLineOf(const std::string& text) {
    return QString::fromStdString(text).section(QLatin1Char('\n'), 0, 0);
}

} // namespace

OperationPreview describedPreview(const core::CommandPreview& preview) {
    OperationPreview described{.rows = {}, .changed = preview.changed};
    for (const core::PreviewedChange& change : preview.shown) {
        described.rows.push_back(PreviewRow{.number = QString::number(change.index.number()),
                                            .before = spanOf(change.before),
                                            .after = spanOf(change.after),
                                            .text = firstLineOf(change.before.mainText)});
    }
    return described;
}

OperationDialog::OperationDialog(OperationScope scope, QWidget* parent)
    : QDialog(parent),
      m_scope(scope),
      m_fields(new QFormLayout),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this}) {
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool OperationDialog::wholeProject() const {
    return m_project == nullptr || m_project->isChecked();
}

void OperationDialog::chooseWholeProject(bool whole) {
    if (m_project == nullptr)
        return;
    m_project->setChecked(whole);
    m_selection->setChecked(!whole);
}

QString OperationDialog::targetLabel() const {
    const bool selectedOnly = m_scope.offersChoice() && !wholeProject();
    const std::size_t count = selectedOnly ? m_scope.selected : m_scope.whole;
    return QString::fromStdString(core::countOf(count, core::Counted::Subtitle));
}

void OperationDialog::revalidate() {
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(isComplete());
}

void OperationDialog::offerPreview(PreviewProvider provider) {
    auto* button = new QPushButton{translate("Preview changes…"), this};
    button->setObjectName(QStringLiteral("preview-button"));
    connect(button, &QPushButton::clicked, this, [this, provider = std::move(provider)] {
        showPreview(provider());
    });

    // Above the buttons of the box, below what says what is touched.
    m_stack->insertWidget(m_stack->indexOf(m_buttons), button);
}

void OperationDialog::showPreview(const OperationPreview& preview) {

    auto* box = new QDialog{this};
    box->setObjectName(QStringLiteral("preview"));
    box->setWindowTitle(translate("Preview"));
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setSizeGripEnabled(true);

    auto* stack = new QVBoxLayout{box};
    const QString changed = preview.changed == 0
                                ? translate("Nothing would change.")
                                : substitute(translatePlural("%1 subtitle would change.",
                                                             "%1 subtitles would change.",
                                                             preview.changed),
                                             QString::number(preview.changed));
    stack->addWidget(new QLabel{changed, box});

    auto* table = new QTableWidget{static_cast<int>(preview.rows.size()), 4, box};
    table->setHorizontalHeaderLabels(QStringList{
        QStringLiteral("#"), translate("Before"), translate("After"), translate("Text")});
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    int row = 0;
    for (const PreviewRow& line : preview.rows) {
        table->setItem(row, 0, new QTableWidgetItem{line.number});
        table->setItem(row, 1, new QTableWidgetItem{line.before});
        table->setItem(row, 2, new QTableWidgetItem{line.after});
        table->setItem(row, 3, new QTableWidgetItem{line.text});
        ++row;
    }
    table->resizeColumnsToContents();
    // **Opens as wide and as tall as what it lists**, rather than at the size of
    // an empty table, where only the start of the first column showed and the
    // rest had to be scrolled to. The last column takes what the window gives
    // it, and the window stays resizable.
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setMinimumWidth(table->horizontalHeader()->length() + (2 * table->frameWidth()));
    stack->addWidget(table);

    auto* close = new QDialogButtonBox{QDialogButtonBox::Close, box};
    connect(close, &QDialogButtonBox::rejected, box, &QDialog::close);
    stack->addWidget(close);

    box->open();
}

void OperationDialog::finish() {
    m_stack = new QVBoxLayout{this};
    QVBoxLayout* stack = m_stack;
    stack->addLayout(m_fields);

    if (m_scope.offersChoice()) {
        // Some of the rows are selected, not all: two targets are possible, and
        // the user says which. The selection is the default, as it was before
        // the choice existed.
        // TRANSLATORS: %1 is a count of subtitles, « 3 subtitles ».
        const QString selectedText = substitute(
            translate("Selected: %1"),
            QString::fromStdString(core::countOf(m_scope.selected, core::Counted::Subtitle)));
        // TRANSLATORS: %1 is a count of subtitles, « 120 subtitles ».
        const QString wholeText = substitute(
            translate("Whole project: %1"),
            QString::fromStdString(core::countOf(m_scope.whole, core::Counted::Subtitle)));
        m_selection = new QRadioButton{selectedText, this};
        m_project = new QRadioButton{wholeText, this};
        m_selection->setChecked(true);
        stack->addWidget(m_selection);
        stack->addWidget(m_project);
    } else {
        // TRANSLATORS: %1 is a count of subtitles, « 120 subtitles ».
        stack->addWidget(new QLabel{substitute(translate("Applies to: %1"), targetLabel()), this});
    }

    stack->addWidget(m_buttons);

    revalidate();
}

} // namespace subedit::gui
