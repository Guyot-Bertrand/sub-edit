#pragma once

// `Tools ▸ Correct Texts…` end to end — issue #505, task 15.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <QFont>

#include <memory>
#include <span>
#include <string>
#include <vector>

class QWidget;

namespace subedit::core {
class PatternCatalogue;
class Project;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ProjectPage;

/// `Tools ▸ Correct Texts…` end to end — D8: opens the wizard, resolves the
/// target, computes proposals in the background, applies what is accepted.
class CorrectionController final {

public:
    class View {

    public:
        virtual ~View() = default;

        [[nodiscard]] virtual QWidget* dialogParent() = 0;

        /// Every open project, in tab order — the same road `ProjectSearch`
        /// walks for "every open project", but as a span: Task 4's
        /// `correctionTargetsOf` is written against `MainWindow::m_pages`'s
        /// own storage.
        [[nodiscard]] virtual std::span<const std::unique_ptr<ProjectPage>> pages() const = 0;

        [[nodiscard]] virtual std::size_t shownProject() const = 0;

        [[nodiscard]] virtual const core::PatternCatalogue& patternCatalogue() const = 0;

        /// The font the ems measure calibrates against — `QApplication::font()`
        /// in production, `DejaVu Sans` under a test (D5's own rule).
        [[nodiscard]] virtual QFont applicationFont() const = 0;

        virtual void announce(const std::string& message) = 0;

        /// Places playback on `index` of `project`'s own page — does nothing
        /// if that project has no film. `Preview`, D8.
        ///
        /// **`const`, unlike a first reading of D8 might suggest.** Every
        /// project this ever names comes from `Session::project()` (const by
        /// its own rule) or from `ProposedCorrection::project` (`const
        /// Project*`, since `proposeCorrections` touches nothing) — nothing
        /// on this road ever holds a mutable one, and `preview` only compares
        /// identity to find the page, never writes through it.
        virtual void preview(const core::Project& project, core::SubtitleIndex index) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// `prompts` and `view` must outlive this.
    CorrectionController(Prompts& prompts, View& view);

    /// Opens the assistant. Settings persist only if it was finished, never
    /// if it was cancelled.
    void open();

    [[nodiscard]] const core::CorrectionSettings& settings() const { return m_settings; }

    void setSettings(core::CorrectionSettings settings) { m_settings = std::move(settings); }

private:
    Prompts* m_prompts;
    View* m_view;
    core::CorrectionSettings m_settings;
};

} // namespace subedit::gui
