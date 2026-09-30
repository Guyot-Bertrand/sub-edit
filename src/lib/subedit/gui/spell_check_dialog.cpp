#include <subedit/core/text/spell_check_walk.hpp>
#include <subedit/gui/spell_check_dialog.hpp>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTextCursor>
#include <QVBoxLayout>

#include <string>
#include <vector>

namespace subedit::gui {

namespace {

/// A UTF-8 byte offset of `text`, as the UTF-16 offset a text edit counts in.
int utf16Offset(const std::string& text, std::size_t bytes) {
    return static_cast<int>(QString::fromUtf8(text.data(), static_cast<qsizetype>(bytes)).size());
}

} // namespace

SpellCheckDialog::SpellCheckDialog(core::SpellCheckWalk& walk, QWidget* parent)
    : QDialog(parent),
      m_walk(walk),
      m_text(new QPlainTextEdit{this}),
      m_replacement(new QLineEdit{this}),
      m_suggestions(new QListWidget{this}),
      m_grid(new QWidget{this}),
      m_add(new QPushButton{QStringLiteral("Add"), m_grid}),
      m_ignore(new QPushButton{QStringLiteral("Ignore"), m_grid}),
      m_ignoreAll(new QPushButton{QStringLiteral("Ignore All"), m_grid}),
      m_replace(new QPushButton{QStringLiteral("Replace"), m_grid}),
      m_replaceAll(new QPushButton{QStringLiteral("Replace All"), m_grid}),
      m_joinBack(new QPushButton{QStringLiteral("Join with Previous"), m_grid}),
      m_joinForward(new QPushButton{QStringLiteral("Join with Next"), m_grid}),
      m_save(new QPushButton{QStringLiteral("Save and Resume"), m_grid}),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Close, this}) {
    setWindowTitle(QStringLiteral("Check Spelling"));

    auto* buttons = new QGridLayout{m_grid};
    buttons->setContentsMargins(0, 0, 0, 0);
    int row = 0;
    for (QPushButton* button :
         {m_add, m_ignore, m_ignoreAll, m_replace, m_replaceAll, m_joinBack, m_joinForward, m_save})
        buttons->addWidget(button, row++, 0);

    auto* middle = new QGridLayout{};
    middle->addWidget(new QLabel{QStringLiteral("Replace with:"), this}, 0, 0);
    middle->addWidget(m_replacement, 1, 0);
    middle->addWidget(new QLabel{QStringLiteral("Suggestions:"), this}, 2, 0);
    middle->addWidget(m_suggestions, 3, 0);
    middle->addWidget(m_grid, 0, 1, 4, 1);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_text);
    layout->addLayout(middle);
    layout->addWidget(m_buttons);

    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_text, &QPlainTextEdit::textChanged, this, [this] { onTextEdited(); });
    connect(m_replacement, &QLineEdit::textEdited, this, [this] { onReplacementEdited(); });
    connect(m_suggestions, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* item) {
        if (item != nullptr)
            setReplacementText(item->text());
    });

    connect(m_add, &QPushButton::clicked, this, [this] {
        m_walk.add();
        proceed();
    });
    connect(m_ignore, &QPushButton::clicked, this, [this] {
        m_walk.ignore();
        proceed();
    });
    connect(m_ignoreAll, &QPushButton::clicked, this, [this] {
        m_walk.ignoreAll();
        proceed();
    });
    connect(m_replace, &QPushButton::clicked, this, [this] {
        m_walk.replace(m_replacement->text().toStdString());
        proceed();
    });
    connect(m_replaceAll, &QPushButton::clicked, this, [this] {
        m_walk.replaceAll(m_replacement->text().toStdString());
        proceed();
    });
    connect(m_joinBack, &QPushButton::clicked, this, [this] {
        m_walk.joinWithPrevious();
        proceed();
    });
    connect(m_joinForward, &QPushButton::clicked, this, [this] {
        m_walk.joinWithNext();
        proceed();
    });
    connect(m_save, &QPushButton::clicked, this, [this] {
        m_walk.resumeWithText(m_text->toPlainText().toStdString());
        proceed();
    });

    for (QPushButton* button : m_grid->findChildren<QPushButton*>())
        button->setEnabled(false);
}

SpellCheckDialog::~SpellCheckDialog() = default;

void SpellCheckDialog::start() {
    proceed();
}

void SpellCheckDialog::proceed() {
    std::optional<core::SpellStop> next = m_walk.advance();
    if (!next.has_value()) {
        finish();
        return;
    }
    m_stop = std::make_unique<core::SpellStop>(*std::move(next));
    display(*m_stop);
    emit stopped(*m_stop);
}

void SpellCheckDialog::display(const core::SpellStop& stop) {
    m_loading = true;
    m_text->setPlainText(QString::fromStdString(stop.text));
    QTextCursor cursor{m_text->document()};
    cursor.setPosition(utf16Offset(stop.text, stop.pos));
    cursor.setPosition(utf16Offset(stop.text, stop.endPos), QTextCursor::KeepAnchor);
    m_text->setTextCursor(cursor);
    m_text->setFocus();
    m_loading = false;

    m_save->setEnabled(false);
    m_add->setEnabled(true);
    m_ignore->setEnabled(true);
    m_ignoreAll->setEnabled(true);
    m_joinBack->setEnabled(m_walk.spaceBefore());
    m_joinForward->setEnabled(m_walk.spaceAfter());

    setReplacementText({});
    populateSuggestions(m_walk.suggest(), true);
}

void SpellCheckDialog::finish() {
    m_done = true;
    m_stop.reset();

    m_loading = true;
    m_text->clear();
    m_loading = false;
    setReplacementText({});
    populateSuggestions({}, false);
    m_grid->setEnabled(false);
    for (QPushButton* button : m_grid->findChildren<QPushButton*>())
        button->setEnabled(false);
    emit finishedWalking();
}

void SpellCheckDialog::populateSuggestions(const std::vector<std::string>& suggestions,
                                           bool select) {
    {
        const QSignalBlocker blocker{m_suggestions};
        m_suggestions->clear();
        for (const std::string& suggestion : suggestions)
            m_suggestions->addItem(QString::fromStdString(suggestion));
    }
    // Gaupol selects the first suggestion, which fills the replacement.
    if (select && m_suggestions->count() > 0)
        m_suggestions->setCurrentRow(0);
}

void SpellCheckDialog::setReplacementText(const QString& text) {
    const QSignalBlocker blocker{m_replacement};
    m_replacement->setText(text);
    updateReplaceButtons();
}

void SpellCheckDialog::updateReplaceButtons() {
    const bool usable =
        m_stop != nullptr && !m_replacement->text().isEmpty() && !m_save->isEnabled();
    m_replace->setEnabled(usable);
    m_replaceAll->setEnabled(usable);
}

void SpellCheckDialog::onReplacementEdited() {
    const QString word = m_replacement->text();
    populateSuggestions(word.isEmpty() ? std::vector<std::string>{}
                                       : m_walk.checker().suggest(word.toStdString()),
                        false);
    updateReplaceButtons();
}

void SpellCheckDialog::onTextEdited() {
    if (m_loading || m_stop == nullptr)
        return;
    // Typing in the text: only saving it is allowed, as in Gaupol.
    m_save->setEnabled(true);
    for (QPushButton* button :
         {m_add, m_ignoreAll, m_ignore, m_joinBack, m_joinForward, m_replace, m_replaceAll})
        button->setEnabled(false);
}

} // namespace subedit::gui
