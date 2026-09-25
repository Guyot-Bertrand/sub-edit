#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/gui/player_factory.hpp>

#include <memory>
#include <optional>
#include <span>

class QAbstractButton;
class QSplitter;
class QTimer;
class QWidget;

namespace subedit::core {
enum class Document;
class FileSystem;
class VideoPlayer;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ProjectPage;
class SubtitleTable;

/// The film, the player and the picture — ADR 0034, issue #484.
///
/// **It owns what the video is made of**: the shared player, built the first
/// time a film needs one; the native surface it draws into and the band that
/// stands in for it while there is no film; the ticker that keeps the window in
/// step with playback; and which page the player currently plays for. Every
/// gesture receives the page it is about — the one on screen, in practice,
/// since there is one picture.
///
/// **What it reads of the table it is given**, and what it asks of the window
/// goes through its `View`: which text the replica shows, and whether playback
/// can be driven at all. The boxes it opens go through `Prompts`.
class VideoPane final {

public:
    /// What the video asks of the window.
    class View {

    public:
        virtual ~View() = default;

        /// The text an operation of text aims at — the replica shows that one.
        [[nodiscard]] virtual core::Document targetDocument() const = 0;

        /// Whether a film is open and playing can be driven — the window lights
        /// or puts out what drives it.
        virtual void playable(bool playable) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// Builds the surface and the band, puts them at the top of `split` — the
    /// window adds the table under them — and waits for a film.
    ///
    /// `files`, `prompts`, `view`, `table` and `split` must outlive this. The
    /// widgets and the ticker belong to `owner`. `buildPlayer` and
    /// `readDeclaredRate` are the two seams of `MainWindow`'s constructor, and
    /// are optional in the same way.
    VideoPane(core::FileSystem& files,
              Prompts& prompts,
              View& view,
              SubtitleTable& table,
              QSplitter& split,
              PlayerFactory buildPlayer,
              FrameRateReader readDeclaredRate,
              QWidget* owner);

    ~VideoPane();

    VideoPane(const VideoPane&) = delete;
    VideoPane& operator=(const VideoPane&) = delete;
    VideoPane(VideoPane&&) = delete;
    VideoPane& operator=(VideoPane&&) = delete;

    /// The surface the film is drawn on. Hidden while no film is open.
    [[nodiscard]] QWidget* picture() const { return m_picture; }

    /// What stands where the picture would be while there is no film.
    [[nodiscard]] QWidget* banner() const { return m_banner; }

    /// The button of the band, `Select Video…` — the window connects it.
    [[nodiscard]] QAbstractButton* invite() const { return m_invite; }

    /// Asks which film to watch `page` against, and associates it. Says whether
    /// one was chosen; the window then `watch`es it and says so.
    [[nodiscard]] bool choose(ProjectPage& page);

    /// Offers `page` the film the naming convention finds beside its file.
    ///
    /// A choice already made is never replaced: D5 lives in `Project`, so
    /// calling this too often costs nothing but a look at a directory.
    void proposeBeside(ProjectPage& page);

    /// Puts the picture in step with the film `page` is now associated with,
    /// and makes `page` the one the player plays for.
    ///
    /// **A film that will not open is said, named, and then let go.** Nothing
    /// else changes: the document is still there, and the association still
    /// stands — the user may well want to see which file the player refused.
    ///
    /// Cheap when nothing changed: a film already open for this same page is
    /// not opened again. Does nothing before `windowShown`.
    void watch(ProjectPage& page);

    /// The window is on screen for the first time: opens the film of `page`.
    ///
    /// **The film waits for this, and it is not a refinement.** libmpv adopts
    /// the window it is handed at the moment it loads a file; handed one that
    /// is not on screen yet, it adopts it and never maps its own — measured,
    /// mpv's window stays `IsUnMapped` for the life of the process.
    void windowShown(ProjectPage& page);

    /// `page` is about to leave the screen: where its film stands is kept, to
    /// take it back there on its return — issue #471.
    void leave(ProjectPage& page);

    /// `page` is about to be freed: the player no longer plays for it.
    void forget(const ProjectPage& page);

    /// Lets the player go, with the film it holds — before the surface it
    /// draws into is destroyed, never after (#470). `pages` forget their film,
    /// so that a window shown again opens it anew.
    void release(std::span<const std::unique_ptr<ProjectPage>> pages);

    /// Plays, or holds where it is — the player is the one that knows which.
    void toggle(const ProjectPage& page);

    /// Places playback at the start of the first selected subtitle of `page`.
    ///
    /// **Only when that first row changes**, and that is not a refinement:
    /// extending a selection downwards over four thousand rows fires this at
    /// every step, and a seek waits for the player to arrive.
    void placeAtSelection(ProjectPage& page);

    /// Reads where playback stands and puts `page` in step with it: the replica
    /// drawn over the picture, and the row the table points at.
    ///
    /// **It gives way to whoever is typing.** Moving the current row closes an
    /// open editor; while a cell is being edited the row stays where it is and
    /// the replica still follows.
    void follow(ProjectPage& page);

    /// How long the film of `page` lasts, or nothing — nothing too for a page
    /// whose film is not the one the shared player has open.
    [[nodiscard]] std::optional<core::Duration> length(const ProjectPage& page) const;

private:
    /// Returns the player, building it the first time one is needed.
    ///
    /// Nothing, when no factory was given or when the factory declined. Asked
    /// **once**: a libmpv that would not give a player will not give one on
    /// the second film either.
    [[nodiscard]] core::VideoPlayer* player();

    /// Shows the picture, or the band that invites one, in the room above the
    /// table — exactly one of the two, and the room goes with it.
    ///
    /// **The splitter keeps a size for each child, shown or not**: swapping
    /// which one is visible without moving the room left the picture with the
    /// size the settings gave it while hidden, that is none — issue #469.
    void showPicture(bool picture);

    core::FileSystem* m_files;
    Prompts* m_prompts;
    View* m_view;
    SubtitleTable* m_table;
    QSplitter* m_split;
    PlayerFactory m_buildPlayer;
    FrameRateReader m_readDeclaredRate;

    QWidget* m_picture = nullptr;
    QWidget* m_banner = nullptr;
    QAbstractButton* m_invite = nullptr;
    QTimer* m_ticker = nullptr;

    std::unique_ptr<core::VideoPlayer> m_player;
    bool m_playerAsked = false;

    /// Whether the window has been on screen once. Nothing is handed to a
    /// player before it has: see `windowShown`.
    bool m_windowShown = false;

    /// The page whose film the shared player currently has open, or nothing.
    ///
    /// **Distinct from the page on screen.** `watch`'s own guard — « the
    /// association has not changed, do nothing » — is right for one project
    /// and wrong for several: switching to a page whose association has not
    /// changed *since it was last shown* still means the player is showing
    /// someone else's film. Comparing against this is what forces the reopen
    /// a switch of tab needs.
    ProjectPage* m_playingPage = nullptr;
};

} // namespace subedit::gui
