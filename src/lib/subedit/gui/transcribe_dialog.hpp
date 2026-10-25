#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/core/video/video_player.hpp>

#include <QDialog>

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;

namespace subedit::gui {

/// What `Transcribe Audio…` is asked for: which audio track of the film, and in which
/// language. **The engine and the model are not asked here**: where a program and a model are
/// is a fact about the machine, and it is set once, in the preferences.
struct TranscribeChoice {
    /// The position of the track among the film's audio tracks, from 0.
    int track = 0;

    /// An ISO 639-1 code, or empty to let the model detect it.
    std::string language{};

    /// Whether the engine is told not to write sounds as notes.
    bool suppressNotes = true;

    /// The part of the film to transcribe; all of it when neither is given. The result replaces the
    /// subtitles of the project like any other, after the same question.
    std::optional<core::Timestamp> start{};
    std::optional<core::Timestamp> end{};

    friend bool operator==(const TranscribeChoice&, const TranscribeChoice&) = default;
};

/// The settings and what is there of what they name, at the moment the dialog looks.
struct ToolsView {
    core::TranscriptionSettings settings{};
    core::TranscriptionTools tools{};
};

/// Asks for the audio track and the language of a transcription, and says whether it can start.
///
/// **The input is the project's film and nothing else**: there is no file chooser for the audio.
/// The tracks are those the player lists, with the language and the title the file declares.
///
/// **It says what is missing**: the program, `ffmpeg`, the model, one line each, with the
/// pointer to the guide and a button that opens the preferences, where they are set. `OK` is out
/// until the three are there, and the dialog looks again when the preferences close.
///
/// **It says where the language comes from.** The field starts with the language the file declares
/// for the chosen track, when it declares one. When it does not, the model listens to the sound and
/// finds it — a multilingual model only — and the line under the field says so; a model for
/// English alone detects nothing, and the line says that the language must be given.
class TranscribeDialog final : public QDialog {
    Q_OBJECT

public:
    /// Looks at the settings and at the tools they name.
    using ToolsProbe = std::function<ToolsView()>;

    /// Opens the preferences; the dialog looks at the tools again afterwards.
    using PreferencesOpener = std::function<void()>;

    /// `tracks` are the film's, in order; there may be none, and the box then offers the
    /// first track all the same, which is the one ffmpeg takes.
    TranscribeDialog(std::vector<core::AudioTrack> tracks,
                     const TranscribeChoice& initial,
                     ToolsProbe probe,
                     const PreferencesOpener& openPreferences,
                     QWidget* parent = nullptr);

    /// The form as the fields say it.
    [[nodiscard]] TranscribeChoice choice() const;

    [[nodiscard]] QComboBox* trackBox() const { return m_track; }

    [[nodiscard]] QComboBox* languageBox() const { return m_language; }

    /// The lines that say what is there, and what to do about what is not.
    [[nodiscard]] QLabel* toolsLabel() const { return m_tools; }

    /// The line under the language field.
    [[nodiscard]] QLabel* languageHint() const { return m_hint; }

    /// The fields of the range, empty for the whole film.
    /// The box for not writing sounds as notes; greyed for an engine that cannot.
    [[nodiscard]] QCheckBox* suppressNotesBox() const { return m_suppressNotes; }

    [[nodiscard]] QLineEdit* startEdit() const { return m_start; }

    [[nodiscard]] QLineEdit* endEdit() const { return m_end; }

    /// What is wrong with the range, or nothing.
    [[nodiscard]] QLabel* rangeHint() const { return m_rangeHint; }

    [[nodiscard]] QPushButton* preferencesButton() const { return m_preferences; }

    [[nodiscard]] QDialogButtonBox* buttons() const { return m_buttons; }

private:
    /// Looks at the tools again, and redraws what depends on them.
    void refresh();

    /// Proposes the declared language of the chosen track, unless the person has chosen one.
    void followTrack();

    /// Says where the language comes from, or that it must be given.
    void explainLanguage();

    /// The code of the language chosen in the list, empty for "detect".
    [[nodiscard]] std::string selectedLanguage() const;

    /// Fills the list of languages with what the engine offers for the model, in the language of
    /// the window.
    void fillLanguages();

    /// `OK` is in when the tools are there and the fields can be used.
    void updateButtons();

    /// Says what is wrong with the range, if anything, and whether `OK` may be pressed.
    void checkRange();

    std::vector<core::AudioTrack> m_tracks;
    ToolsProbe m_probe;
    core::TranscriptionTools m_current{};
    QComboBox* m_track;
    QComboBox* m_language;
    QCheckBox* m_suppressNotes;
    QLineEdit* m_start;
    QLineEdit* m_end;
    QLabel* m_rangeHint;
    QLabel* m_hint;
    QLabel* m_tools;
    QPushButton* m_preferences;
    QDialogButtonBox* m_buttons;

    /// Whether the person typed a language, which a change of track must then respect.
    bool m_languageChosen = false;

    /// Whether the range, as typed, can be used.
    bool m_rangeValid = true;

    /// The code asked for, kept across a refill of the list: the person's choice, the track's, or
    /// the setting's.
    std::string m_wantedLanguage;
};

} // namespace subedit::gui
