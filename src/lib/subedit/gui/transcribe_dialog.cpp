#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/transcription/language_display.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/audio_track_menu.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/timestamp_edit.hpp>
#include <subedit/gui/transcribe_dialog.hpp>

#include <QCheckBox>
#include <QCollator>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

/// `00:02:00`, the form the fields ask for.
[[nodiscard]] std::string paddedClock(core::Timestamp position) {
    return position.format(
        core::DecimalMark::Period, core::HourField::Always, core::Decimals::Seconds);
}

} // namespace

TranscribeDialog::TranscribeDialog(std::vector<core::AudioTrack> tracks,
                                   const TranscribeChoice& initial,
                                   ToolsProbe probe,
                                   const PreferencesOpener& openPreferences,
                                   QWidget* parent)
    : QDialog(parent),
      m_tracks(std::move(tracks)),
      m_probe(std::move(probe)),
      m_track(new QComboBox{this}),
      m_language(new QComboBox{this}),
      m_suppressNotes(new QCheckBox{translate("Do not write sounds as notes"), this}),
      m_start(new TimestampEdit{{.milliseconds = false, .optional = true}, this}),
      m_end(new TimestampEdit{{.milliseconds = false, .optional = true}, this}),
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

    m_wantedLanguage = initial.language;
    m_language->setToolTip(
        translate("Choose the language of the speech. \"Detect\" lets the model find it from the "
                  "first 30 seconds of sound, which a film that opens with music can get wrong."));
    connect(m_language, &QComboBox::activated, this, [this] {
        m_languageChosen = true;
        m_wantedLanguage = selectedLanguage();
        explainLanguage();
    });
    connect(m_track, &QComboBox::currentIndexChanged, this, [this] { followTrack(); });

    m_suppressNotes->setChecked(initial.suppressNotes);
    m_suppressNotes->setToolTip(translate(
        "Sounds such as (phone ringing) or [MUSIC PLAYING] are not written. Turn it off to "
        "keep them, for subtitles for the hard of hearing."));

    m_start->setPlaceholderText(QStringLiteral("HH:MM:SS"));
    m_end->setPlaceholderText(QStringLiteral("HH:MM:SS"));
    const QString rangeTip = translate("A time as HH:MM:SS, for example 00:02:00. Leave both "
                                       "empty to transcribe the whole film.");
    m_start->setToolTip(rangeTip);
    m_end->setToolTip(rangeTip);
    if (initial.start)
        m_start->setText(QString::fromStdString(paddedClock(*initial.start)));
    if (initial.end)
        m_end->setText(QString::fromStdString(paddedClock(*initial.end)));
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
                  "that replace the ones of the project. The work runs in the background and "
                  "can be cancelled."),
        this};
    explanation->setWordWrap(true);

    auto* fields = new QFormLayout;
    fields->addRow(translate("Audio track"), m_track);
    fields->addRow(translate("Language"), m_language);
    fields->addRow(QString{}, m_hint);
    fields->addRow(QString{}, m_suppressNotes);

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

std::string TranscribeDialog::selectedLanguage() const {
    return m_language->currentData().toString().toStdString();
}

TranscribeChoice TranscribeDialog::choice() const {
    return TranscribeChoice{.track = m_track->currentIndex(),
                            .language = selectedLanguage(),
                            .suppressNotes = m_suppressNotes->isChecked(),
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
    fillLanguages();
    explainLanguage();
    checkRange();

    // Greyed, with the reason, for an engine that cannot be told: the checkbox is not removed, so
    // that the person sees the option exists and why it is out.
    const bool available = view.tools.suppressNotesAvailable();
    m_suppressNotes->setEnabled(available);
    if (!available)
        m_suppressNotes->setToolTip(
            translate("This engine cannot be told not to write sounds as notes."));
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
    updateButtons();
}

void TranscribeDialog::updateButtons() {
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_current.ready() && m_rangeValid);
}

void TranscribeDialog::fillLanguages() {
    const std::optional<core::EngineCapabilities>& capabilities = m_current.capabilities;
    const bool englishOnly = m_current.model == core::ModelState::EnglishOnly;
    const std::string reader{core::interfaceLanguageName()};

    const QSignalBlocker quiet{m_language};
    m_language->clear();
    // A model that detects nothing offers no "detect"; one that does, offers it first.
    if (!englishOnly)
        m_language->addItem(translate("Detect from the first 30 seconds of sound"), QString{});

    // The engine's own closed set, by name in the language of the window, in that language's order.
    std::vector<std::pair<QString, QString>> entries;
    if (capabilities) {
        for (const std::string& code : capabilities->languages)
            entries.emplace_back(
                QStringLiteral("%1 (%2)").arg(
                    QString::fromStdString(core::languageDisplayName(code, reader)),
                    QString::fromStdString(code)),
                QString::fromStdString(code));
    } else if (!m_wantedLanguage.empty()) {
        // Nothing known yet of what the engine offers: keep the language already wanted.
        entries.emplace_back(QString::fromStdString(m_wantedLanguage),
                             QString::fromStdString(m_wantedLanguage));
    }
    QCollator collator{QLocale{QString::fromStdString(reader.empty() ? "en" : reader)}};
    std::ranges::sort(entries, [&collator](const auto& left, const auto& right) {
        return collator.compare(left.first, right.first) < 0;
    });
    for (const auto& [text, code] : entries)
        m_language->addItem(text, code);

    const int wanted = m_language->findData(QString::fromStdString(m_wantedLanguage));
    m_language->setCurrentIndex(wanted >= 0 ? wanted : 0);
    // One entry is no choice: shown, not offered.
    m_language->setEnabled(m_language->count() > 1);
}

void TranscribeDialog::followTrack() {
    if (!m_languageChosen) {
        const auto index = static_cast<std::size_t>(m_track->currentIndex());
        const auto code = index < m_tracks.size() ? core::twoLetterCodeOf(m_tracks[index].language)
                                                  : std::optional<std::string>{};
        m_wantedLanguage = code.value_or(std::string{});
        const int at = m_language->findData(QString::fromStdString(m_wantedLanguage));
        m_language->setCurrentIndex(at >= 0 ? at : 0);
    }
    explainLanguage();
}

void TranscribeDialog::explainLanguage() {
    const auto index = static_cast<std::size_t>(m_track->currentIndex());
    const bool declared =
        index < m_tracks.size() && core::twoLetterCodeOf(m_tracks[index].language).has_value();
    const bool englishOnly = m_current.model == core::ModelState::EnglishOnly;

    QString hint;
    if (englishOnly)
        hint = QString::fromStdString(core::noticeOfEnglishOnlyModel());
    else if (m_languageChosen)
        hint = translate("The language you chose is used.");
    else if (declared)
        hint = translate("Proposed from the language the file declares for this track.");
    else
        hint = translate("The file declares no language for this track: the model will detect it "
                         "from the first 30 seconds of sound. If the film opens with music or "
                         "silence it can choose wrongly, and the speech is then translated into "
                         "that language. Choose the language if you know it.");
    m_hint->setText(hint);
    updateButtons();
}

} // namespace subedit::gui
