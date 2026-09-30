#include <subedit/core/text/spell_dictionary.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/join_split_page.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

namespace subedit::gui {

SpellCheckSettingsDialog::SpellCheckSettingsDialog(const core::SpellProvider* provider,
                                                   bool selectionAvailable,
                                                   bool translationAvailable,
                                                   QWidget* parent)
    : QDialog(parent),
      m_provider(provider),
      m_language(new QComboBox{this}),
      m_reason(new QLabel{this}),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this}) {
    setWindowTitle(QStringLiteral("Spell-Check Settings"));

    auto* targetGroup = new QGroupBox{QStringLiteral("Target"), this};
    m_selection = new QRadioButton{QStringLiteral("Selection"), targetGroup};
    m_selection->setEnabled(selectionAvailable);
    m_currentProject = new QRadioButton{QStringLiteral("Current Project"), targetGroup};
    m_currentProject->setChecked(true);
    m_allProjects = new QRadioButton{QStringLiteral("All Open Projects"), targetGroup};
    auto* targetLayout = new QVBoxLayout{targetGroup};
    targetLayout->addWidget(m_selection);
    targetLayout->addWidget(m_currentProject);
    targetLayout->addWidget(m_allProjects);

    auto* documentGroup = new QGroupBox{QStringLiteral("Document"), this};
    m_text = new QRadioButton{QStringLiteral("Text"), documentGroup};
    m_text->setChecked(true);
    m_translation = new QRadioButton{QStringLiteral("Translation"), documentGroup};
    m_translation->setEnabled(translationAvailable);
    auto* documentLayout = new QVBoxLayout{documentGroup};
    documentLayout->addWidget(m_text);
    documentLayout->addWidget(m_translation);

    auto* form = new QFormLayout{};
    form->addRow(QStringLiteral("Language:"), m_language);

    auto* layout = new QVBoxLayout{this};
    layout->addLayout(form);
    layout->addWidget(m_reason);
    layout->addWidget(targetGroup);
    layout->addWidget(documentGroup);
    layout->addWidget(m_buttons);

    connect(m_language, &QComboBox::currentTextChanged, this, [this] { refresh(); });
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    apply(core::SpellCheckSettings{});
}

void SpellCheckSettingsDialog::apply(const core::SpellCheckSettings& settings) {
    populateSpellLanguages(*m_language, m_provider, settings.language);

    using Target = core::SpellCheckTarget;
    if (settings.target == Target::Selection && m_selection->isEnabled())
        m_selection->setChecked(true);
    else if (settings.target == Target::AllProjects)
        m_allProjects->setChecked(true);
    else
        m_currentProject->setChecked(true);

    if (settings.document == core::SpellCheckDocument::Translation && m_translation->isEnabled())
        m_translation->setChecked(true);
    else
        m_text->setChecked(true);
    refresh();
}

core::SpellCheckSettings SpellCheckSettingsDialog::settings() const {
    return {.language = language(), .target = target(), .document = document()};
}

std::string SpellCheckSettingsDialog::language() const {
    return m_language->currentText().toStdString();
}

core::SpellCheckTarget SpellCheckSettingsDialog::target() const {
    if (m_selection->isChecked())
        return core::SpellCheckTarget::Selection;
    if (m_allProjects->isChecked())
        return core::SpellCheckTarget::AllProjects;
    return core::SpellCheckTarget::CurrentProject;
}

core::SpellCheckDocument SpellCheckSettingsDialog::document() const {
    return m_translation->isChecked() ? core::SpellCheckDocument::Translation
                                      : core::SpellCheckDocument::Main;
}

QString SpellCheckSettingsDialog::unavailableReason() const {
    return m_available ? QString{} : m_reason->text();
}

void SpellCheckSettingsDialog::refresh() {
    const std::string chosen = language();
    m_available = hasSpellDictionary(m_provider, chosen);
    m_reason->setText(m_available ? QString{}
                                  : QString::fromStdString(core::noDictionaryFor(chosen)));
}

} // namespace subedit::gui
