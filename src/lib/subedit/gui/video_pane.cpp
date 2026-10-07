#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/frame_step.hpp>
#include <subedit/core/video/replica.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/core/wording/video.hpp>
#include <subedit/gui/frame_source.hpp>
#include <subedit/gui/play_bar.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/video_pane.hpp>
#include <subedit/gui/video_surface.hpp>

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// How often the window asks the player where it is, in milliseconds.
///
/// Ten times a second, which is under what an eye notices on a replica and far
/// under what the reading costs — the answer is a property of an object in
/// this same process. Gaupol polls its own overlay every ten milliseconds; a
/// hundred is the same promise at a tenth of the price.
constexpr int kFollowIntervalMs = 100;

/// How long the gate of a seek asked from the bar stays closed, in milliseconds.
///
/// Twenty a second: under what the eye follows as a succession, and about what a seek costs at
/// the far end of a film — measured, issue #611, a tenth of a second at 720p. A shorter gate
/// would hand the player positions it cannot keep up with.
constexpr int kSeekGateMs = 50;

/// How long the gate of a step stays closed, in milliseconds.
///
/// A little under the period of a held key, which repeats some thirty times a second: a step that
/// is quick goes at the pace of the key, and one that is slow keeps no backlog, since what comes in
/// while the gate is shut takes one place.
constexpr int kStepGateMs = 20;

/// How much a gesture of volume moves it, in per cent — Gaupol's five.
constexpr int kVolumeStep = 5;

/// How close to the position a subtitle must start, or have ended, to count as ahead of it or
/// behind it: Gaupol's millisecond, so that the neighbour of a position that is exactly a start
/// is not that very subtitle.
constexpr std::int64_t kNeighbourMargin = 1;

/// How tall the picture may not go under, in pixels.
///
/// A splitter with nothing to stop it lets a child be dragged to nothing, and
/// a video view of zero pixels is indistinguishable from the one that is not
/// there — which is the state the window uses to mean « no film ».
constexpr int kMinimumVideoHeight = 180;

/// Which row of a selection playback follows: the first, in table order.
///
/// -1 when nothing is selected. `selectedRows` hands them back in the order
/// they were selected, which is not the order they are read in.
[[nodiscard]] int firstSelectedRow(const QItemSelectionModel& selection) {
    const QModelIndexList rows = selection.selectedRows();
    if (rows.isEmpty())
        return -1;

    return std::ranges::min(rows, {}, [](const QModelIndex& index) { return index.row(); }).row();
}

/// The first and the last row of a selection, in table order.
struct SelectedSpan {
    std::size_t first = 0;
    std::size_t last = 0;
};

/// Nothing for an empty selection.
[[nodiscard]] std::optional<SelectedSpan> selectedSpan(const QItemSelectionModel& selection) {
    const QModelIndexList rows = selection.selectedRows();
    if (rows.isEmpty())
        return std::nullopt;

    const auto [lowest, highest] =
        std::ranges::minmax(rows, {}, [](const QModelIndex& index) { return index.row(); });
    return SelectedSpan{.first = static_cast<std::size_t>(lowest.row()),
                        .last = static_cast<std::size_t>(highest.row())};
}

/// How long a subtitle inserted at the position lasts, at most: Gaupol's three seconds.
constexpr std::int64_t kInsertedLengthMilliseconds = 3000;

/// A second, in milliseconds.
constexpr std::int64_t kMillisecondsPerSecond = 1000;

} // namespace

VideoPane::VideoPane(core::FileSystem& files,
                     Prompts& prompts,
                     View& view,
                     SubtitleTable& table,
                     QSplitter& split,
                     PlayerFactory buildPlayer,
                     FrameRateReader readDeclaredRate,
                     QWidget* owner)
    : m_files(&files),
      m_prompts(&prompts),
      m_view(&view),
      m_table(&table),
      m_split(&split),
      m_buildPlayer(std::move(buildPlayer)),
      m_readDeclaredRate(std::move(readDeclaredRate)),
      m_videoBox(new QWidget{owner}),
      m_picture(new VideoSurface{m_videoBox}),
      m_bar(new PlayBar{m_videoBox}),
      m_banner(new QWidget{owner}),
      m_invite(new QPushButton{QStringLiteral("Select Video…"), m_banner}),
      m_ticker(new QTimer{owner}),
      m_seekGate(new QTimer{owner}),
      m_stepGate(new QTimer{owner}) {
    m_picture->setMinimumHeight(kMinimumVideoHeight);

    // The picture above, the bar under it, nothing between them and nothing around them.
    auto* stack = new QVBoxLayout{m_videoBox};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->setSpacing(0);
    stack->addWidget(m_picture, 1);
    stack->addWidget(m_bar);
    // Hidden three times, the box and what is in it: a test asks `isHidden` of the picture, and a
    // child is only « not hidden » of itself, whatever its parent shows.
    m_videoBox->hide();
    m_picture->hide();
    m_bar->hide();

    // **An absence a user cannot act on is worse than an empty pane.** Hiding
    // the picture when there is no film left nothing at all where one would go,
    // and the only way in was a menu one had to know about. A single button
    // says both things at once: there is no film, and here is how to choose
    // one.
    //
    // A band rather than a pane: it costs the table a line of height, where the
    // picture costs it a third of the window.
    auto* band = new QHBoxLayout{m_banner};
    band->addStretch();
    band->addWidget(m_invite);
    band->addStretch();

    // The picture on top; the window puts the table under it, and the line
    // between them is draggable.
    split.addWidget(m_videoBox);
    split.addWidget(m_banner);

    // The volume the bar shows before anything has set it: the player's own, which is the full
    // volume. Without this the handle waits at zero for the first gesture, and says silence over a
    // film that is not silent.
    m_bar->showVolume(m_settings.volume);

    m_seekGate->setSingleShot(true);
    m_seekGate->setInterval(kSeekGateMs);
    QObject::connect(m_seekGate, &QTimer::timeout, m_seekGate, [this] { flushSeek(); });

    m_stepGate->setSingleShot(true);
    m_stepGate->setInterval(kStepGateMs);
    QObject::connect(m_stepGate, &QTimer::timeout, m_stepGate, [this] { flushStep(); });

    QObject::connect(m_bar, &PlayBar::seekRequested, m_bar, [this](core::Timestamp position) {
        requestSeek(position);
    });
    // The handle is let go: whatever was asked last is reached now, and not at the next tick of
    // a gate nobody is waiting on.
    QObject::connect(m_bar, &PlayBar::seekFinished, m_bar, [this] {
        m_seekGate->stop();
        flushSeek();
    });
    QObject::connect(m_bar, &PlayBar::playToggled, m_bar, [this] {
        if (m_playingPage != nullptr)
            toggle(*m_playingPage);
    });
    QObject::connect(
        m_bar, &PlayBar::volumeRequested, m_bar, [this](int volume) { applyVolume(volume); });

    m_ticker->setInterval(kFollowIntervalMs);
    QObject::connect(m_ticker, &QTimer::timeout, m_ticker, [this] {
        if (m_playingPage != nullptr)
            follow(*m_playingPage);
    });
}

VideoPane::~VideoPane() {
    // The surface outlives this, as a child of the window, and must not be left holding
    // a player that goes with it.
    m_picture->attach(nullptr);
}

QWidget* VideoPane::picture() const {
    return m_picture;
}

bool VideoPane::choose(ProjectPage& page) {
    const std::optional<std::filesystem::path>& source = page.session->project().sourceFile().path;
    const std::filesystem::path directory =
        source.has_value() ? source->parent_path() : std::filesystem::path{};

    const std::optional<std::filesystem::path> chosen = m_prompts->videoToOpen(directory);
    if (!chosen.has_value())
        return false;

    page.session->chooseVideo(*chosen);
    return true;
}

void VideoPane::proposeBeside(ProjectPage& page) {
    const std::optional<std::filesystem::path>& source = page.session->project().sourceFile().path;
    if (!source.has_value())
        return;

    if (const std::optional<std::filesystem::path> found = core::findVideoBeside(*m_files, *source);
        found.has_value()) {
        // The answer is dropped on purpose: whether the proposal was taken
        // is D5's business, and a caller acting on it would be a second
        // place where that rule lives.
        (void)page.session->proposeVideo(*found);
    }
}

core::VideoPlayer* VideoPane::player() {
    if (!m_playerAsked && m_buildPlayer) {
        m_playerAsked = true;
        // Asked here and not in the constructor, so that a window nobody shows a
        // film to never builds a player at all.
        m_player = m_buildPlayer();

        // **A player that can be drawn is drawn**; the double of the tests cannot
        // and the surface stays black, which is all a test of the window needs.
        m_picture->attach(dynamic_cast<FrameSource*>(m_player.get()));

        // What the volume was left at, from the settings or the last gesture.
        if (m_player != nullptr)
            m_player->setVolume(m_settings.volume);
    }

    return m_player.get();
}

void VideoPane::windowShown(ProjectPage& page) {
    if (m_windowShown)
        return;

    m_windowShown = true;
    watch(page);
}

void VideoPane::watch(ProjectPage& page) {
    // Nothing is handed to a player before the window has been on screen once.
    // `windowShown` comes back here the moment it has, and until then this
    // leaves `page.associated` alone so that it finds the film still waiting.
    if (!m_windowShown)
        return;

    const std::optional<core::AssociatedVideo>& associated = page.session->project().video();
    const std::filesystem::path wanted =
        associated.has_value() ? associated->path : std::filesystem::path{};

    // **Two things have to agree, not one.** `wanted == page.associated`
    // alone answers « has this project's own association changed since it was
    // last synced », which is right for one project and wrong for several: a
    // switch of tab back to a page whose association never changed still
    // means the shared player is showing whatever the page just left behind
    // was watching, not this one.
    if (wanted == page.associated && m_playingPage == &page)
        return;

    // The place a tab was left at belongs to the film it was left on: another
    // film starts from its beginning.
    const bool sameFilm = wanted == page.associated;
    if (!sameFilm)
        page.resumeAt.reset();

    m_playingPage = &page;
    page.associated = wanted;
    page.watching = false;
    page.shown.clear();
    page.placedAt = -1;

    // Asked once per film, here, where the association has just changed for
    // certain: `ffprobe` is a process, and running it at every opening of the
    // frame rate dialog would pay for it again for an answer that cannot have
    // moved. Nothing, without a reader or without a film — which is what a
    // machine with no `ffmpeg` gets, and it is an ordinary state.
    page.session->setDeclaredFrameRate(
        !wanted.empty() && m_readDeclaredRate ? m_readDeclaredRate(wanted) : std::nullopt);

    // Shown before the film is opened, so that the surface has its size when the
    // first picture is announced. Taken away again below if the film will not open,
    // which costs nothing anybody sees: nothing has been painted into it yet.
    showPicture(!wanted.empty());

    core::VideoPlayer* watching = wanted.empty() ? nullptr : player();
    if (watching != nullptr) {
        if (const std::expected<void, core::PlayerError> opened = watching->open(wanted); opened) {
            page.watching = true;
            // Back where the tab was left, paused as every opening is.
            if (page.resumeAt.has_value())
                watching->seek(*page.resumeAt);
        } else
            m_prompts->reportFailure(wanted.string() + ": " + opened.error().reason);
    } else if (!wanted.empty() && m_buildPlayer) {
        // A film was named and there is no player to show it with. Said here
        // and not when the program started, because that is where it matters
        // and where it is not a remark about something nobody asked for yet.
        // Why there is none — a libmpv that would not start — is one sentence in
        // the manual rather than a taxonomy in a dialog.
        m_prompts->reportFailure(wanted.string() + ": no video player is available");
    }

    // A film that has been left behind must not go on playing under a document
    // that no longer shows it — least of all one nobody can see any more.
    if (!page.watching && m_player) {
        m_player->pause();
        m_player->showSubtitle({});
    }

    // The bar and the timecode go with the film too: a position over a picture that is not there.
    if (!page.watching) {
        m_bar->showPosition(std::nullopt, std::nullopt);
        m_bar->showPlaying(false);
        m_picture->setTimecode({});
    }

    // The mark goes with the film: a row left green under a document that no
    // longer shows anything would name a moment nobody is at.
    if (!page.watching && page.model)
        page.model->setShowing(std::nullopt);

    // Exactly one of the two, always: a band that stayed under a playing film
    // would offer to choose the one already chosen.
    showPicture(page.watching);
    m_view->playable(page.watching);

    if (page.watching) {
        m_ticker->start();
        follow(page);
    } else {
        m_ticker->stop();
    }
}

void VideoPane::showPicture(bool picture) {
    QList<int> sizes = m_split->sizes();
    const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
    // The room above the table is one, whichever child holds it; the picture
    // is never given less than its minimum, the table pays for it.
    const int above =
        std::min(std::max(sizes.at(0) + sizes.at(1), picture ? kMinimumVideoHeight : 0), total);

    m_videoBox->setVisible(picture);
    m_picture->setVisible(picture);
    m_bar->setVisible(picture);
    m_banner->setVisible(!picture);

    sizes[0] = picture ? above : 0;
    sizes[1] = picture ? 0 : above;
    sizes[2] = total - above;
    m_split->setSizes(sizes);
}

void VideoPane::leave(ProjectPage& page) {
    // Where the film of the tab being left stands, taken before the shared
    // player is handed to another film — issue #471.
    if (&page == m_playingPage && page.watching)
        page.resumeAt = m_player->position();
}

void VideoPane::forget(const ProjectPage& page) {
    // Cleared before the page it might name is freed: `watch` only ever
    // compares this pointer, never dereferences it, but comparing one that no
    // longer points at anything is not a comparison this class makes anywhere
    // else, and it does not start here.
    if (m_playingPage == &page)
        m_playingPage = nullptr;
}

void VideoPane::release(std::span<const std::unique_ptr<ProjectPage>> pages) {
    // **Here, and the surface lets go of the player first.** The player calls the
    // surface back from a thread of its own, and the surface reads the player when
    // it paints: neither may outlive the other. (Issue #470 was the same lesson
    // with a native window libmpv held after Qt had taken it away.)
    m_ticker->stop();
    m_picture->attach(nullptr);
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        page->watching = false;
        // Forgotten, so that a window shown again opens the film anew.
        page->associated.clear();
    }
    m_playingPage = nullptr;
    m_player.reset();
    m_playerAsked = false;
    m_view->playable(false);
}

void VideoPane::toggle(const ProjectPage& page) {
    if (!page.watching)
        return;

    if (m_player->isPlaying())
        m_player->pause();
    else
        m_player->play();
}

void VideoPane::placeAtSelection(ProjectPage& page) {
    if (!page.watching)
        return;

    const int row = firstSelectedRow(*m_table->selectionModel());
    if (row < 0 || row == page.placedAt)
        return;

    page.placedAt = row;
    const auto index = core::SubtitleIndex::fromValue(static_cast<std::size_t>(row));
    m_player->seek(page.session->project().subtitleAt(index).start);

    // At once rather than at the next tick: what the picture shows and what the
    // table points at have to agree by the time the click is over.
    follow(page);
}

void VideoPane::follow(ProjectPage& page) {
    if (!page.watching)
        return;

    const core::Project& project = page.session->project();

    // Written as one running answer rather than as a guard and a return, the
    // way `seconds` is in the player: « the player does not know where it is »
    // is an answer of the same rank as a position, and it leads to the same
    // place as a moment between two subtitles — nothing drawn, and the row
    // left where it was.
    const std::optional<core::Timestamp> where = m_player->position();

    // The bar and the timecode say the same position the replica is chosen by. **Read once**, so
    // that the three agree: a bar a tick ahead of its replica would be a defect nobody could
    // trace.
    m_bar->showPosition(where, m_player->duration());
    m_bar->showPlaying(m_player->isPlaying());
    m_picture->setTimecode(where.has_value()
                               ? QString::fromStdString(where->format(core::DecimalMark::Comma))
                               : QString{});

    const std::optional<core::SubtitleIndex> showing =
        where.has_value() ? core::showingAt(project, *where) : std::nullopt;

    // Read from the project at every tick, which is what makes D2 true rather
    // than merely stated: a text edited a moment ago is on the picture within a
    // tenth of a second, and nothing was written to a disk to put it there.
    //
    // **The text of the document aimed at**, the rule of the whole window: the
    // column of the current cell says which, and there is no setting.
    //
    // **Through the pivot** — issue #408: the text is held as its file wrote it, and what the
    // overlay is handed is that text with its tags understood, in the vocabulary it draws.
    const core::Document document = m_view->targetDocument();
    const std::string line = showing.has_value()
                                 ? core::replicaOf(project.subtitleAt(*showing).text(document),
                                                   project.sourceFile(document).format)
                                 : std::string{};
    if (line != page.shown) {
        m_player->showSubtitle(line);
        page.shown = line;
    }

    page.model->setShowing(showing);

    if (!showing.has_value())
        return;

    // **Whoever is typing wins.** Moving the current cell closes the editor
    // open on it, and a correction half made would go with it.
    if (m_table->isEditing())
        return;

    const QModelIndex current = m_table->currentIndex();
    const int row = static_cast<int>(showing->value());
    if (current.isValid() && current.row() == row)
        return;

    // `NoUpdate` is what keeps the selection out of this. The selection is what
    // an operation applies to, and a film playing in the background has no
    // business rewriting the user's target row by row — it also happens to be
    // what keeps this from firing the seek that watches the selection.
    const QModelIndex followed = page.model->index(row, current.isValid() ? current.column() : 0);
    m_table->selectionModel()->setCurrentIndex(followed, QItemSelectionModel::NoUpdate);
    m_table->scrollTo(followed);
}

std::vector<core::AudioTrack> VideoPane::audioTracks() const {
    // Only the film of the page on screen: the player is shared, and what it knows is the tracks
    // of whichever film it holds.
    return m_player != nullptr && m_playingPage != nullptr && m_playingPage->watching
               ? m_player->audioTracks()
               : std::vector<core::AudioTrack>{};
}

void VideoPane::selectAudioTrack(int id) {
    if (m_player != nullptr && m_playingPage != nullptr && m_playingPage->watching)
        m_player->selectAudioTrack(id);
}

core::VideoSettings VideoPane::settings() const {
    return m_settings;
}

void VideoPane::setSettings(const core::VideoSettings& settings) {
    m_settings = settings;
    applyVolume(settings.volume);
}

void VideoPane::applyVolume(int volume) {
    m_settings.volume = std::clamp(volume, 0, core::kLargestVolume);
    if (m_player != nullptr)
        m_player->setVolume(m_settings.volume);
    m_bar->showVolume(m_settings.volume);
}

void VideoPane::changeVolume(int delta) {
    applyVolume(m_settings.volume + (delta < 0 ? -kVolumeStep : kVolumeStep));
}

void VideoPane::requestSeek(core::Timestamp position) {
    if (m_playingPage == nullptr || !m_playingPage->watching)
        return;

    m_pendingSeek = position;
    // The gate open: this one goes at once, which is what makes a click on the groove feel
    // immediate. Closed: it waits, and replaces whatever waited before it.
    if (!m_seekGate->isActive())
        flushSeek();
}

void VideoPane::flushSeek() {
    if (!m_pendingSeek.has_value() || m_playingPage == nullptr || !m_playingPage->watching) {
        m_pendingSeek.reset();
        return;
    }

    const core::Timestamp position = *m_pendingSeek;
    m_pendingSeek.reset();
    m_player->seek(position);
    follow(*m_playingPage);
    m_seekGate->start();
}

void VideoPane::step(ProjectPage& /*page*/, int direction) {
    // Nothing to check of the page: `flushStep` asks the one the player plays for, and lets a step
    // go that has nothing to move.
    m_pendingStep = direction < 0 ? -1 : 1;
    // The gate open: this one goes at once. Closed: it takes the one place there is.
    if (!m_stepGate->isActive())
        flushStep();
}

void VideoPane::flushStep() {
    if (m_pendingStep == 0 || m_playingPage == nullptr || !m_playingPage->watching) {
        m_pendingStep = 0;
        return;
    }

    const int frames = m_pendingStep * m_settings.stepFrames;
    m_pendingStep = 0;
    m_player->stepFrames(frames);
    follow(*m_playingPage);
    m_stepGate->start();
}

void VideoPane::nudge(ProjectPage& page, core::Boundary boundary, int direction) {
    const std::optional<SelectedSpan> span = selectedSpan(*m_table->selectionModel());
    if (!span.has_value())
        return;

    const std::optional<core::CountedFrameRate> counted =
        core::countedFrameRateOf(page.session->project());
    if (!counted.has_value()) {
        m_prompts->reportFailure(core::noFrameToCountBy());
        return;
    }

    const core::SubtitleIndex row = core::SubtitleIndex::fromValue(span->first);
    const core::Timestamp moved =
        core::movedByFrames(page.session->project().subtitleAt(row).position(boundary),
                            counted->rate,
                            (direction < 0 ? -1 : 1) * m_settings.stepFrames);
    if (page.session->project().subtitleAt(row).position(boundary) == moved)
        return;

    page.model->applied(page.session->apply(
        std::make_unique<core::SetPositionCommand>(page.session->project(), row, boundary, moved)));
    page.placedAt = -1;

    // The edge is where the film is shown, so that what was set can be seen.
    if (page.watching) {
        m_player->seek(moved);
        follow(page);
    }
}

void VideoPane::seekBy(ProjectPage& page, int direction) {
    if (!page.watching)
        return;

    const std::optional<core::Timestamp> where = m_player->position();
    const std::optional<core::Duration> length = m_player->duration();
    if (!where.has_value() || !length.has_value())
        return;

    const std::int64_t jump =
        static_cast<std::int64_t>(m_settings.seekLengthSeconds) * kMillisecondsPerSecond;
    const std::int64_t target = std::clamp<std::int64_t>(
        where->milliseconds() + (direction < 0 ? -jump : jump), 0, length->milliseconds());
    m_player->seek(core::Timestamp::fromMilliseconds(target));
    follow(page);
}

void VideoPane::seekToNeighbour(ProjectPage& page, bool next) {
    if (!page.watching)
        return;

    const std::optional<core::Timestamp> where = m_player->position();
    if (!where.has_value())
        return;

    // The subtitles are walked and not searched: a project may be out of order (ADR 0008), and
    // « the first that starts after » means the smallest start, not the first one met.
    std::optional<core::Timestamp> found;
    for (const core::Subtitle& subtitle : page.session->project().subtitles()) {
        if (next && subtitle.start.milliseconds() > where->milliseconds() + kNeighbourMargin) {
            if (!found.has_value() || subtitle.start < *found)
                found = subtitle.start;
        } else if (!next &&
                   subtitle.end.milliseconds() < where->milliseconds() - kNeighbourMargin) {
            if (!found.has_value() || subtitle.start > *found)
                found = subtitle.start;
        }
    }

    if (!found.has_value())
        return;

    m_player->seek(*found);
    follow(page);
}

void VideoPane::seekToSelection(ProjectPage& page, bool end) {
    if (!page.watching)
        return;

    const std::optional<SelectedSpan> span = selectedSpan(*m_table->selectionModel());
    if (!span.has_value())
        return;

    const core::Project& project = page.session->project();
    const core::Timestamp edge =
        end ? project.subtitleAt(core::SubtitleIndex::fromValue(span->last)).end
            : project.subtitleAt(core::SubtitleIndex::fromValue(span->first)).start;
    m_player->seek(core::Timestamp::fromMilliseconds(
        std::max<std::int64_t>(edge.milliseconds() - m_settings.contextLengthMilliseconds, 0)));
    follow(page);
}

void VideoPane::playSelection(ProjectPage& page) {
    if (!page.watching)
        return;

    const std::optional<SelectedSpan> span = selectedSpan(*m_table->selectionModel());
    if (!span.has_value())
        return;

    const core::Project& project = page.session->project();
    const core::Timestamp start =
        project.subtitleAt(core::SubtitleIndex::fromValue(span->first)).start;
    m_player->seek(core::Timestamp::fromMilliseconds(
        std::max<std::int64_t>(start.milliseconds() - m_settings.contextLengthMilliseconds, 0)));
    // Up to the end of the last subtitle of the selection, and not up to the end of the
    // film: `playUntil` stops on the frame.
    m_player->playUntil(project.subtitleAt(core::SubtitleIndex::fromValue(span->last)).end);
    follow(page);
}

void VideoPane::markEdge(ProjectPage& page, core::Boundary boundary) {
    if (!page.watching)
        return;

    const std::optional<SelectedSpan> span = selectedSpan(*m_table->selectionModel());
    const std::optional<core::Timestamp> where = m_player->position();
    if (!span.has_value() || !where.has_value())
        return;

    const core::SubtitleIndex row = core::SubtitleIndex::fromValue(span->first);
    if (page.session->project().subtitleAt(row).position(boundary) == *where)
        return;

    page.model->applied(page.session->apply(std::make_unique<core::SetPositionCommand>(
        page.session->project(), row, boundary, *where)));
    page.placedAt = -1;
    follow(page);
}

void VideoPane::insertAtPosition(ProjectPage& page) {
    if (!page.watching)
        return;

    const std::optional<core::Timestamp> where = m_player->position();
    if (!where.has_value())
        return;

    // Counted and not searched, as the neighbours are: a project may be out of order.
    const std::span<const core::Subtitle> subtitles = page.session->project().subtitles();
    const auto before = static_cast<std::size_t>(std::ranges::count_if(
        subtitles, [&](const core::Subtitle& subtitle) { return subtitle.start <= *where; }));

    core::Timestamp end = *where + core::Duration::fromMilliseconds(kInsertedLengthMilliseconds);
    if (before < subtitles.size() && subtitles[before].start < end)
        end = subtitles[before].start;

    const core::SubtitleIndex at = core::SubtitleIndex::fromValue(before);
    page.model->applied(page.session->apply(std::make_unique<core::InsertCommand>(
        at, std::vector<core::Subtitle>{core::Subtitle{.start = *where, .end = end}})));
    page.placedAt = -1;
    selectRow(static_cast<int>(before));
    follow(page);
}

void VideoPane::selectFromPosition(ProjectPage& page, bool next) {
    if (!page.watching)
        return;

    const std::span<const core::Subtitle> subtitles = page.session->project().subtitles();
    const std::optional<core::Timestamp> where = m_player->position();
    if (subtitles.empty() || !where.has_value())
        return;

    // The first that starts after, or the last that started before — by the order of the file,
    // as Gaupol reads it — and the end of the file on that side when there is none.
    std::size_t found = next ? subtitles.size() - 1 : 0;
    for (std::size_t at = 0; at < subtitles.size(); ++at) {
        // Walked from the front for the next, from the back for the previous.
        const std::size_t row = next ? at : subtitles.size() - 1 - at;
        if (next ? subtitles[row].start > *where : subtitles[row].start < *where) {
            found = row;
            break;
        }
    }
    selectRow(static_cast<int>(found));
}

void VideoPane::selectRow(int row) {
    // The column of the current cell stays, as `MainWindow::selectRows` keeps it: it says which
    // text an operation aims at.
    const QModelIndex current = m_table->currentIndex();
    const int column = current.isValid() ? current.column() : 0;
    QAbstractItemModel* model = m_table->model();
    const QModelIndex from = model->index(row, column);
    const QModelIndex to = model->index(row, model->columnCount() - 1);
    m_table->selectionModel()->setCurrentIndex(from, QItemSelectionModel::NoUpdate);
    m_table->selectionModel()->select(
        QItemSelection{from, to}, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_table->scrollTo(from);
}

std::optional<core::Duration> VideoPane::length(const ProjectPage& page) const {
    // The player is shared: what it knows is the length of the film of the
    // page it plays for, and of no other.
    return page.watching && &page == m_playingPage ? m_player->duration() : std::nullopt;
}

} // namespace subedit::gui
