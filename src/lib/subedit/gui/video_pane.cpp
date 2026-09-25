#include <subedit/core/edit/session.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/video_pane.hpp>

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPushButton>
#include <QSplitter>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
      m_picture(new QWidget{owner}),
      m_banner(new QWidget{owner}),
      m_invite(new QPushButton{QStringLiteral("Select Video…"), m_banner}),
      m_ticker(new QTimer{owner}) {
    // **A window of the system, and that is the whole point of these two
    // attributes.** libmpv draws into a window the platform numbers; a plain Qt
    // widget shares its parent's, and there would be nothing of its own to hand
    // over. `WA_DontCreateNativeAncestors` keeps the demand from spreading
    // upwards and turning the table into a native window as well.
    m_picture->setAttribute(Qt::WA_NativeWindow);
    m_picture->setAttribute(Qt::WA_DontCreateNativeAncestors);
    m_picture->setMinimumHeight(kMinimumVideoHeight);
    m_picture->hide();

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
    split.addWidget(m_picture);
    split.addWidget(m_banner);

    m_ticker->setInterval(kFollowIntervalMs);
    QObject::connect(m_ticker, &QTimer::timeout, m_ticker, [this] {
        if (m_playingPage != nullptr)
            follow(*m_playingPage);
    });
}

VideoPane::~VideoPane() = default;

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
        // Asked here and not in the constructor, so that the surface is native
        // before its number is read — and so that a window nobody shows a film
        // to never builds a player at all.
        m_player = m_buildPlayer(static_cast<std::uintptr_t>(m_picture->winId()));
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

    // **Shown before the film is opened, and not after.** libmpv adopts the
    // window it is handed at that moment; one that is not on screen is adopted
    // and never mapped. Taken away again below if the film will not open, which
    // costs nothing anybody sees: nothing has been painted into it yet.
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
        // Why there is none — a session whose windows libmpv cannot adopt, a
        // libmpv that would not start — is one sentence in the manual rather
        // than a taxonomy in a dialog.
        m_prompts->reportFailure(wanted.string() + ": no video player is available");
    }

    // A film that has been left behind must not go on playing under a document
    // that no longer shows it — least of all one nobody can see any more.
    if (!page.watching && m_player) {
        m_player->pause();
        m_player->showSubtitle({});
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

    m_picture->setVisible(picture);
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
    // **Here, while the surface libmpv draws into still exists** — issue
    // #470. Quitting through the event loop takes the native window away
    // before the members of the window are destroyed; libmpv, still holding
    // it, then asked X to destroy a window that was already gone, and the
    // default X error handler ended the process from libmpv's own thread —
    // `BadWindow`, then Qt objects destroyed from the wrong thread, then an
    // exit code of 1.
    m_ticker->stop();
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
    const std::optional<core::SubtitleIndex> showing =
        where.has_value() ? core::showingAt(project, *where) : std::nullopt;

    // Read from the project at every tick, which is what makes D2 true rather
    // than merely stated: a text edited a moment ago is on the picture within a
    // tenth of a second, and nothing was written to a disk to put it there.
    //
    // **The text of the document aimed at**, the rule of the whole window: the
    // column of the current cell says which, and there is no setting.
    const std::string line = showing.has_value()
                                 ? project.subtitleAt(*showing).text(m_view->targetDocument())
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

std::optional<core::Duration> VideoPane::length(const ProjectPage& page) const {
    // The player is shared: what it knows is the length of the film of the
    // page it plays for, and of no other.
    return page.watching && &page == m_playingPage ? m_player->duration() : std::nullopt;
}

} // namespace subedit::gui
