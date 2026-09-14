#include <subedit/core/config/search_options.hpp>
#include <subedit/gui/search_dialog.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

namespace subedit::gui {

SearchDialog::SearchDialog(QWidget* parent)
    : QDialog(parent),
      m_pattern(new QLineEdit{this}),
      m_replacement(new QLineEdit{this}),
      m_regex(new QCheckBox{QStringLiteral("Regular expression"), this}),
      m_ignoreCase(new QCheckBox{QStringLiteral("Ignore case"), this}),
      m_status(new QLabel{this}),
      m_previous(new QPushButton{QStringLiteral("Find &Previous"), this}),
      m_next(new QPushButton{QStringLiteral("Find &Next"), this}),
      m_replace(new QPushButton{QStringLiteral("&Replace"), this}),
      m_replaceAll(new QPushButton{QStringLiteral("Replace &All"), this}) {
    setWindowTitle(QStringLiteral("Find and replace"));

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Find"), m_pattern);
    fields->addRow(QStringLiteral("Replace with"), m_replacement);

    auto* options = new QHBoxLayout;
    options->addWidget(m_regex);
    options->addWidget(m_ignoreCase);
    options->addStretch();

    auto* gestures = new QHBoxLayout;
    gestures->addWidget(m_previous);
    gestures->addWidget(m_next);
    gestures->addWidget(m_replace);
    gestures->addWidget(m_replaceAll);

    auto* close = new QDialogButtonBox{QDialogButtonBox::Close, this};
    connect(close, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // What was found, or not: a line and not a box, because the dialog stays
    // open and a box would have to be dismissed at every miss.
    m_status->setWordWrap(true);

    auto* stack = new QVBoxLayout{this};
    stack->addLayout(fields);
    stack->addLayout(options);
    stack->addLayout(gestures);
    stack->addWidget(m_status);
    stack->addWidget(close);

    // Enter finds the next match, which is what one presses after typing.
    m_next->setDefault(true);

    connect(m_previous, &QPushButton::clicked, this, &SearchDialog::findPreviousRequested);
    connect(m_next, &QPushButton::clicked, this, &SearchDialog::findNextRequested);
    connect(m_replace, &QPushButton::clicked, this, &SearchDialog::replaceRequested);
    connect(m_replaceAll, &QPushButton::clicked, this, &SearchDialog::replaceAllRequested);

    connect(m_pattern, &QLineEdit::textChanged, this, [this] {
        refreshButtons();
        emit searchChanged();
    });
    for (QCheckBox* option : {m_regex, m_ignoreCase})
        connect(option, &QCheckBox::toggled, this, &SearchDialog::searchChanged);

    // **Born on the defaults, and not on Qt's.** A check box starts unchecked,
    // and Gaupol's default is to ignore the case: a dialog made without being
    // told otherwise — the capture of the manual was one — showed the opposite
    // of what the preferences start from.
    setOptions(core::SearchOptions{});

    refreshButtons();
}

QString SearchDialog::pattern() const {
    return m_pattern->text();
}

QString SearchDialog::replacement() const {
    return m_replacement->text();
}

core::SearchOptions SearchDialog::options() const {
    return core::SearchOptions{.regex = m_regex->isChecked(),
                               .ignoreCase = m_ignoreCase->isChecked()};
}

void SearchDialog::setOptions(core::SearchOptions options) {
    m_regex->setChecked(options.regex);
    m_ignoreCase->setChecked(options.ignoreCase);
}

void SearchDialog::setStatus(const QString& message) {
    m_status->setText(message);
}

void SearchDialog::refreshButtons() {
    const bool something = !m_pattern->text().isEmpty();
    for (QPushButton* gesture : {m_previous, m_next, m_replace, m_replaceAll})
        gesture->setEnabled(something);
}

} // namespace subedit::gui
