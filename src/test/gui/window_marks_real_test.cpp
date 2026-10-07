// A mark set from the position of the film, read back by the real player — issue #617.
//
// The criterion is that setting a mark and returning to it shows the same picture. The fake
// player cannot say so: it has no pictures. This one runs the window on libmpv and a numbered
// video, whose every frame shows its own number, so the oracle is what is drawn and not what the
// player says of itself.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/mpv_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QDir>
#include <QItemSelectionModel>
#include <QString>
#include <QTemporaryDir>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>

#include "fake_prompts.hpp"
#include "numbered_frames.hpp"

namespace {

using subedit::core::RealFileSystem;
using subedit::core::Timestamp;
using subedit::gui::MainWindow;
using subedit::gui::MpvPlayer;

} // namespace

TEST_CASE("a start set from the position takes the player back to the same frame",
          "[gui][numbered][GUI-MARK-01]") {
    const QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const std::filesystem::path folder = directory.path().toStdString();
    std::filesystem::copy_file(std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "videos" /
                                   "images-23-976.mp4",
                               folder / "film.mp4");
    std::ofstream{folder / "film.srt"} << "1\n00:00:00,100 --> 00:00:00,500\nOne.\n\n";

    RealFileSystem files;
    auto opened = subedit::core::openProject(files, folder / "film.srt");
    REQUIRE(opened.has_value());

    MpvPlayer* real = nullptr;
    subedit::gui::PlayerFactory realPlayers = subedit::gui::mpvPlayers();
    subedit::test::FakePrompts prompts;
    MainWindow window{files,
                      std::move(*opened),
                      prompts,
                      [&]() {
                          std::unique_ptr<subedit::core::VideoPlayer> made = realPlayers();
                          real = dynamic_cast<MpvPlayer*>(made.get());
                          return made;
                      },
                      subedit::gui::declaredFrameRates(files)};
    window.show();
    REQUIRE(real != nullptr);

    window.table()->selectionModel()->select(window.table()->model()->index(0, 0),
                                             QItemSelectionModel::ClearAndSelect |
                                                 QItemSelectionModel::Rows);

    // Frames of a video at 24000/1001 images a second, whose starts are fractions of a
    // millisecond: the position the player gives is the one that has to bring it back.
    for (const int frame : {7, 30, 61, 143}) {
        INFO("frame " << frame);
        real->seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame, 24000, 1001)));
        window.setStartFromVideoAction()->trigger();

        real->seek(Timestamp::fromMilliseconds(0));
        const auto cellText = window.table()
                                  ->model()
                                  ->data(window.table()->model()->index(0, 1), Qt::DisplayRole)
                                  .toString();
        const Timestamp marked =
            Timestamp::parse(cellText.toStdString()).value_or(Timestamp::origin());
        REQUIRE(marked != Timestamp::origin());
        real->seek(marked);

        const subedit::gui::Picture picture = real->picture().value_or(subedit::gui::Picture{});
        REQUIRE(picture.width > 0);
        CHECK(subedit::test::frameNumberOf(picture) == frame);
    }
}
