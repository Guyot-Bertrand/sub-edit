#include <subedit/core/config/theme.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/preferences_dialog.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

#include <array>
#include <cstddef>

namespace subedit::gui {

namespace {

/// Les trois valeurs, dans l'ordre où on les rencontre : celle qui ne fait rien
/// d'abord, puisque c'est le défaut.
constexpr std::array<core::Theme, 3> kThemes = {
    core::Theme::System, core::Theme::Light, core::Theme::Dark};

} // namespace

PreferencesDialog::PreferencesDialog(core::Theme theme, QWidget* parent)
    : QDialog(parent), m_theme(new QComboBox{this}) {
    setWindowTitle(QStringLiteral("Preferences"));

    for (const core::Theme one : kThemes)
        m_theme->addItem(QString::fromUtf8(core::nameOf(one)));

    m_theme->setCurrentIndex(static_cast<int>(theme));

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Theme"), m_theme);

    // Ce que « système » fait, dit là où on le lit : sans cette ligne, un
    // lecteur qui choisit « System » et ne voit rien changer croit à une panne.
    auto* explanation = new QLabel{
        QStringLiteral("%1 %2.").arg(QString::fromUtf8(core::nameOf(core::Theme::System)),
                                     QString::fromUtf8(core::systemThemeExplained())),
        this};
    explanation->setWordWrap(true);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addLayout(fields);
    stack->addWidget(explanation);
    stack->addWidget(buttons);
}

core::Theme PreferencesDialog::theme() const {
    return kThemes.at(static_cast<std::size_t>(m_theme->currentIndex()));
}

} // namespace subedit::gui
