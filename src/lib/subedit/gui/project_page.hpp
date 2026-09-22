#pragma once

#include <subedit/core/edit/search.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/selection.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace subedit::core {
class Session;
} // namespace subedit::core

namespace subedit::gui {

class SubtitleTableModel;

/// Everything a project owns, apart from the window around it — ADR 0033,
/// decision D2 of the phase-11 spec.
///
/// **The window owns exactly one, for now** — issue #436, which moves this
/// state out of `MainWindow` without adding a tab to show a second one: **it
/// changes nothing a user can see**, and every test of the window keeps
/// passing unrewritten. A second page, and the tab that would show it, are
/// #437.
///
/// **What stays at the window is written in the ADR, and is not here**: the
/// menus and their actions, the status bar, the title; the player, the video
/// view, the follow timer — one of each, shared, its film changed with the
/// page rather than duplicated; the theme, the directories, the settings of
/// an adjustment; the clipboard, the search dialog and its two options. A
/// project reaching for one of those would be reaching the wrong way.
struct ProjectPage {
    ProjectPage();
    ~ProjectPage();

    ProjectPage(const ProjectPage&) = delete;
    ProjectPage& operator=(const ProjectPage&) = delete;
    ProjectPage(ProjectPage&&) = delete;
    ProjectPage& operator=(ProjectPage&&) = delete;

    /// Held by pointer so that a header naming `ProjectPage` — `main_window.hpp`
    /// among them — stays parsable by `moc`, which chokes on the C++20 library
    /// headers `session.hpp` drags in.
    std::unique_ptr<core::Session> session;
    std::unique_ptr<SubtitleTableModel> model;

    /// The row playback was last placed at, or -1.
    int placedAt = -1;

    /// The film the window last acted on for this project, whether or not it
    /// opened.
    ///
    /// Distinct from `watching` on purpose: a film that was refused must not
    /// be offered to the player again — and refused again, and reported again
    /// — every time the naming convention speaks.
    std::filesystem::path associated;

    /// Whether a film is open and being drawn for this project.
    bool watching = false;

    /// The line the overlay currently carries for this project.
    ///
    /// Held so that a tick that changes nothing costs nothing: the replica is
    /// recomputed from the project ten times a second, and it is only handed
    /// over when it differs — which is also what makes a keystroke show up on
    /// the picture within a tick.
    std::string shown;

    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document
    /// changes — including a structural undo or redo, which resets the model
    /// rather than reporting the change — and when the column of the current
    /// cell changes to the other text.
    std::optional<core::TextMatch> match;

    /// The text the search last aimed at, which is what tells a change of
    /// column that changes the text from one that does not.
    ///
    /// **A match is a place in one text.** The translation of a subtitle may
    /// read exactly what the main text does, so a match kept across the change
    /// would be replaced there without the user having seen it found.
    core::Document searchDocument = core::Document::Main;

    /// The target of the search under way, captured at its first gesture.
    ///
    /// **Captured and not read again**, because the search itself moves the
    /// selection: read at every `Find Next`, the target would shrink to the row
    /// of the last match. A selection the user makes resets it.
    std::optional<core::Selection> searchTarget;

    /// Set while the search moves the selection, so that the move is not
    /// mistaken for the user choosing another target.
    bool movingToMatch = false;

    /// The encoding last chosen in `Save As…` for this project, absent until
    /// one has been.
    ///
    /// **It serves the document with no file, and nothing else.** An opened
    /// document carries its own, and that is the one the box proposes: the byte
    /// round trip of phase 8 is that promise, and a setting does not undo it
    /// behind the back of whoever saves.
    std::optional<core::Encoding> writeEncoding;
};

} // namespace subedit::gui
