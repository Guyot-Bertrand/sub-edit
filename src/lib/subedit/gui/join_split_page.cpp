#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_dictionary.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/join_split_page.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QVBoxLayout>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::gui {

std::string spellLanguageFor(const std::vector<std::string>& offered,
                             std::string_view systemLocale) {
    std::string system{systemLocale};
    if (!core::isValidSpellLanguage(system))
        system = "en";
    if (std::ranges::find(offered, system) != offered.end())
        return system;
    const std::string language = system.substr(0, system.find('_'));
    const auto same = std::ranges::find_if(offered, [&language](const std::string& code) {
        return code.substr(0, code.find('_')) == language;
    });
    return same != offered.end() ? *same : system;
}

JoinSplitPage::JoinSplitPage(const core::SpellProvider* provider, QWidget* parent)
    : QWizardPage(parent),
      m_provider(provider),
      m_language(new QComboBox{this}),
      m_join(new QCheckBox{QStringLiteral("Join words"), this}),
      m_split(new QCheckBox{QStringLiteral("Split words"), this}),
      m_reason(new QLabel{this}) {
    setTitle(QStringLiteral("Join or Split Words"));

    auto* form = new QFormLayout{};
    form->addRow(QStringLiteral("Language:"), m_language);

    auto* layout = new QVBoxLayout{this};
    layout->addLayout(form);
    layout->addWidget(m_join);
    layout->addWidget(m_split);
    layout->addWidget(m_reason);
    layout->addStretch();

    connect(m_language, &QComboBox::currentTextChanged, this, [this] { refresh(); });
}

void JoinSplitPage::applySettings(const core::CorrectionSettings& settings) {
    std::vector<std::string> offered;
    if (m_provider != nullptr)
        offered = core::availableSpellLanguages(*m_provider);
    const std::string wanted =
        settings.spellLanguage.empty()
            ? spellLanguageFor(offered, QLocale::system().name().toStdString())
            : settings.spellLanguage;
    if (std::ranges::find(offered, wanted) == offered.end())
        offered.push_back(wanted); // shown even without a dictionary: that is what is said

    m_language->clear();
    for (const std::string& code : offered)
        m_language->addItem(QString::fromStdString(code));
    m_language->setCurrentText(QString::fromStdString(wanted));

    m_join->setChecked(settings.joinWords);
    m_split->setChecked(settings.splitWords);
    refresh();
}

std::string JoinSplitPage::language() const {
    return m_language->currentText().toStdString();
}

bool JoinSplitPage::join() const {
    return m_join->isChecked();
}

bool JoinSplitPage::split() const {
    return m_split->isChecked();
}

QString JoinSplitPage::unavailableReason() const {
    return m_available ? QString{} : m_reason->text();
}

void JoinSplitPage::refresh() {
    const std::string chosen = language();
    m_available = m_provider != nullptr && !chosen.empty() && m_provider->open(chosen) != nullptr;
    m_join->setEnabled(m_available);
    m_split->setEnabled(m_available);
    m_reason->setText(m_available ? QString{}
                                  : QString::fromStdString(core::noDictionaryFor(chosen)));
}

} // namespace subedit::gui
