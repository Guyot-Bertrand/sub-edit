#pragma once

// The join-and-split page of the correction assistant — issue #508, D6 and
// D8: between the mentions and the common errors, two boxes and the language
// the words are spelt by.

#include <subedit/core/config/correction_settings.hpp>

#include <QString>
#include <QWizardPage>

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class SpellProvider;
} // namespace subedit::core

class QCheckBox;
class QComboBox;
class QLabel;

namespace subedit::gui {

/// The language to open the page on when none was chosen yet: `systemLocale`,
/// matched exactly and then by its language alone among `offered`, else the
/// system's own code — which the page then says it has no dictionary for. A
/// locale that is not a code (`C`) reads as `en`.
[[nodiscard]] std::string spellLanguageFor(const std::vector<std::string>& offered,
                                           std::string_view systemLocale);

/// **Without a dictionary for the language the page stays, greyed, and says
/// why** — D6: Gaupol drops the page without a word, and a user who wonders
/// where it went has nothing to read. The language itself stays choosable: a
/// dictionary for another one may be there.
class JoinSplitPage final : public QWizardPage {
    Q_OBJECT

public:
    /// `provider` is where dictionaries come from and may be null — a
    /// window with no spell-checking at all — and must outlive the page.
    explicit JoinSplitPage(const core::SpellProvider* provider, QWidget* parent = nullptr);

    /// Opens the page on `settings`. An empty language is the system's.
    void applySettings(const core::CorrectionSettings& settings);

    /// The language chosen, a locale code.
    [[nodiscard]] std::string language() const;

    [[nodiscard]] bool join() const;
    [[nodiscard]] bool split() const;

    /// Whether there is a dictionary for the language chosen.
    [[nodiscard]] bool available() const { return m_available; }

    /// What the page says when it does not, empty when it does.
    [[nodiscard]] QString unavailableReason() const;

private:
    /// Greys or ungreys the boxes for the language the combo shows now.
    void refresh();

    const core::SpellProvider* m_provider;
    QComboBox* m_language;
    QCheckBox* m_join;
    QCheckBox* m_split;
    QLabel* m_reason;
    bool m_available = false;
};

} // namespace subedit::gui
