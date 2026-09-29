// The script/language/country widget of a task page — issue #505, task 7.

#include <subedit/gui/pattern_code.hpp>
#include <subedit/gui/pattern_code_selector.hpp>

#include <QComboBox>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QString>

namespace subedit::gui {

PatternCodeSelector::PatternCodeSelector(const core::PatternCatalogue& catalogue,
                                         core::PatternKind kind,
                                         QWidget* parent)
    : QWidget(parent),
      m_catalogue(&catalogue),
      m_kind(kind),
      m_script(new QComboBox{this}),
      m_language(new QComboBox{this}),
      m_country(new QComboBox{this}) {
    auto* layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_script);
    layout->addWidget(m_language);
    layout->addWidget(m_country);

    m_language->addItem(QStringLiteral("(every language)"), QString{});
    m_country->addItem(QStringLiteral("(every country)"), QString{});
    for (const std::string& script : scriptsOf(*m_catalogue, m_kind))
        m_script->addItem(QString::fromStdString(script), QString::fromStdString(script));

    connect(m_script, &QComboBox::currentIndexChanged, this, [this] {
        refreshLanguages();
        emit codeChanged();
    });
    connect(m_language, &QComboBox::currentIndexChanged, this, [this] {
        refreshCountries();
        emit codeChanged();
    });
    connect(m_country, &QComboBox::currentIndexChanged, this, &PatternCodeSelector::codeChanged);

    refreshLanguages();
}

void PatternCodeSelector::refreshLanguages() {
    const QString script = m_script->currentData().toString();
    const QSignalBlocker block{m_language};
    while (m_language->count() > 1)
        m_language->removeItem(1);
    for (const std::string& language : languagesOf(*m_catalogue, m_kind, script.toStdString()))
        m_language->addItem(QString::fromStdString(language), QString::fromStdString(language));
    m_language->setCurrentIndex(0);
    refreshCountries();
}

void PatternCodeSelector::refreshCountries() {
    const QString script = m_script->currentData().toString();
    const QString language = m_language->currentData().toString();
    const QSignalBlocker block{m_country};
    while (m_country->count() > 1)
        m_country->removeItem(1);
    for (const std::string& country :
         countriesOf(*m_catalogue, m_kind, script.toStdString(), language.toStdString()))
        m_country->addItem(QString::fromStdString(country), QString::fromStdString(country));
    m_country->setCurrentIndex(0);
}

std::string PatternCodeSelector::code() const {
    return joinCode(PatternCodeParts{
        .script = m_script->currentData().toString().toStdString(),
        .language = m_language->currentData().toString().toStdString(),
        .country = m_country->currentData().toString().toStdString(),
    });
}

void PatternCodeSelector::setCode(std::string_view code) {
    // Each `setCurrentIndex` below would otherwise fire the connected
    // `currentIndexChanged` lambda on its own, emitting `codeChanged()` up to
    // three times with `code()` reading a transient, not-yet-cascaded value
    // in between. Blocking the three combos for the whole cascade keeps the
    // *contents* refreshed at each step — `refreshLanguages()`/
    // `refreshCountries()` still run unguarded, same as always — while
    // deferring the *signal* to a single emission once everything has
    // settled.
    const PatternCodeParts parts = splitCode(code);
    {
        const QSignalBlocker blockScript{m_script};
        const int scriptIndex = m_script->findData(QString::fromStdString(parts.script));
        m_script->setCurrentIndex(scriptIndex >= 0 ? scriptIndex : 0);
        refreshLanguages();

        const QSignalBlocker blockLanguage{m_language};
        const int languageIndex = m_language->findData(QString::fromStdString(parts.language));
        m_language->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);
        refreshCountries();

        const QSignalBlocker blockCountry{m_country};
        const int countryIndex = m_country->findData(QString::fromStdString(parts.country));
        m_country->setCurrentIndex(countryIndex >= 0 ? countryIndex : 0);
    }
    emit codeChanged();
}

} // namespace subedit::gui
