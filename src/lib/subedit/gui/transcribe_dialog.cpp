#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/audio_track_menu.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/transcribe_dialog.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <utility>

namespace subedit::gui {

TranscribeDialog::TranscribeDialog(std::vector<core::AudioTrack> tracks,
                                   const TranscribeChoice& initial,
                                   ToolsProbe probe,
                                   const PreferencesOpener& openPreferences,
                                   QWidget* parent)
    : QDialog(parent),
      m_tracks(std::move(tracks)),
      m_probe(std::move(probe)),
      m_track(new QComboBox{this}),
      m_language(new QLineEdit{this}),
      m_hint(new QLabel{this}),
      m_tools(new QLabel{this}),
      m_preferences(new QPushButton{translate("Preferences…"), this}),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this}),
      m_languageChosen(!initial.language.empty()) {
    setWindowTitle(translate("Transcribe audio"));

    int number = 1;
    for (const core::AudioTrack& track : m_tracks)
        m_track->addItem(audioTrackLabel(track, number++));
    if (m_tracks.empty())
        m_track->addItem(translate("First audio track"));
    m_track->setCurrentIndex(initial.track < m_track->count() ? initial.track : 0);

    m_language->setText(QString::fromStdString(initial.language));
    m_language->setPlaceholderText(translate("Detected from the sound"));
    m_language->setMaxLength(2);
    m_language->setToolTip(translate("A two-letter language code such as en or fr. Leave it empty "
                                     "to let the model detect the language."));
    connect(m_language, &QLineEdit::textEdited, this, [this] {
        m_languageChosen = true;
        explainLanguage();
    });
    connect(m_track, &QComboBox::currentIndexChanged, this, [this] { followTrack(); });
    connect(m_preferences, &QPushButton::clicked, this, [this, openPreferences] {
        openPreferences();
        refresh();
    });

    m_hint->setWordWrap(true);
    m_tools->setWordWrap(true);

    auto* explanation = new QLabel{
        translate("The audio of the film open in this window is transcribed into subtitles "
                  "that replace the ones of the project. The work runs in the background and "
                  "can be cancelled."),
        this};
    explanation->setWordWrap(true);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Audio track"), m_track);
    fields->addRow(translate("Language"), m_language);
    fields->addRow(QString{}, m_hint);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(explanation);
    stack->addLayout(fields);
    stack->addWidget(m_tools);
    stack->addWidget(m_preferences);
    stack->addWidget(m_buttons);

    if (!m_languageChosen)
        followTrack();
    refresh();
}

TranscribeChoice TranscribeDialog::choice() const {
    return TranscribeChoice{.track = m_track->currentIndex(),
                            .language = m_language->text().trimmed().toStdString()};
}

void TranscribeDialog::refresh() {
    const ToolsView view = m_probe();
    m_current = view.tools;

    QString report =
        QString::fromStdString(core::transcriptionToolsReport(view.tools, view.settings));
    if (!view.tools.ready())
        report += QStringLiteral("\n\n%1\n%2")
                      .arg(translate("Set them in the preferences."),
                           QString::fromStdString(core::installationGuidePointer()));
    m_tools->setText(report);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(view.tools.ready());
    explainLanguage();
}

void TranscribeDialog::followTrack() {
    if (!m_languageChosen) {
        const auto index = static_cast<std::size_t>(m_track->currentIndex());
        const auto code = index < m_tracks.size() ? core::twoLetterCodeOf(m_tracks[index].language)
                                                  : std::optional<std::string>{};
        m_language->setText(code ? QString::fromStdString(*code) : QString{});
    }
    explainLanguage();
}

void TranscribeDialog::explainLanguage() {
    const auto index = static_cast<std::size_t>(m_track->currentIndex());
    const bool declared =
        index < m_tracks.size() && core::twoLetterCodeOf(m_tracks[index].language).has_value();

    QString hint;
    if (m_current.model == core::ModelState::EnglishOnly)
        hint = translate("This model cannot detect the language: type it.");
    else if (m_languageChosen)
        hint = translate("The language you typed is used.");
    else if (declared)
        hint = translate("Proposed from the language the file declares for this track.");
    else
        hint = translate("The file declares no language for this track: the model will detect it "
                         "from the sound. Type it if you know it.");
    m_hint->setText(hint);
}

} // namespace subedit::gui
