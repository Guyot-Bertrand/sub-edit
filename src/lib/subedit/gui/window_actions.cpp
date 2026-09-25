#include <subedit/core/text/letter_case.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/window_actions.hpp>

#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QString>
#include <QToolBar>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <span>
#include <utility>

namespace subedit::gui {

namespace {

/// Builds one action, named for the toolbar and for the menu, and out.
///
/// `text` is what the menu reads and it changes at every operation —
/// « Undo: shifting ». `iconText` is what the toolbar button reads and it never
/// changes: a button whose width followed the last operation would move under
/// the pointer.
[[nodiscard]] QAction*
buildAction(QObject* parent, const QString& shortName, const QString& themeIcon) {
    auto* action = new QAction{QIcon::fromTheme(themeIcon), shortName, parent};
    action->setIconText(shortName);
    action->setEnabled(false);
    return action;
}

/// The same, lit from the start: an action that needs nothing open to work.
[[nodiscard]] QAction*
buildLitAction(QObject* parent, const QString& shortName, const QString& themeIcon) {
    QAction* action = buildAction(parent, shortName, themeIcon);
    action->setEnabled(true);
    return action;
}

/// The shortcuts of `Save As…`, one of which the platform may not give.
///
/// **The platform theme gives `Ctrl+Shift+S` on every desktop** — measured
/// under xcb, under wayland, and under `offscreen` as soon as a theme is laid
/// down. With no theme, Qt gives none: its internal table defines `SaveAs` for
/// macOS and Windows alone, and that is the table a test binary meets.
///
/// The conventional binding is therefore added when the platform says nothing —
/// issue #274. This is not deciding in its stead: it is saying the same thing
/// it does where it speaks, and not leaving a destructive command out of reach
/// of the keyboard where it says nothing.
[[nodiscard]] QList<QKeySequence> saveAsShortcuts() {
    static const QKeySequence conventional{QStringLiteral("Ctrl+Shift+S")};

    QList<QKeySequence> given = QKeySequence::keyBindings(QKeySequence::SaveAs);
    if (!given.contains(conventional))
        given.append(conventional);

    return given;
}

/// How many buttons the toolbar carries.
constexpr std::size_t kToolbarWords = 10;

} // namespace

WindowActions::WindowActions(QObject* owner)
    : newProject(
          buildLitAction(owner, QStringLiteral("&New Project"), QStringLiteral("document-new"))),
      open(buildLitAction(owner, QStringLiteral("Open…"), QStringLiteral("document-open"))),
      openTranslation(buildAction(owner, QStringLiteral("Open &Translation…"), {})),
      save(buildLitAction(owner, QStringLiteral("Save"), QStringLiteral("document-save"))),
      saveAs(buildLitAction(owner, QStringLiteral("Save As…"), QStringLiteral("document-save-as"))),
      saveTranslation(buildAction(owner, QStringLiteral("Save Tr&anslation"), {})),
      saveTranslationAs(buildAction(owner, QStringLiteral("Save Translation As…"), {})),
      closeProject(buildAction(owner, QStringLiteral("&Close"), QStringLiteral("window-close"))),
      nextTab(new QAction{owner}),
      previousTab(new QAction{owner}),
      undo(buildAction(owner, QStringLiteral("Undo"), QStringLiteral("edit-undo"))),
      redo(buildAction(owner, QStringLiteral("Redo"), QStringLiteral("edit-redo"))),
      cut(buildAction(owner, QStringLiteral("Cu&t Texts"), QStringLiteral("edit-cut"))),
      copy(buildAction(owner, QStringLiteral("&Copy Texts"), QStringLiteral("edit-copy"))),
      paste(buildAction(owner, QStringLiteral("&Paste Texts"), QStringLiteral("edit-paste"))),
      findAndReplace(buildAction(
          owner, QStringLiteral("&Find and Replace…"), QStringLiteral("edit-find-replace"))),
      insert(buildAction(owner, QStringLiteral("Insert Subtitles…"), QStringLiteral("list-add"))),
      remove(buildAction(owner, QStringLiteral("Remove Subtitles"), QStringLiteral("list-remove"))),
      mergeSubtitles(buildAction(owner, QStringLiteral("&Merge Subtitles"), {})),
      splitSubtitle(buildAction(owner, QStringLiteral("S&plit Subtitle"), {})),
      preferences(new QAction{QStringLiteral("&Preferences…"), owner}),
      selectVideo(buildLitAction(owner, QStringLiteral("Select Video…"), {})),
      playPause(buildAction(
          owner, QStringLiteral("Play / Pause"), QStringLiteral("media-playback-start"))),
      shift(buildAction(owner, QStringLiteral("Shift Positions…"), {})),
      transform(buildAction(owner, QStringLiteral("Transform Positions…"), {})),
      frameRate(buildAction(owner, QStringLiteral("Convert Frame Rate…"), {})),
      adjustDurations(buildAction(owner, QStringLiteral("Adjust Durations…"), {})),
      appendFile(buildAction(owner, QStringLiteral("Append &File…"), {})),
      splitProject(buildAction(owner, QStringLiteral("Spli&t Project…"), {})),
      italic(buildAction(owner, QStringLiteral("&Italic"), QStringLiteral("format-text-italic"))),
      dialogueDashes(buildAction(owner, QStringLiteral("&Dialogue"), {})),
      hearingImpaired(buildAction(owner, QStringLiteral("Remove Hearing-Impaired Mentions…"), {})),
      snap(buildAction(owner, QStringLiteral("Snap to Frame Rate…"), {})),
      shiftOntoGrid(buildAction(owner, shiftOntoGridLabel(std::nullopt), {})),
      analyseGrid(buildAction(owner, QStringLiteral("Frame Rate &Analysis…"), {})),
      saveAllDocuments(buildLitAction(owner, QStringLiteral("&Save All"), {})),
      closeAllProjects(buildLitAction(owner, QStringLiteral("&Close All"), {})),
      manual(buildAction(owner, QStringLiteral("&Manual"), {})),
      about(new QAction{QStringLiteral("&About subedit"), owner}) {
    // **Gaupol's four, in Gaupol's order** — `Text ▸ Case` offers Title,
    // Sentence, Upper, Lower, and a user who knows one knows the other.
    static constexpr std::array<const char*, 4> kCaseLabels = {
        "&Title Case", "&Sentence case", "&UPPER CASE", "&lower case"};
    for (std::size_t which = 0; which < letterCase.size(); ++which)
        letterCase.at(which) = buildAction(owner, QString::fromUtf8(kCaseLabels.at(which)), {});

    newProject->setToolTip(QStringLiteral("Open an empty project in a new tab"));

    // **Every binding the platform gives "redo", and not the first** — issue
    // #274.
    //
    // What `QKeySequence` answers depends on the platform theme, and a test
    // binary has none: under `offscreen`, Qt falls back on its internal table
    // and puts `Ctrl+Y` at the head; under any desktop at all, the theme gives
    // `Ctrl+Shift+Z` and nothing else. `setShortcut` keeps only the first, so
    // one line of code laid down two different shortcuts depending on where it
    // ran — and the test saw only the one the user does not have.
    // `setShortcuts` takes them all: both work everywhere.
    undo->setShortcut(QKeySequence::Undo);
    redo->setShortcuts(QKeySequence::keyBindings(QKeySequence::Redo));

    open->setShortcut(QKeySequence::Open);
    newProject->setShortcut(QKeySequence::New);
    closeProject->setShortcut(QKeySequence::Close);
    save->setShortcut(QKeySequence::Save);
    saveAs->setShortcuts(saveAsShortcuts());
    saveAllDocuments->setShortcut(QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_L});
    closeAllProjects->setShortcut(QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_W});

    // **`Ctrl+PageDown` and `Ctrl+PageUp`**, the platform's own for moving
    // between tabs — no `QKeySequence::StandardKey` names them, so they are
    // written out, as Gaupol's own binding is.
    nextTab->setShortcut(QKeySequence{QStringLiteral("Ctrl+PgDown")});
    previousTab->setShortcut(QKeySequence{QStringLiteral("Ctrl+PgUp")});

    // **`Ins` and `Del`, and not Gaupol's letters.** It gives `I` and
    // `Delete`; a bare letter of window scope would be taken before the editor
    // of a cell saw it, which the `Ctrl+P` of the player already explains. The
    // two editing keys, for their part, are claimed by Qt's input fields for as
    // long as an editor is open: that is what lets `Del` erase a character
    // rather than a subtitle.
    insert->setShortcut(QKeySequence{Qt::Key_Insert});
    remove->setShortcut(QKeySequence::Delete);

    // **No shortcut for merging and splitting, where Gaupol has `M` and `S`.**
    // A bare letter of window scope would be taken before a cell editor saw it,
    // and the `Ctrl` forms are spoken for: `Ctrl+S` saves. Two entries one
    // reaches by the menu are better than a key that types a letter into the
    // wrong place.

    // **The platform's three, as Gaupol has them.** No conflict with a cell
    // editor: a text field claims these sequences for as long as it has the
    // focus, the way it claims `Del`, so inside an open cell they copy and
    // paste characters rather than subtitles.
    cut->setShortcut(QKeySequence::Cut);
    copy->setShortcut(QKeySequence::Copy);
    paste->setShortcut(QKeySequence::Paste);

    // `Ctrl+F`, Gaupol's. A cell editor does not claim it, so it opens the
    // dialog from inside an open cell too.
    findAndReplace->setShortcut(QKeySequence::Find);

    // **`Ctrl+I` and not a bare `I`**, for the reason the player's `Ctrl+P`
    // already carries: a one-letter shortcut of window scope is taken before
    // the cell editor sees it, and this table has three columns one types in.
    italic->setShortcut(QKeySequence{QStringLiteral("Ctrl+I")});

    // **`Ctrl+P` where Gaupol has a bare `P`**, and the difference is not
    // taste. A one-letter shortcut of window scope is taken before the widget
    // that has the focus sees it, so a `P` would be swallowed on its way into
    // a cell editor — and this table has three columns one types in. Nothing
    // prints here, so the sequence is free.
    playPause->setShortcut(QKeySequence{QStringLiteral("Ctrl+P")});

    // **Out for as long as nobody has said where the manual is**, which is the
    // case of a binary run from the build tree: `main` calls `setManualPath`
    // with what `installedManualPath()` resolved, and the entry lights up if
    // the manual is there. An entry that opened emptiness would be worse than
    // an entry saying it has nothing to open.
    manual->setShortcut(QKeySequence::HelpContents);
}

QAction* WindowActions::caseAction(core::LetterCase wanted) const {
    const auto* const found = std::ranges::find(core::kLetterCases, wanted);
    return letterCase.at(
        static_cast<std::size_t>(std::distance(std::ranges::begin(core::kLetterCases), found)));
}

void WindowActions::placeIn(QMainWindow& window, std::span<QAction* const> columns) const {
    // `addAction` on the window and not a menu: the bar of tabs already offers
    // a click, and these are for whoever would rather not reach for the mouse.
    window.addAction(nextTab);
    window.addAction(previousTab);

    // **The menu bar, in the order a user reads it**: the document, what one
    // does to it, what accompanies it, what inspects it, what explains it.
    // Reading order and not construction order — the two had drifted apart, and
    // it is the first that a user meets.
    QMenu* file = window.menuBar()->addMenu(QStringLiteral("&File"));
    file->addAction(newProject);
    file->addAction(open);
    file->addAction(openTranslation);
    file->addSeparator();
    file->addAction(save);
    file->addAction(saveAs);
    // The translation is a file of its own, and so are the entries that write
    // it: under the two of the main document, where a user looking for how to
    // save will look first.
    file->addSeparator();
    file->addAction(saveTranslation);
    file->addAction(saveTranslationAs);
    // Below everything the document itself offers: closing is what one does
    // to the tab, not to what it holds.
    file->addSeparator();
    file->addAction(closeProject);

    QMenu* edition = window.menuBar()->addMenu(QStringLiteral("&Edit"));
    edition->addAction(undo);
    edition->addAction(redo);
    edition->addSeparator();
    // Where every program puts them, and above the edits of structure: they
    // move texts, and never add or take away a row — save a paste that runs
    // past the end.
    edition->addAction(cut);
    edition->addAction(copy);
    edition->addAction(paste);
    edition->addSeparator();
    edition->addAction(findAndReplace);
    edition->addSeparator();
    // Under a separator: undoing is what one does *to* an edit; inserting and
    // removing *are* edits.
    edition->addAction(insert);
    edition->addAction(remove);
    // Beside them: merging and splitting change how many rows there are, as
    // inserting and removing do, and Gaupol keeps the four together.
    edition->addAction(mergeSubtitles);
    edition->addAction(splitSubtitle);
    edition->addSeparator();
    // Under another: setting the theme is no edit at all.
    edition->addAction(preferences);

    // Born with the translation column, and where the other columns would sit
    // one day — issue #442. After `Edit` and before `Video`: what one does to
    // the document, then how one looks at it, then what accompanies it.
    QMenu* view = window.menuBar()->addMenu(QStringLiteral("&View"));
    // A submenu, as Gaupol's `View ▸ Columns`: five entries loose in the menu
    // would be five entries for one question.
    QMenu* columnMenu = view->addMenu(QStringLiteral("&Columns"));
    for (QAction* entry : columns)
        columnMenu->addAction(entry);

    QMenu* video = window.menuBar()->addMenu(QStringLiteral("&Video"));
    video->addAction(selectVideo);
    video->addSeparator();
    video->addAction(playPause);

    QMenu* tools = window.menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(shift);
    tools->addAction(transform);
    tools->addAction(frameRate);
    // With the operations on positions: it moves ends, and nothing else.
    tools->addAction(adjustDurations);
    tools->addSeparator();
    // On its own: it adds subtitles rather than editing the ones already
    // there — Gaupol's own placement, next to the position operations and
    // apart from the two that follow.
    tools->addAction(appendFile);
    // Its inverse, beside it.
    tools->addAction(splitProject);
    tools->addSeparator();
    // Those that rewrite a text rather than move a position, together.
    tools->addAction(italic);
    tools->addAction(dialogueDashes);
    // A submenu for the four, which is what Gaupol does: four entries side by
    // side in a menu of nine would drown the rest.
    QMenu* caseMenu = tools->addMenu(QStringLiteral("Ca&se"));
    for (QAction* one : letterCase)
        caseMenu->addAction(one);
    tools->addAction(hearingImpaired);
    tools->addSeparator();
    // The two of phase 16, together: one lays each position on the nearest
    // frame, the other moves the whole file back onto its own grid. They read
    // alike and are not alike, which is why they sit side by side rather than
    // among the four above.
    tools->addAction(snap);
    tools->addAction(shiftOntoGrid);
    tools->addSeparator();
    // Below the separator because it changes nothing: the four above it act on
    // the document, this one only reports on it.
    tools->addAction(analyseGrid);

    // What acts on every project at once. Gaupol's menu of the same name also
    // lists the tabs and has `Save All As…`; the first is left to the tab bar
    // and the second is a series of `Save As…` that `Save All` already asks.
    QMenu* projects = window.menuBar()->addMenu(QStringLiteral("&Projects"));
    projects->addAction(saveAllDocuments);
    projects->addAction(closeAllProjects);

    QMenu* help = window.menuBar()->addMenu(QStringLiteral("&Help"));
    help->addAction(manual);
    help->addSeparator();
    help->addAction(about);

    // **The frequent gestures, in groups** — issue #474. Gaupol's bar carries
    // opening, saving, undoing and redoing, and finding; its player gets a bar
    // of its own. This one adds what the user reaches for most while editing —
    // a new project, inserting, removing — and the player's one button, and
    // keeps `Italic`, which was on it first. A gesture that opens a box is no
    // reason to keep it off: the box is what the button leads to.
    //
    // Each button reads a short word of its own; the menus keep the whole
    // entry. The table of what differs from Gaupol, and why, is in spec 11.
    const std::array<std::pair<QAction*, QString>, kToolbarWords> words{{
        {newProject, QStringLiteral("New")},
        {open, QStringLiteral("Open")},
        {save, QStringLiteral("Save")},
        {undo, QStringLiteral("Undo")},
        {redo, QStringLiteral("Redo")},
        {findAndReplace, QStringLiteral("Find")},
        {insert, QStringLiteral("Insert")},
        {remove, QStringLiteral("Remove")},
        {italic, QStringLiteral("Italic")},
        {playPause, QStringLiteral("Play")},
    }};
    for (const auto& [action, word] : words)
        action->setIconText(word);

    QToolBar* bar = window.addToolBar(QStringLiteral("Main"));
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->addAction(newProject);
    bar->addAction(open);
    bar->addAction(save);
    bar->addSeparator();
    bar->addAction(undo);
    bar->addAction(redo);
    bar->addSeparator();
    bar->addAction(findAndReplace);
    bar->addSeparator();
    bar->addAction(insert);
    bar->addAction(remove);
    bar->addAction(italic);
    bar->addSeparator();
    bar->addAction(playPause);
}

} // namespace subedit::gui
