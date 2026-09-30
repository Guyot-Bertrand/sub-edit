#pragma once

// Gaupol's `LanguageDialog` for the spell check — issue #509: which language
// to check by, which subtitles, and which of their texts. Kept apart from
// `Check Spelling…` so that a language with no dictionary can still be left.

#include <subedit/core/config/spell_check_settings.hpp>

#include <QDialog>
#include <QString>

#include <string>

namespace subedit::core {
class SpellProvider;
} // namespace subedit::core

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QRadioButton;

namespace subedit::gui {

class SpellCheckSettingsDialog final : public QDialog {
    Q_OBJECT

public:
    /// `provider` is where dictionaries come from and may be null; it must
    /// outlive the dialog. `selectionAvailable`: rows are selected.
    /// `translationAvailable`: a project carries a translation.
    SpellCheckSettingsDialog(const core::SpellProvider* provider,
                             bool selectionAvailable,
                             bool translationAvailable,
                             QWidget* parent = nullptr);

    /// Opens the dialog on `settings`. An empty language is the system's; a
    /// target or document that is not available falls back to the current
    /// project and the text.
    void apply(const core::SpellCheckSettings& settings);

    /// What the dialog shows now.
    [[nodiscard]] core::SpellCheckSettings settings() const;

    [[nodiscard]] std::string language() const;
    [[nodiscard]] core::SpellCheckTarget target() const;
    [[nodiscard]] core::SpellCheckDocument document() const;

    /// Whether unknown words are underlined while typed — issue #525.
    [[nodiscard]] bool inlineCheck() const;

    /// Whether there is a dictionary for the language shown.
    [[nodiscard]] bool available() const { return m_available; }

    /// What the dialog says when there is not, empty when there is.
    [[nodiscard]] QString unavailableReason() const;

    [[nodiscard]] QComboBox* languageBox() const { return m_language; }

    [[nodiscard]] QRadioButton* selectionRadio() const { return m_selection; }

    [[nodiscard]] QRadioButton* currentProjectRadio() const { return m_currentProject; }

    [[nodiscard]] QRadioButton* allProjectsRadio() const { return m_allProjects; }

    [[nodiscard]] QRadioButton* textRadio() const { return m_text; }

    [[nodiscard]] QRadioButton* translationRadio() const { return m_translation; }

    [[nodiscard]] QCheckBox* inlineCheckBox() const { return m_inline; }

    [[nodiscard]] QDialogButtonBox* buttons() const { return m_buttons; }

private:
    void refresh();

    const core::SpellProvider* m_provider;
    QComboBox* m_language;
    QLabel* m_reason;
    QRadioButton* m_selection;
    QRadioButton* m_currentProject;
    QRadioButton* m_allProjects;
    QRadioButton* m_text;
    QRadioButton* m_translation;
    QCheckBox* m_inline;
    QDialogButtonBox* m_buttons;
    bool m_available = false;
};

} // namespace subedit::gui
