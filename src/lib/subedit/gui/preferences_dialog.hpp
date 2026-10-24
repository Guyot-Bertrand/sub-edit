#pragma once

#include <subedit/core/config/editor_settings.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/gui/prompts.hpp>

#include <QDialog>

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QWidget;

namespace subedit::gui {

/// What the language field of the dialog starts from — issue #669.
struct LanguageChoice {
    /// The value of `interface.language`: nothing for `system`, the default.
    std::optional<std::string> chosen{};

    /// The languages installed beside the program, as `core::availableLanguages` finds them.
    /// English is offered whatever this holds: it is the source.
    std::vector<std::string> installed{};
};

/// What the transcription group of the dialog starts from, and what it asks of the window.
struct TranscriptionPreferences {
    /// The settings; only the program and the model are changed here.
    core::TranscriptionSettings settings{};

    /// Looks at what the settings name, to say what is there. Without it the group says nothing.
    std::function<core::TranscriptionTools(const core::TranscriptionSettings&)> tools{};

    /// Asks which file to take, from where the field stands.
    std::function<std::optional<std::filesystem::path>(ChosenFile, const std::filesystem::path&)>
        choose{};
};

/// What is set by no gesture other than setting it.
///
/// **Few preferences, and that is a criterion and not a shortfall**: the
/// geometry, the maximised state, the column widths and the position of the
/// handle are all set by moving the window or dragging an edge, and a
/// preference that already has a gesture does not need a field. The theme has
/// none; nor do the three settings of the editor — issue #526, Gaupol's
/// Preferences ▸ Editor: the unit a line's length is shown in, and whether the
/// cells and the cell editor show it.
///
/// **The interface language is the one that does not apply at once** (#669): the window is
/// translated as it is built, and re-translating a live one is not worth its cost (spec 15, D4).
/// The dialog says so under the field, and the choice is kept for the next launch.
///
/// The default frame rate the scoping announced **will not come**: #267 looked
/// into it and set it aside. None of the three possible readers needed it — the
/// conversion prefills its top field with the deduced grid, the alignment opens
/// on what the video declares, and a project with neither is a project where
/// the user chooses. A preference nobody uses is a box that lies.
class PreferencesDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(core::Theme theme,
                               const core::EditorSettings& editor = {},
                               const core::VideoSettings& video = {},
                               const LanguageChoice& language = {},
                               const TranscriptionPreferences& transcription = {},
                               QWidget* parent = nullptr);

    /// The theme chosen, whether or not it was accepted — the caller looks at
    /// the return code to know whether to take it into account.
    [[nodiscard]] core::Theme theme() const;

    /// The list of themes, so that a test picks without clicking.
    [[nodiscard]] QComboBox* themeBox() const { return m_theme; }

    /// The three settings of the editor as they stand in the dialog, whether or
    /// not it was accepted.
    [[nodiscard]] core::EditorSettings editor() const;

    /// The two settings of the video player the dialog holds, as they stand in it: the jump and the
    /// lead-in. **The volume is not here** — the bar and the gestures set it — and comes back as
    /// the default, for the caller to put its own.
    [[nodiscard]] core::VideoSettings video() const;

    /// The interface language chosen, nothing for `System default`, whether or not it was
    /// accepted.
    [[nodiscard]] std::optional<std::string> interfaceLanguage() const;

    /// The settings of the transcription as they stand in the dialog: the ones it was given, with
    /// the program and the model the fields hold.
    [[nodiscard]] core::TranscriptionSettings transcription() const;

    /// The fields of the transcription group, so that a test fills them without clicking.
    [[nodiscard]] QLineEdit* programEdit() const { return m_program; }

    [[nodiscard]] QLineEdit* modelEdit() const { return m_model; }

    [[nodiscard]] QLabel* toolsLabel() const { return m_tools; }

    [[nodiscard]] QPushButton* programButton() const { return m_programButton; }

    [[nodiscard]] QPushButton* modelButton() const { return m_modelButton; }

    /// The list of languages, so that a test picks without clicking.
    [[nodiscard]] QComboBox* languageBox() const { return m_language; }

    /// The controls, so that a test sets them without clicking.
    [[nodiscard]] QComboBox* lengthUnitBox() const { return m_lengthUnit; }

    [[nodiscard]] QSpinBox* seekLengthBox() const { return m_seekLength; }

    [[nodiscard]] QDoubleSpinBox* contextLengthBox() const { return m_contextLength; }

    [[nodiscard]] QSpinBox* stepFramesBox() const { return m_stepFrames; }

    [[nodiscard]] QCheckBox* hardwareDecodingBox() const { return m_hardwareDecoding; }

    [[nodiscard]] QCheckBox* showLengthsInCellsBox() const { return m_showInCells; }

    [[nodiscard]] QCheckBox* showLengthsInEditorBox() const { return m_showInEditor; }

private:
    /// The unit means nothing when no length is shown, so it is greyed rather
    /// than left to be set to no effect.
    void refreshLengthUnitState();

    /// Says what is there of what the program and the model fields name.
    void refreshTools();

    /// Fills `field` with the file the person picks for `what`.
    void pick(ChosenFile what, QLineEdit* field) const;

    QComboBox* m_theme;
    QComboBox* m_language;
    QComboBox* m_lengthUnit;
    QCheckBox* m_showInCells;
    QCheckBox* m_showInEditor;
    QSpinBox* m_seekLength;
    QDoubleSpinBox* m_contextLength;
    QSpinBox* m_stepFrames;
    QCheckBox* m_hardwareDecoding;

    TranscriptionPreferences m_transcription;
    QLineEdit* m_program;
    QLineEdit* m_model;
    QPushButton* m_programButton;
    QPushButton* m_modelButton;
    QLabel* m_tools;
};

} // namespace subedit::gui
