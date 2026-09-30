#pragma once

// `Tools > Check Spelling…` and `Tools > Spell-Check Settings…` end to end —
// issue #509, decision D6.

#include <subedit/core/config/spell_check_settings.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <filesystem>
#include <memory>
#include <span>
#include <string>

class QWidget;

namespace subedit::core {
class Project;
class SpellProvider;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ProjectPage;

/// Opens the settings window and the spell-check window, resolves the
/// target, applies what the walk corrected — one history entry per project —
/// and saves the replacement list.
class SpellCheckController final {

public:
    class View {

    public:
        virtual ~View() = default;

        [[nodiscard]] virtual QWidget* dialogParent() = 0;

        /// Every open project, in tab order.
        [[nodiscard]] virtual std::span<const std::unique_ptr<ProjectPage>> pages() const = 0;

        [[nodiscard]] virtual std::size_t shownProject() const = 0;

        /// Where dictionaries come from, or null for a window with none.
        [[nodiscard]] virtual const core::SpellProvider* spellProvider() const = 0;

        /// The configuration directory the per-language replacement lists
        /// live under — **given**, never resolved here (ADR 0022).
        [[nodiscard]] virtual std::filesystem::path spellConfigDirectory() const = 0;

        virtual void announce(const std::string& message) = 0;

        /// Shows the tab of `project` and selects the row `index` in it.
        virtual void reveal(const core::Project& project, core::SubtitleIndex index) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// `prompts` and `view` must outlive this.
    SpellCheckController(Prompts& prompts, View& view);

    /// Opens the settings window; the settings are kept only if it is accepted.
    void configure();

    /// Walks the misspelt words of the target, and applies what was corrected
    /// when the window closes, however it closes.
    void openCheck();

    /// Whether there is a dictionary for the language the settings name
    /// (the system's when empty).
    [[nodiscard]] bool spellCheckAvailable() const;

    /// Why `openCheck` cannot run; empty when it can.
    [[nodiscard]] std::string unavailableReason() const;

    /// The language the settings name, the system's when empty.
    [[nodiscard]] std::string resolvedLanguage() const;

    [[nodiscard]] const core::SpellCheckSettings& settings() const { return m_settings; }

    void setSettings(core::SpellCheckSettings settings) { m_settings = std::move(settings); }

private:
    Prompts* m_prompts;
    View* m_view;
    core::SpellCheckSettings m_settings;
};

} // namespace subedit::gui
