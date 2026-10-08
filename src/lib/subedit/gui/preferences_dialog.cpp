#include <subedit/core/config/theme.hpp>
#include <subedit/core/wording/settings.hpp>
#include <subedit/gui/preferences_dialog.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

#include <array>
#include <cmath>
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

/// A second, in milliseconds, and the step of the lead-in's field in seconds.
constexpr double kMillisecondsPerSecond = 1000.0;
constexpr double kContextStepSeconds = 0.5;

} // namespace

PreferencesDialog::PreferencesDialog(core::Theme theme,
                                     const core::EditorSettings& editor,
                                     const core::VideoSettings& video,
                                     QWidget* parent)
    : QDialog(parent),
      m_theme(new QComboBox{this}),
      m_lengthUnit(new QComboBox{this}),
      m_showInCells(new QCheckBox{QStringLiteral("Show line lengths in cells"), this}),
      m_showInEditor(new QCheckBox{QStringLiteral("Show line lengths in the editor"), this}),
      m_seekLength(new QSpinBox{this}),
      m_contextLength(new QDoubleSpinBox{this}),
      m_stepFrames(new QSpinBox{this}),
      m_hardwareDecoding(new QCheckBox{QStringLiteral("Decode on the graphics card"), this}) {
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

    // The jump of `Seek Backward` and `Seek Forward`, and the lead-in of the gestures on the
    // selection — Gaupol's Preferences ▸ Video. Whole seconds for the jump, and tenths for the
    // lead-in: one second is a lead-in, and so is a half.
    m_seekLength->setRange(core::kSmallestSeekLengthSeconds, core::kLargestSeekLengthSeconds);
    m_seekLength->setSuffix(QStringLiteral(" s"));
    m_seekLength->setValue(video.seekLengthSeconds);
    m_contextLength->setRange(0.0,
                              core::kLargestContextLengthMilliseconds / kMillisecondsPerSecond);
    // **The same decimal mark everywhere**: the interface is in English, and a spin box that
    // follows the machine's locale writes « 1,0 s » on one and « 1.0 s » on the next — which would
    // make the picture of this dialog in the manual a different one on every machine.
    m_contextLength->setLocale(QLocale::c());
    m_contextLength->setDecimals(1);
    m_contextLength->setSingleStep(kContextStepSeconds);
    m_contextLength->setSuffix(QStringLiteral(" s"));
    m_contextLength->setValue(video.contextLengthMilliseconds / kMillisecondsPerSecond);
    // Frames and never milliseconds: a step of N is N pictures, whatever the rate of the film.
    m_stepFrames->setRange(core::kSmallestStepFrames, core::kLargestStepFrames);
    m_stepFrames->setSuffix(QStringLiteral(" frames"));
    m_stepFrames->setValue(video.stepFrames);
    m_hardwareDecoding->setChecked(video.hardwareDecoding);

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Theme"), m_theme);
    fields->addRow(QStringLiteral("Length unit"), m_lengthUnit);
    fields->addRow(m_showInCells);
    fields->addRow(m_showInEditor);
    fields->addRow(QStringLiteral("Seek length"), m_seekLength);
    fields->addRow(QStringLiteral("Context length"), m_contextLength);
    fields->addRow(QStringLiteral("Frame step"), m_stepFrames);
    fields->addRow(m_hardwareDecoding);

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

core::VideoSettings PreferencesDialog::video() const {
    return {.seekLengthSeconds = m_seekLength->value(),
            .contextLengthMilliseconds =
                static_cast<int>(std::lround(m_contextLength->value() * kMillisecondsPerSecond)),
            .stepFrames = m_stepFrames->value(),
            .hardwareDecoding = m_hardwareDecoding->isChecked()};
}

core::Theme PreferencesDialog::theme() const {
    return kThemes.at(static_cast<std::size_t>(m_theme->currentIndex()));
}

} // namespace subedit::gui
