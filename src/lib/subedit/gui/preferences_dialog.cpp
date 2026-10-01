#include <subedit/core/config/theme.hpp>
#include <subedit/core/wording/settings.hpp>
#include <subedit/gui/preferences_dialog.hpp>

#include <QCheckBox>
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

/// The three values, in the order one meets them: the one that does nothing
/// first, since it is the default.
constexpr std::array<core::Theme, 3> kThemes = {
    core::Theme::System, core::Theme::Light, core::Theme::Dark};

/// The two units, in the order Gaupol lists them: ems first, the default.
constexpr std::array<core::LengthUnit, 2> kUnits = {core::LengthUnit::Ems,
                                                    core::LengthUnit::Characters};

[[nodiscard]] QString labelOf(core::LengthUnit unit) {
    return unit == core::LengthUnit::Ems ? QStringLiteral("Ems") : QStringLiteral("Characters");
}

} // namespace

PreferencesDialog::PreferencesDialog(core::Theme theme,
                                     const core::EditorSettings& editor,
                                     QWidget* parent)
    : QDialog(parent),
      m_theme(new QComboBox{this}),
      m_lengthUnit(new QComboBox{this}),
      m_showInCells(new QCheckBox{QStringLiteral("Show line lengths in cells"), this}),
      m_showInEditor(new QCheckBox{QStringLiteral("Show line lengths in the editor"), this}) {
    setWindowTitle(QStringLiteral("Preferences"));

    for (const core::Theme one : kThemes)
        m_theme->addItem(QString::fromUtf8(core::nameOf(one)));

    m_theme->setCurrentIndex(static_cast<int>(theme));

    for (const core::LengthUnit one : kUnits)
        m_lengthUnit->addItem(labelOf(one));
    m_lengthUnit->setCurrentIndex(editor.lengthUnit == core::LengthUnit::Ems ? 0 : 1);
    m_showInCells->setChecked(editor.showLengthsInCells);
    m_showInEditor->setChecked(editor.showLengthsInEditor);
    connect(m_showInCells, &QCheckBox::toggled, this, [this] { refreshLengthUnitState(); });
    connect(m_showInEditor, &QCheckBox::toggled, this, [this] { refreshLengthUnitState(); });
    refreshLengthUnitState();

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Theme"), m_theme);
    fields->addRow(QStringLiteral("Length unit"), m_lengthUnit);
    fields->addRow(m_showInCells);
    fields->addRow(m_showInEditor);

    // What "system" does, said where it is read: without this line, a reader
    // who picks "System" and sees nothing change believes it broken.
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

void PreferencesDialog::refreshLengthUnitState() {
    m_lengthUnit->setEnabled(m_showInCells->isChecked() || m_showInEditor->isChecked());
}

core::EditorSettings PreferencesDialog::editor() const {
    return {.lengthUnit = kUnits.at(static_cast<std::size_t>(m_lengthUnit->currentIndex())),
            .showLengthsInCells = m_showInCells->isChecked(),
            .showLengthsInEditor = m_showInEditor->isChecked()};
}

core::Theme PreferencesDialog::theme() const {
    return kThemes.at(static_cast<std::size_t>(m_theme->currentIndex()));
}

} // namespace subedit::gui
