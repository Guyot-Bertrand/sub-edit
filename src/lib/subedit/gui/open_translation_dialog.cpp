#include <subedit/core/edit/translation.hpp>
#include <subedit/gui/open_translation_dialog.hpp>

#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

namespace subedit::gui {

OpenTranslationDialog::OpenTranslationDialog(const QString& fileName, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Open Translation"));

    // **Built here, from named strings, one statement to a line.** Gcov
    // attributes to a widget built from a temporary and spread over several
    // lines the exit its exception would take, and counts that line as never
    // run — for a line that runs on every call.
    const QString question =
        QStringLiteral("How should the lines of %1 be matched to the subtitles?").arg(fileName);
    const QString byPosition =
        QStringLiteral("By position — each line goes to the subtitle it falls in");
    const QString byNumber = QStringLiteral("By number — the nth line goes to the nth subtitle");

    m_text = new QLabel{question, this};
    m_text->setWordWrap(true);
    m_position = new QRadioButton{byPosition, this};
    m_number = new QRadioButton{byNumber, this};
    m_position->setChecked(true);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Open | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_text);
    layout->addWidget(m_position);
    layout->addWidget(m_number);
    layout->addWidget(buttons);
}

core::TranslationMethod OpenTranslationDialog::method() const {
    return m_number->isChecked() ? core::TranslationMethod::Number
                                 : core::TranslationMethod::Position;
}

QString OpenTranslationDialog::text() const {
    return m_text->text();
}

} // namespace subedit::gui
