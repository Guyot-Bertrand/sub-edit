#pragma once

#include <array>
#include <memory>
#include <span>

class QAction;
class QMainWindow;
class QMenu;
class QObject;

namespace subedit::core {
enum class LetterCase;
} // namespace subedit::core

namespace subedit::gui {

/// Every action of the window, its shortcuts, and where the menus and the
/// toolbar put it — issue #483, ADR 0034.
///
/// **It builds and lays out; it does not act.** No action is connected here:
/// what a gesture does is the window's, which connects each one to its own
/// slot. Nor does it decide when an action is out — the window recomputes that
/// after every operation. What this holds is what never changes once built: the
/// labels, the icons, the shortcuts, and the state an action starts in.
///
/// A plain aggregate of pointers, read by name: the window writes
/// `m_actions->undo` where it wrote `m_undo`. The actions belong to the owner
/// given to the constructor, as Qt's parent — this only points at them.
struct WindowActions final {
    /// Builds every action, owned by `owner` for their lifetime.
    ///
    /// **Out on arrival**, save those that need nothing open to work — opening,
    /// saving, a new project, `Save All`, `Close All`, choosing a film, the
    /// preferences, `About`, and moving between tabs. The window lights the rest
    /// on its first refresh.
    explicit WindowActions(QObject* owner);

    /// Out of line, where `QMenu` is complete: a `unique_ptr` of an incomplete type would make
    /// every file that destroys one include it.
    ~WindowActions();

    WindowActions(const WindowActions&) = delete;
    WindowActions& operator=(const WindowActions&) = delete;
    WindowActions(WindowActions&&) = delete;
    WindowActions& operator=(WindowActions&&) = delete;

    /// Lays the menu bar, the toolbar and the two tab shortcuts on `window`.
    ///
    /// `columns` are the entries of `View ▸ Columns`, which `TableColumns` owns
    /// and this only files under their menu.
    void placeIn(QMainWindow& window, std::span<QAction* const> columns) const;

    /// The entry that puts the target in `wanted`.
    [[nodiscard]] QAction* caseAction(core::LetterCase wanted) const;

    // `File`, and the tabs.
    QAction* newProject = nullptr;
    QAction* open = nullptr;
    QAction* openTranslation = nullptr;
    QAction* save = nullptr;
    QAction* saveAs = nullptr;
    QAction* saveTranslation = nullptr;
    QAction* saveTranslationAs = nullptr;
    QAction* closeProject = nullptr;
    QAction* nextTab = nullptr;
    QAction* previousTab = nullptr;

    // `Edit`.
    QAction* undo = nullptr;
    QAction* redo = nullptr;
    QAction* cut = nullptr;
    QAction* copy = nullptr;
    QAction* paste = nullptr;
    QAction* findAndReplace = nullptr;
    QAction* insert = nullptr;
    QAction* remove = nullptr;
    QAction* mergeSubtitles = nullptr;
    QAction* splitSubtitle = nullptr;
    QAction* preferences = nullptr;

    // `View`.
    QAction* framePositions = nullptr;

    // `Video`.
    QAction* selectVideo = nullptr;
    QAction* playPause = nullptr;
    QAction* playSelection = nullptr;
    QAction* seekPrevious = nullptr;
    QAction* seekNext = nullptr;
    QAction* seekBackward = nullptr;
    QAction* seekForward = nullptr;
    QAction* seekSelectionStart = nullptr;
    QAction* seekSelectionEnd = nullptr;
    QAction* stepBackward = nullptr;
    QAction* stepForward = nullptr;
    QAction* nudgeStartEarlier = nullptr;
    QAction* nudgeStartLater = nullptr;
    QAction* nudgeEndEarlier = nullptr;
    QAction* nudgeEndLater = nullptr;
    QAction* setStartFromVideo = nullptr;
    QAction* setEndFromVideo = nullptr;
    QAction* insertAtVideo = nullptr;
    QAction* selectPreviousFromVideo = nullptr;
    QAction* selectNextFromVideo = nullptr;

    // `Video ▸ Audio`.
    QAction* volumeDown = nullptr;
    QAction* volumeUp = nullptr;

    /// `Video ▸ Audio ▸ Language`, whose entries are the tracks of the film — issue #616. Held
    /// here and not parented to the window, which a menu cannot be to a plain `QObject`; it goes
    /// before the menu bar that shows it does, and takes its entry with it.
    std::unique_ptr<QMenu> audioLanguage;

    // `Tools`.
    QAction* shift = nullptr;
    QAction* transform = nullptr;
    QAction* frameRate = nullptr;
    QAction* adjustDurations = nullptr;
    QAction* appendFile = nullptr;
    QAction* splitProject = nullptr;
    QAction* italic = nullptr;
    QAction* dialogueDashes = nullptr;
    /// In the order of `core::kLetterCases`, which is Gaupol's.
    std::array<QAction*, 4> letterCase{};
    QAction* hearingImpaired = nullptr;
    QAction* correctTexts = nullptr;
    QAction* checkSpelling = nullptr;
    QAction* spellCheckSettings = nullptr;
    QAction* snap = nullptr;
    QAction* shiftOntoGrid = nullptr;
    QAction* analyseGrid = nullptr;

    // `Projects`.
    QAction* saveAllDocuments = nullptr;
    QAction* closeAllProjects = nullptr;

    // `Help`.
    QAction* manual = nullptr;
    QAction* about = nullptr;
};

} // namespace subedit::gui
