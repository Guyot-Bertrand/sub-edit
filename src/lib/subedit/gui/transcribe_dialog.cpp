#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/audio_track_menu.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/transcribe_dialog.hpp>

#include <QComboBox>
#include <QDialogButtonBox>
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
                                   ToolsProbe probe,
                                   const PreferencesOpener& openPreferences,
                                   QWidget* parent)
    : QDialog(parent),
      m_tracks(std::move(tracks)),
      m_probe(std::move(probe)),
      m_track(new QComboBox{this}),
      m_language(new QLineEdit{this}),
      m_start(new QLineEdit{this}),
      m_end(new QLineEdit{this}),
      m_rangeHint(new QLabel{this}),
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

    m_start->setPlaceholderText(translate("Beginning"));
    m_end->setPlaceholderText(translate("End"));
    const QString rangeTip = translate("A time such as 2:00, 1:02:03 or 90 (seconds). Leave both "
                                       "empty to transcribe the whole film.");
    m_start->setToolTip(rangeTip);
    m_end->setToolTip(rangeTip);
    if (initial.start)
        m_start->setText(QString::fromStdString(core::clockText(*initial.start)));
    if (initial.end)
        m_end->setText(QString::fromStdString(core::clockText(*initial.end)));
    connect(m_start, &QLineEdit::textChanged, this, [this] { checkRange(); });
    connect(m_end, &QLineEdit::textChanged, this, [this] { checkRange(); });
    m_rangeHint->setWordWrap(true);
    connect(m_preferences, &QPushButton::clicked, this, [this, openPreferences] {
        openPreferences();
        refresh();
    });

    m_hint->setWordWrap(true);
    m_tools->setWordWrap(true);

    auto* explanation = new QLabel{
        translate("The audio of the film open in this window is transcribed into subtitles "
                  "that replace the ones of the project. Give a range to try a part of the "
                  "film first: the result then opens as a project of its own and nothing is "
                  "replaced. The work runs in the background and can be cancelled."),
        this};
    explanation->setWordWrap(true);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Audio track"), m_track);
    fields->addRow(translate("Language"), m_language);
    fields->addRow(QString{}, m_hint);

    auto* range = new QHBoxLayout;
    range->addWidget(m_start);
    range->addWidget(new QLabel{translate("to"), this});
    range->addWidget(m_end);
    fields->addRow(translate("Part of the film"), range);
    fields->addRow(QString{}, m_rangeHint);

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
                            .language = m_language->text().trimmed().toStdString(),
                            .start = core::readRangeBound(m_start->text().toStdString()),
                            .end = core::readRangeBound(m_end->text().toStdString())};
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
    explainLanguage();
    checkRange();
}

void TranscribeDialog::checkRange() {
    const auto read = [](const QLineEdit* field) {
        const std::string text = field->text().toStdString();
        return std::pair{core::readRangeBound(text),
                         text.find_first_not_of(' ') == std::string::npos};
    };
    const auto [start, startEmpty] = read(m_start);
    const auto [end, endEmpty] = read(m_end);

    QString problem;
    if (!startEmpty && !start)
        problem = QString::fromStdString(core::notATimeInRange(m_start->text().toStdString()));
    else if (!endEmpty && !end)
        problem = QString::fromStdString(core::notATimeInRange(m_end->text().toStdString()));
    else if (!core::isRangePossible(start, end))
        problem = QString::fromStdString(core::emptyRange());
    m_rangeHint->setText(problem);
    m_rangeValid = problem.isEmpty();
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_current.ready() && m_rangeValid);
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
