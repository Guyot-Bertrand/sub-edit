#pragma once

#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <memory>
#include <optional>
#include <span>
#include <vector>

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

class PlayBar;
class Prompts;
struct ProjectPage;
class SubtitleTable;
class VideoSurface;

/// The film, the player and the picture — ADR 0034, issue #484.
///
/// **It owns what the video is made of**: the shared player, built the first
/// time a film needs one; the surface that paints its picture and the band that
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

    /// The surface the film is painted on. Hidden while no film is open.
    [[nodiscard]] QWidget* picture() const;

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
    /// **The film waits for this**, so that nothing is handed to a player before
    /// the window has its real size. It used to be a necessity — libmpv adopted a
    /// native window and never mapped its own if that one was not on screen yet —
    /// and it stopped being one with ADR 0041; the single entry it gives the film
    /// stayed.
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

    /// Moves playback by one jump, back when `direction` is negative and forward otherwise — the
    /// length of the jump is `VideoSettings::seekLengthSeconds`, and playback stays between the
    /// start of the film and its end. Gaupol's `Seek Backward` and `Seek Forward`.
    void seekBy(ProjectPage& page, int direction);

    /// Places playback at the start of the next subtitle, or of the previous one — Gaupol's
    /// `Seek Next` and `Seek Previous`, read the way Gaupol reads them: *next* is the first that
    /// starts after the position, *previous* is the last that has ended before it. Nothing when
    /// there is none.
    void seekToNeighbour(ProjectPage& page, bool next);

    /// Places playback at the start of the selection, or at its end, the lead-in before it —
    /// Gaupol's `Seek Selection Start` and `Seek Selection End`. Nothing without a selection.
    void seekToSelection(ProjectPage& page, bool end);

    /// Plays the selection: from its start, the lead-in before it, and **up to the end of the
    /// last subtitle selected** — `playUntil`, so that it stops on the frame and not up to three
    /// frames later. Nothing without a selection.
    void playSelection(ProjectPage& page);

    /// Moves playback by the frame step — back when `direction` is negative, forward otherwise.
    /// The step is a count of frames (`VideoSettings::stepFrames`, one at least), so that a step
    /// of N shows the picture N frames away at 25 as at 23.976; playback stops at the ends of the
    /// film, and is held afterwards.
    ///
    /// **One step at a time**: the first request goes at once, and those that follow within
    /// `kStepGateMs` — a held key repeats faster than a step is made — **keep one place between
    /// them**, the last, which goes when the gate opens. They do not queue: a key held for two
    /// seconds on a slow step would otherwise go on stepping for as long again after it was let go.
    void step(ProjectPage& page, int direction);

    /// Moves the start, or the end, of the first selected subtitle by the frame step, earlier
    /// when `direction` is negative — the same step as `step`, in the same frames, **counted by
    /// the rate of the film, else of the document, else of the grid**. One command, so one undo;
    /// the order and the overlap are said by the table as for a cell. **Without any rate it
    /// refuses, and says so.** The film, when there is one, is placed on the new position.
    void nudge(ProjectPage& page, core::Boundary boundary, int direction);

    /// Sets the start, or the end, of the first selected subtitle to where playback stands —
    /// Gaupol's `Set Start from Video Position` and `Set End from Video Position`. **The same
    /// command as typing the position in the cell**, so the same rules: an end before its start
    /// is let stand and flagged, and one undo takes it back. Nothing without a selection.
    void markEdge(ProjectPage& page, core::Boundary boundary);

    /// Inserts a subtitle that starts where playback stands, three seconds long or up to the
    /// next one when that comes sooner, and selects it — Gaupol's `Insert Subtitle at Video
    /// Position`. It goes where the order puts it: after every subtitle that starts at or before
    /// the position.
    void insertAtPosition(ProjectPage& page);

    /// Selects the subtitle that starts after where playback stands, or the last that started
    /// before it — Gaupol's `Select Next` and `Select Previous from Video Position`. When there
    /// is none on that side, the subtitle at the end of the file that way, as Gaupol does.
    void selectFromPosition(ProjectPage& page, bool next);

    /// Moves the volume by `delta` per cent, within 0 to 100. Gaupol's `Volume Down` and
    /// `Volume Up` are five.
    void changeVolume(int delta);

    /// How the player is driven — what the window keeps from one session to the next. The volume
    /// is the one the bar and the gestures last set.
    [[nodiscard]] core::VideoSettings settings() const;

    /// Lays `settings` down: the jump and the lead-in for the next gestures, the volume at once.
    void setSettings(const core::VideoSettings& settings);

    /// The audio tracks of the film the player has open, or none — for the menu that lists them.
    /// None as well while no film is open, as for a page whose film the shared player does not
    /// hold.
    [[nodiscard]] std::vector<core::AudioTrack> audioTracks() const;

    /// Plays the track `id` of the film, which `audioTracks` named. Nothing when no film is open.
    void selectAudioTrack(int id);

    /// The bar under the picture.
    [[nodiscard]] PlayBar* bar() const { return m_bar; }

    /// How long the film of `page` lasts, or nothing — nothing too for a page
    /// whose film is not the one the shared player has open.
    [[nodiscard]] std::optional<core::Duration> length(const ProjectPage& page) const;

private:
    /// Selects `row` alone, keeping the column of the current cell, and shows it.
    void selectRow(int row);

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

    /// Asks for a position from the bar: **the first at once, then at most one every
    /// `kSeekGateMs`, and the last one asked is always reached.** A seek waits for the picture,
    /// and a drag is dozens of positions a second — handing every one to the player would queue
    /// the frames of a path nobody is looking at, and leave the picture behind the handle.
    void requestSeek(core::Timestamp position);

    /// Hands the player the position last asked, if one is waiting, and closes the gate again.
    void flushSeek();

    /// The table stops following playback: somebody moved it by hand — issue #619. The button of
    /// the bar says so.
    void suspendFollowing();

    /// The table follows playback again, and **centers on the row playing at the next `follow`**
    /// even if it is the row it already pointed at. Called by every gesture of the player: a play,
    /// a jump, a mark, a step.
    void resumeFollowing();

    /// Makes the step that waited for the gate, if one did, and closes the gate again.
    void flushStep();

    /// Takes the film to `position` and has the table follow it again — **the one road of every
    /// gesture that moves the film**: a jump, a neighbour, a selection, a drag of the bar, an edge
    /// that was moved. Written once because the gesture that forgot to take the following up again
    /// was the defect issue #619 repaired, and the next variant would have written it again.
    ///
    /// With `playUntil`, the film plays from there and stops on that frame.
    void goTo(ProjectPage& page,
              core::Timestamp position,
              std::optional<core::Timestamp> playUntil = std::nullopt);

    /// Puts the volume at `volume` — the player, the bar and the memory of it.
    void applyVolume(int volume);

    core::FileSystem* m_files;
    Prompts* m_prompts;
    View* m_view;
    SubtitleTable* m_table;
    QSplitter* m_split;
    PlayerFactory m_buildPlayer;
    FrameRateReader m_readDeclaredRate;

    /// The picture and the bar under it, which are the child of the splitter that is not the
    /// band: the room above the table is one, and holds both.
    QWidget* m_videoBox = nullptr;
    VideoSurface* m_picture = nullptr;
    PlayBar* m_bar = nullptr;
    QWidget* m_banner = nullptr;
    QAbstractButton* m_invite = nullptr;
    QTimer* m_ticker = nullptr;

    /// Closes for a moment after a seek asked from the bar. See `requestSeek`.
    QTimer* m_seekGate = nullptr;
    std::optional<core::Timestamp> m_pendingSeek{};

    /// Whether the table follows playback, and the row it was last centered on — a row is centered
    /// when it **changes**, and not at every tick. Minus one forgets it, which is what makes a
    /// resume center the row even if it is the same.
    bool m_following = true;
    int m_centeredRow = -1;

    /// Closes for a moment after a step. See `step`.
    QTimer* m_stepGate = nullptr;
    int m_pendingStep = 0;

    core::VideoSettings m_settings{};

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
