#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/pattern_code_selector.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace subedit::gui {

CorrectionTaskPage::CorrectionTaskPage(const QString& title,
                                       const core::PatternCatalogue& catalogue,
                                       core::PatternKind kind,
                                       QWidget* parent)
    : QWizardPage(parent),
      m_catalogue(&catalogue),
      m_kind(kind),
      m_selector(new PatternCodeSelector{catalogue, kind, this}),
      m_extraLayout(new QVBoxLayout{}),
      m_list(new PatternList{catalogue, kind, this}) {
    setTitle(title);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_selector);
    layout->addLayout(m_extraLayout);
    layout->addWidget(m_list);

    connect(m_selector, &PatternCodeSelector::codeChanged, this, [this] {
        m_list->setCode(code(), m_settings);
    });
}

std::string CorrectionTaskPage::code() const {
    return m_selector->code();
}

std::vector<core::PatternActivation> CorrectionTaskPage::activations() const {
    return m_list->activations();
}

void CorrectionTaskPage::applyBase(const std::string& code,
                                   const core::CorrectionSettings& settings) {
    m_settings = settings;
    m_selector->setCode(code);
    m_list->setCode(code, settings);
}

MentionsPage::MentionsPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(
          QStringLiteral("Mentions"), catalogue, core::PatternKind::HearingImpaired, parent),
      m_brackets(new QCheckBox{QStringLiteral("Sound in brackets"), this}),
      m_parentheses(new QCheckBox{QStringLiteral("Sound in parentheses"), this}) {
    extraLayout()->addWidget(m_brackets);
    extraLayout()->addWidget(m_parentheses);
}

void MentionsPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.mentions.code, settings);
    m_brackets->setChecked(settings.soundInBrackets);
    m_parentheses->setChecked(settings.soundInParentheses);
}

bool MentionsPage::soundInBrackets() const {
    return m_brackets->isChecked();
}

bool MentionsPage::soundInParentheses() const {
    return m_parentheses->isChecked();
}

CommonErrorsPage::CommonErrorsPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(
          QStringLiteral("Common Errors"), catalogue, core::PatternKind::CommonError, parent),
      m_human(new QCheckBox{QStringLiteral("Human"), this}),
      m_ocr(new QCheckBox{QStringLiteral("OCR"), this}) {
    extraLayout()->addWidget(m_human);
    extraLayout()->addWidget(m_ocr);
}

void CommonErrorsPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.commonErrors.code, settings);
    m_human->setChecked(settings.human);
    m_ocr->setChecked(settings.ocr);
}

bool CommonErrorsPage::human() const {
    return m_human->isChecked();
}

bool CommonErrorsPage::ocr() const {
    return m_ocr->isChecked();
}

CapitalizationPage::CapitalizationPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(
          QStringLiteral("Capitalization"), catalogue, core::PatternKind::Capitalization, parent) {}

void CapitalizationPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.capitalization.code, settings);
}

LineBreakPage::LineBreakPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(
          QStringLiteral("Line Break"), catalogue, core::PatternKind::LineBreak, parent),
      m_maxLength(new QDoubleSpinBox{this}),
      m_maxLines(new QSpinBox{this}),
      m_unit(new QComboBox{this}) {
    m_maxLength->setRange(1.0, 1000.0);
    m_maxLines->setRange(1, 100);
    m_unit->addItem(QStringLiteral("Characters"));
    m_unit->addItem(QStringLiteral("Ems"));

    auto* form = new QFormLayout{};
    form->addRow(QStringLiteral("Maximum length:"), m_maxLength);
    form->addRow(QStringLiteral("Maximum lines:"), m_maxLines);
    form->addRow(QStringLiteral("Unit:"), m_unit);
    extraLayout()->addLayout(form);
}

void LineBreakPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.lineBreak.code, settings);
    m_maxLength->setValue(settings.lineBreakMaxLength);
    m_maxLines->setValue(settings.lineBreakMaxLines);
}

double LineBreakPage::maxLength() const {
    return m_maxLength->value();
}

int LineBreakPage::maxLines() const {
    return m_maxLines->value();
}

bool LineBreakPage::useEms() const {
    return m_unit->currentIndex() == 1;
}

} // namespace subedit::gui
