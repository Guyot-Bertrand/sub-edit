#include <subedit/core/config/theme.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/wording/settings.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/preferences_dialog.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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
    return unit == core::LengthUnit::Ems ? translate("Ems") : translate("Characters");
}

/// The source language, offered whatever is installed.
constexpr std::string_view kEnglish = "en";

/// A language as the list shows it: by its own name, or by its code when the table of names does
/// not know it.
[[nodiscard]] QString labelOf(std::string_view language) {
    return QString::fromUtf8(core::endonymOf(language).value_or(language));
}

/// The list of the language field: `System default`, English, each installed language, and the
/// language chosen if it is none of these — a setting carried from another machine is shown as it
/// is and kept, rather than silently changed to something nobody chose.
void fillLanguages(QComboBox& box, const LanguageChoice& choice) {
    // TRANSLATORS: the interface language that follows the environment, LANGUAGE and LANG.
    box.addItem(translate("System default"), QString{});

    std::vector<std::string> languages{std::string{kEnglish}};
    for (const std::string& one : choice.installed)
        if (std::ranges::find(languages, one) == languages.end())
            languages.push_back(one);
    if (choice.chosen && std::ranges::find(languages, *choice.chosen) == languages.end())
        languages.push_back(*choice.chosen);

    for (const std::string& one : languages)
        box.addItem(labelOf(one), QString::fromStdString(one));

    if (choice.chosen)
        box.setCurrentIndex(box.findData(QString::fromStdString(*choice.chosen)));
}

/// A second, in milliseconds, and the step of the lead-in's field in seconds.
constexpr double kMillisecondsPerSecond = 1000.0;
constexpr double kContextStepSeconds = 0.5;

} // namespace

PreferencesDialog::PreferencesDialog(core::Theme theme,
                                     const core::EditorSettings& editor,
                                     const core::VideoSettings& video,
                                     const LanguageChoice& language,
                                     const TranscriptionPreferences& transcription,
                                     QWidget* parent)
    : QDialog(parent),
      m_theme(new QComboBox{this}),
      m_language(new QComboBox{this}),
      m_lengthUnit(new QComboBox{this}),
      m_showInCells(new QCheckBox{translate("Show line lengths in cells"), this}),
      m_showInEditor(new QCheckBox{translate("Show line lengths in the editor"), this}),
      m_seekLength(new QSpinBox{this}),
      m_contextLength(new QDoubleSpinBox{this}),
      m_stepFrames(new QSpinBox{this}),
      m_hardwareDecoding(new QCheckBox{translate("Decode on the graphics card"), this}),
      m_transcription(transcription),
      m_program(new QLineEdit{this}),
      m_model(new QLineEdit{this}),
      m_programButton(new QPushButton{translate("Browse…"), this}),
      m_modelButton(new QPushButton{translate("Browse…"), this}),
      m_tools(new QLabel{this}) {
    setWindowTitle(translate("Preferences"));

    for (const core::Theme one : kThemes)
        m_theme->addItem(QString::fromUtf8(core::nameOf(one)));

    m_theme->setCurrentIndex(static_cast<int>(theme));

    fillLanguages(*m_language, language);

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
    // TRANSLATORS: the unit after a number of seconds, with its space: « 1.5 s ».
    m_seekLength->setSuffix(translate(" s"));
    m_seekLength->setValue(video.seekLengthSeconds);
    m_contextLength->setRange(0.0,
                              core::kLargestContextLengthMilliseconds / kMillisecondsPerSecond);
    // **The same decimal mark everywhere**: the interface is in English, and a spin box that
    // follows the machine's locale writes « 1,0 s » on one and « 1.0 s » on the next — which would
    // make the picture of this dialog in the manual a different one on every machine.
    m_contextLength->setLocale(QLocale::c());
    m_contextLength->setDecimals(1);
    m_contextLength->setSingleStep(kContextStepSeconds);
    m_contextLength->setSuffix(translate(" s"));
    m_contextLength->setValue(video.contextLengthMilliseconds / kMillisecondsPerSecond);
    // Frames and never milliseconds: a step of N is N pictures, whatever the rate of the film.
    m_stepFrames->setRange(core::kSmallestStepFrames, core::kLargestStepFrames);
    // TRANSLATORS: the unit after a number of frames, with its space: « 2 frames ».
    m_stepFrames->setSuffix(translate(" frames"));
    m_stepFrames->setValue(video.stepFrames);
    m_hardwareDecoding->setChecked(video.hardwareDecoding);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Theme"), m_theme);
    fields->addRow(translate("Interface language"), m_language);
    // Said where it is chosen (D4): a language that changed nothing would look like a broken one.
    auto* nextLaunch = new QLabel{translate("Takes effect at the next launch."), this};
    nextLaunch->setWordWrap(true);
    fields->addRow(QString{}, nextLaunch);
    fields->addRow(translate("Length unit"), m_lengthUnit);
    fields->addRow(m_showInCells);
    fields->addRow(m_showInEditor);
    fields->addRow(translate("Seek length"), m_seekLength);
    fields->addRow(translate("Context length"), m_contextLength);
    fields->addRow(translate("Frame step"), m_stepFrames);
    fields->addRow(m_hardwareDecoding);

    // What "system" does, said where it is read: without this line, a reader
    // who picks "System" and sees nothing change believes it broken.
    auto* explanation = new QLabel{
        QStringLiteral("%1 %2.").arg(QString::fromUtf8(core::nameOf(core::Theme::System)),
                                     QString::fromUtf8(core::systemThemeExplained())),
        this};
    explanation->setWordWrap(true);

    // The audio transcription: where its program and its model are, which is a fact about the
    // machine and is set once. Under the fields, what is there of what they name.
    m_program->setText(QString::fromStdString(transcription.settings.executable.string()));
    m_program->setPlaceholderText(translate("Searched for on the PATH"));
    m_model->setText(QString::fromStdString(transcription.settings.model.string()));
    m_tools->setWordWrap(true);
    connect(m_program, &QLineEdit::textChanged, this, [this] { refreshTools(); });
    connect(m_model, &QLineEdit::textChanged, this, [this] { refreshTools(); });
    connect(m_programButton, &QPushButton::clicked, this, [this] {
        pick(ChosenFile::Program, m_program);
    });
    connect(
        m_modelButton, &QPushButton::clicked, this, [this] { pick(ChosenFile::Model, m_model); });

    auto* programRow = new QHBoxLayout;
    programRow->addWidget(m_program, 1);
    programRow->addWidget(m_programButton);
    auto* modelRow = new QHBoxLayout;
    modelRow->addWidget(m_model, 1);
    modelRow->addWidget(m_modelButton);
    auto* transcriptionFields = new QFormLayout;
    transcriptionFields->addRow(translate("Program"), programRow);
    transcriptionFields->addRow(translate("Model"), modelRow);
    transcriptionFields->addRow(m_tools);
    auto* transcriptionGroup = new QGroupBox{translate("Transcription"), this};
    transcriptionGroup->setLayout(transcriptionFields);
    refreshTools();

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addLayout(fields);
    stack->addWidget(explanation);
    stack->addWidget(transcriptionGroup);
    stack->addWidget(buttons);
}

void PreferencesDialog::refreshTools() {
    if (!m_transcription.tools)
        return;
    const core::TranscriptionSettings settings = transcription();
    const core::TranscriptionTools tools = m_transcription.tools(settings);
    QString report = QString::fromStdString(core::transcriptionToolsReport(tools, settings));
    // The pointer to the guide only when something is missing: a person whose tools are all
    // there has no use for it.
    if (!tools.ready())
        report +=
            QStringLiteral("\n\n%1").arg(QString::fromStdString(core::installationGuidePointer()));
    m_tools->setText(report);
}

void PreferencesDialog::pick(ChosenFile what, QLineEdit* field) const {
    if (!m_transcription.choose)
        return;
    const auto chosen =
        m_transcription.choose(what, std::filesystem::path{field->text().toStdString()});
    if (chosen)
        field->setText(QString::fromStdString(chosen->string()));
}

core::TranscriptionSettings PreferencesDialog::transcription() const {
    core::TranscriptionSettings settings = m_transcription.settings;
    settings.executable = std::filesystem::path{m_program->text().trimmed().toStdString()};
    settings.model = std::filesystem::path{m_model->text().trimmed().toStdString()};
    return settings;
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

std::optional<std::string> PreferencesDialog::interfaceLanguage() const {
    const QString code = m_language->currentData().toString();
    if (code.isEmpty())
        return std::nullopt;
    return code.toStdString();
}

core::Theme PreferencesDialog::theme() const {
    return kThemes.at(static_cast<std::size_t>(m_theme->currentIndex()));
}

} // namespace subedit::gui
