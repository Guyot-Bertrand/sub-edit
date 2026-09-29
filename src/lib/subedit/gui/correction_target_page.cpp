#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/correction_target_page.hpp>

#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

namespace subedit::gui {

CorrectionTargetPage::CorrectionTargetPage(bool selectionAvailable,
                                           bool translationAvailable,
                                           QWidget* parent)
    : QWizardPage(parent) {
    setTitle(QStringLiteral("Tasks and Target"));

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

    auto* taskGroup = new QGroupBox{QStringLiteral("Tasks"), this};
    m_mentions = new QCheckBox{QStringLiteral("Mentions"), taskGroup};
    m_commonErrors = new QCheckBox{QStringLiteral("Common Errors"), taskGroup};
    m_capitalization = new QCheckBox{QStringLiteral("Capitalization"), taskGroup};
    m_lineBreak = new QCheckBox{QStringLiteral("Line Break"), taskGroup};
    auto* taskLayout = new QVBoxLayout{taskGroup};
    taskLayout->addWidget(m_mentions);
    taskLayout->addWidget(m_commonErrors);
    taskLayout->addWidget(m_capitalization);
    taskLayout->addWidget(m_lineBreak);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(targetGroup);
    layout->addWidget(documentGroup);
    layout->addWidget(taskGroup);
}

CorrectionScope CorrectionTargetPage::scope() const {
    if (m_selection->isChecked())
        return CorrectionScope::Selection;
    if (m_allProjects->isChecked())
        return CorrectionScope::AllProjects;
    return CorrectionScope::CurrentProject;
}

core::Document CorrectionTargetPage::document() const {
    return m_translation->isChecked() ? core::Document::Translation : core::Document::Main;
}

bool CorrectionTargetPage::taskChecked(core::CorrectionTask task) const {
    switch (task) {
    case core::CorrectionTask::Mentions:
        return m_mentions->isChecked();
    case core::CorrectionTask::CommonErrors:
        return m_commonErrors->isChecked();
    case core::CorrectionTask::Capitalization:
        return m_capitalization->isChecked();
    case core::CorrectionTask::LineBreak:
        return m_lineBreak->isChecked();
    }
    return false; // unreachable: every enumerator is handled above
}

void CorrectionTargetPage::setTaskChecked(core::CorrectionTask task, bool checked) {
    switch (task) {
    case core::CorrectionTask::Mentions:
        m_mentions->setChecked(checked);
        return;
    case core::CorrectionTask::CommonErrors:
        m_commonErrors->setChecked(checked);
        return;
    case core::CorrectionTask::Capitalization:
        m_capitalization->setChecked(checked);
        return;
    case core::CorrectionTask::LineBreak:
        m_lineBreak->setChecked(checked);
        return;
    }
}

} // namespace subedit::gui
