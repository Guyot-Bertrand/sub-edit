#pragma once

#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/time/duration.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>

class QWidget;

namespace subedit::core {
class Command;
enum class CommandKind;
struct Diagnostic;
enum class Document;
enum class LetterCase;
class Project;
class Selection;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ProjectPage;

/// Two sentences for one box, one to a line, and whichever is empty left out —
/// issue #418: one operation, one box.
///
/// What was done comes first and what it left past the end of the film after
/// it: the second is a warning about the first, and is read as one.
[[nodiscard]] std::string joinedNotices(const std::string& done, const std::string& warning);

/// The operations of `Tools`, and the one road from an operation to the
/// history — ADR 0035, issue #486.
///
/// **Each gesture receives the page it is about**, and reads its target in that
/// page's own selection: the rows selected, or the whole file when nothing is.
/// Each one follows the same path — a box, a command the core builds, `apply`,
/// then an account — and none of them keeps anything of a page.
///
/// **It keeps one thing of its own**: the form of the last adjustment of
/// durations, which the next one offers again and the settings carry from one
/// session to the next.
///
/// What belongs to the screen it asks of the window through its `View`, and
/// the boxes go through `Prompts`.
class ProjectOperations final {

public:
    /// What the operations ask of the window.
    class View {

    public:
        virtual ~View() = default;

        /// What the boxes this opens sit over.
        [[nodiscard]] virtual QWidget* dialogParent() = 0;

        /// The text an operation of text aims at.
        [[nodiscard]] virtual core::Document targetDocument() const = 0;

        /// How long the film of `page` lasts, or nothing — what an operation
        /// that moves positions is measured against.
        [[nodiscard]] virtual std::optional<core::Duration>
        videoLength(const ProjectPage& page) const = 0;

        /// Says in the status bar what a gesture did, when there is nothing else
        /// to say — the rule #398 set.
        virtual void announce(const std::string& message) = 0;

        /// Asks which file to append, from the remembered directory.
        [[nodiscard]] virtual std::optional<std::filesystem::path> fileToAppend() = 0;

        /// Reads `path`, or says why it will not open, in the words `Open…`
        /// uses.
        [[nodiscard]] virtual std::expected<core::OpenedFile, std::string>
        read(const std::filesystem::path& path) = 0;

        /// Shows what a reading ran into, in the panel under the table.
        virtual void showDiagnostics(std::span<const core::Diagnostic> diagnostics) = 0;

        /// Selects the rows from `first` to `last` of the page on screen, and
        /// brings them into view.
        virtual void selectRows(int first, int last) = 0;

        /// Opens `project` in a tab of its own, as holding subtitles no file
        /// has: closing it must ask.
        virtual void openAside(core::Project project) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// `prompts` and `view` must outlive this.
    ProjectOperations(Prompts& prompts, View& view);

    /// Applies `command` to `page`, over `target`, and says in a box what it
    /// left past the end of the film.
    ///
    /// **`page` need not be the one on screen** — issue #461: `Replace All`
    /// over every project reaches each page without bringing its tab forward.
    ///
    /// The one road from a dialog to the history: every operation ends here,
    /// so the notice below cannot be forgotten in one of them. `target` is what
    /// the operation was applied to: what reaches past the end of the film is
    /// read over it, after the fact, on the state the operation produced.
    void
    apply(ProjectPage& page, std::unique_ptr<core::Command> command, const core::Selection& target);

    /// The same, and it says nothing: what the operation left past the end of
    /// the film comes back as the sentence to say, empty when there is none.
    ///
    /// **Why the box is not opened here**: `reportOutcome` is modal, and an
    /// operation with an account of its own to give used to open a second one
    /// straight after the first. One operation, one box — issue #418.
    [[nodiscard]] std::string applyQuietly(ProjectPage& page,
                                           std::unique_ptr<core::Command> command,
                                           const core::Selection& target);

    /// `Shift Positions…`.
    void shift(ProjectPage& page);

    /// `Transform Positions…`.
    void transform(ProjectPage& page);

    /// `Convert Frame Rate…`.
    void convertFrameRate(ProjectPage& page);

    /// Asks for the four constraints, applies them to the target, and says what
    /// no end could satisfy.
    ///
    /// **Said even when nothing moved**: a target already at its gaps may still
    /// hold subtitles too short for their minimum, and « nothing to adjust »
    /// alone would let that pass for « everything is fine ».
    void adjustDurations(ProjectPage& page);

    /// Asks for a file, and appends it to the end of the project — D6.
    void appendFile(ProjectPage& page);

    /// Asks where to cut, and moves the tail into a project of its own, in a
    /// new tab — D6.
    void splitProject(ProjectPage& page);

    /// `Remove Hearing-Impaired Mentions…`.
    void removeHearingImpaired(ProjectPage& page);

    /// Puts the target in italics, or takes its italics out.
    ///
    /// **One entry and not two**, as in Gaupol: which of the two it does is
    /// read from the target before anything is built, and a mixed selection
    /// goes into italics whole.
    void toggleItalics(ProjectPage& page);

    /// Puts the target in `wanted`.
    void changeCase(ProjectPage& page, core::LetterCase wanted);

    /// Puts dialogue dashes on the target, or takes them off — read from the
    /// target before anything is built, as for the italic.
    void toggleDialogueDashes(ProjectPage& page);

    /// Asks which grid to lay the positions on, and lays them on it.
    void snap(ProjectPage& page);

    /// Moves the whole file back onto the grid it was written on.
    ///
    /// No dialog: the operation takes no option, and the amount it will use is
    /// already in the menu entry that opened it.
    void shiftOntoGrid(ProjectPage& page);

    /// Opens the analysis, which reports and changes nothing.
    void analyseGrid(const ProjectPage& page);

    /// The form of the last adjustment of durations — Gaupol's defaults until
    /// one is made or the settings give one.
    [[nodiscard]] const core::DurationAdjustmentSettings& durationSettings() const {
        return m_durationSettings;
    }

    void setDurationSettings(const core::DurationAdjustmentSettings& settings) {
        m_durationSettings = settings;
    }

private:
    /// What an operation left past the end of the film, said as a sentence, or
    /// nothing.
    ///
    /// **A notice, never a refusal** — decision D4. A subtitle landing after
    /// the closing credits may be exactly what was meant; refusing wrongly
    /// costs more than a warning that is ignored.
    [[nodiscard]] std::string whatPassesTheEnd(const ProjectPage& page,
                                               core::CommandKind kind,
                                               const core::Selection& target) const;

    Prompts* m_prompts;
    View* m_view;

    /// The form of the last adjustment of durations, offered again by the next
    /// one.
    core::DurationAdjustmentSettings m_durationSettings;
};

} // namespace subedit::gui
