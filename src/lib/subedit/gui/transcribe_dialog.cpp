#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/gui/audio_track_menu.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/transcribe_dialog.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <utility>

namespace subedit::gui {

TranscribeDialog::TranscribeDialog(std::vector<core::AudioTrack> tracks,
                                   const TranscribeChoice& initial,
                                   QWidget* parent)
    : QDialog(parent),
      m_tracks(std::move(tracks)),
      m_track(new QComboBox{this}),
      m_language(new QLineEdit{this}),
      m_model(new QLineEdit{this}),
      m_browse(new QPushButton{translate("Browse…"), this}),
      m_languageChosen(!initial.language.empty()) {
    setWindowTitle(translate("Transcribe audio"));

    int number = 1;
    for (const core::AudioTrack& track : m_tracks)
        m_track->addItem(audioTrackLabel(track, number++));
    if (m_tracks.empty())
        m_track->addItem(translate("First audio track"));
    m_track->setCurrentIndex(initial.track < m_track->count() ? initial.track : 0);

    m_language->setText(QString::fromStdString(initial.language));
    m_language->setPlaceholderText(translate("Detected"));
    m_language->setMaxLength(2);
    m_language->setToolTip(translate("A two-letter language code such as en or fr. Leave it empty "
                                     "to let the model detect the language."));
    connect(m_language, &QLineEdit::textEdited, this, [this] { m_languageChosen = true; });

    m_model->setText(QString::fromStdString(initial.model.string()));
    m_model->setToolTip(translate("The model file the engine reads. It is not shipped with the "
                                  "program: the manual says where to find one."));
    connect(m_browse, &QPushButton::clicked, this, [this] { chooseModel(); });
    connect(m_track, &QComboBox::currentIndexChanged, this, [this] { followTrack(); });
    if (!m_languageChosen)
        followTrack();

    auto* explanation = new QLabel{
        translate("The audio of the film open in this window is transcribed into subtitles "
                  "that replace the ones of the project. The work runs in the background and "
                  "can be cancelled."),
        this};
    explanation->setWordWrap(true);

    auto* modelRow = new QHBoxLayout;
    modelRow->addWidget(m_model, 1);
    modelRow->addWidget(m_browse);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Audio track"), m_track);
    fields->addRow(translate("Language"), m_language);
    fields->addRow(translate("Model"), modelRow);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(explanation);
    stack->addLayout(fields);
    stack->addWidget(buttons);
}

TranscribeChoice TranscribeDialog::choice() const {
    return TranscribeChoice{.track = m_track->currentIndex(),
                            .language = m_language->text().trimmed().toStdString(),
                            .model =
                                std::filesystem::path{m_model->text().trimmed().toStdString()}};
}

void TranscribeDialog::followTrack() {
    if (m_languageChosen)
        return;
    const auto index = static_cast<std::size_t>(m_track->currentIndex());
    if (index >= m_tracks.size())
        return;
    const auto code = core::twoLetterCodeOf(m_tracks[index].language);
    m_language->setText(code ? QString::fromStdString(*code) : QString{});
}

void TranscribeDialog::chooseModel() {
    const QString chosen = QFileDialog::getOpenFileName(this,
                                                        translate("Choose a model"),
                                                        m_model->text(),
                                                        translate("Models (*.bin);;All files (*)"));
    if (!chosen.isEmpty())
        m_model->setText(chosen);
}

} // namespace subedit::gui
