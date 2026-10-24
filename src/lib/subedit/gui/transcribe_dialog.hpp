#pragma once

#include <subedit/core/video/video_player.hpp>

#include <QDialog>

#include <filesystem>
#include <span>
#include <string>
#include <vector>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace subedit::gui {

/// What `Transcribe Audio…` is asked for: which audio track of the film, in which
/// language, with which model. The engine and the rest are settings.
struct TranscribeChoice {
    /// The position of the track among the film's audio tracks, from 0.
    int track = 0;

    /// An ISO 639-1 code, or empty to let the engine detect it.
    std::string language{};

    /// The model file; empty when none is set.
    std::filesystem::path model{};

    friend bool operator==(const TranscribeChoice&, const TranscribeChoice&) = default;
};

/// Asks for the audio track, the language and the model of a transcription.
///
/// **The input is the project's film and nothing else**: there is no file chooser for the
/// audio. The tracks are those the player lists, with the language and the title the file
/// declares. The language field starts with the one the settings hold, else the declared
/// language of the chosen track, and follows the track as long as nobody has typed in it.
class TranscribeDialog final : public QDialog {
    Q_OBJECT

public:
    /// `tracks` are the film's, in order; there may be none, and the box then offers the
    /// first track all the same, which is the one ffmpeg takes.
    TranscribeDialog(std::vector<core::AudioTrack> tracks,
                     const TranscribeChoice& initial,
                     QWidget* parent = nullptr);

    /// The form as the fields say it.
    [[nodiscard]] TranscribeChoice choice() const;

    [[nodiscard]] QComboBox* trackBox() const { return m_track; }

    [[nodiscard]] QLineEdit* languageEdit() const { return m_language; }

    [[nodiscard]] QLineEdit* modelEdit() const { return m_model; }

private:
    /// Proposes the declared language of the chosen track, unless the person has chosen one.
    void followTrack();

    void chooseModel();

    std::vector<core::AudioTrack> m_tracks;
    QComboBox* m_track;
    QLineEdit* m_language;
    QLineEdit* m_model;
    QPushButton* m_browse;

    /// Whether the person typed a language, which a change of track must then respect.
    bool m_languageChosen = false;
};

} // namespace subedit::gui
