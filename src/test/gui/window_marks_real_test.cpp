// The window on the real player and a numbered video — issues #617 and #618.
//
// The criteria here are about pictures: setting a mark and returning to it shows the same one, a
// step of N frames shows the one N frames away. The fake player cannot say so, it has no pictures.
// This one runs the window on libmpv and a video whose every frame shows its own number, so the
// oracle is what is drawn and not what the player says of itself.

#include <subedit/core/config/settings.hpp>
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
#include <QTest>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include "fake_prompts.hpp"
#include "numbered_frames.hpp"
#include "waiting.hpp"

namespace {

using subedit::core::RealFileSystem;
using subedit::core::Settings;
using subedit::core::Timestamp;
using subedit::gui::MainWindow;
using subedit::gui::MpvPlayer;

/// A window on the real player, with a numbered video beside a one-line document.
struct RealFilm {
    QTemporaryDir directory;
    RealFileSystem files;
    subedit::test::FakePrompts prompts;
    MpvPlayer* real = nullptr;
    std::unique_ptr<MainWindow> window;

    explicit RealFilm(const char* video, int stepFrames = 1) {
        REQUIRE(directory.isValid());
        const std::filesystem::path folder = directory.path().toStdString();
        std::filesystem::copy_file(std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "videos" / video,
                                   folder / "film.mp4");
        std::ofstream{folder / "film.srt"} << "1\n00:00:00,100 --> 00:00:00,500\nOne.\n\n";

        auto opened = subedit::core::openProject(files, folder / "film.srt");
        REQUIRE(opened.has_value());

        const subedit::gui::PlayerFactory players = subedit::gui::mpvPlayers();
        window = std::make_unique<MainWindow>(
            files,
            std::move(*opened),
            prompts,
            [this, players]() {
                std::unique_ptr<subedit::core::VideoPlayer> made = players();
                real = dynamic_cast<MpvPlayer*>(made.get());
                return made;
            },
            subedit::gui::declaredFrameRates(files));
        window->applySettings(Settings{.video = {.stepFrames = stepFrames}});
        window->show();
        REQUIRE(real != nullptr);
    }

    /// The number of the frame on screen.
    [[nodiscard]] int shown() const {
        const subedit::gui::Picture picture = real->picture().value_or(subedit::gui::Picture{});
        REQUIRE(picture.width > 0);
        return subedit::test::frameNumberOf(picture);
    }

    void select(int row) const {
        window->table()->selectionModel()->select(window->table()->model()->index(row, 0),
                                                  QItemSelectionModel::ClearAndSelect |
                                                      QItemSelectionModel::Rows);
    }
};

/// Whether the film comes to show frame `expected`.
///
/// **Waited for and not read at once**: a step asked while the gate of the one before is still
/// shut takes the one place there is, and is made when the gate opens — which on a loaded machine,
/// under a sanitizer, is later than a fixed pause guesses. The picture it reaches is the same.
[[nodiscard]] bool reaches(const RealFilm& film, int expected) {
    return subedit::test::waitUntil([&film, expected] { return film.shown() == expected; });
}
} // namespace

TEST_CASE("a start set from the position takes the player back to the same frame",
          "[gui][numbered][GUI-MARK-01]") {
    const RealFilm film{"images-23-976.mp4"};
    film.select(0);

    // Frames of a video at 24000/1001 images a second, whose starts are fractions of a
    // millisecond: the position the player gives is the one that has to bring it back.
    for (const int frame : {7, 30, 61, 143}) {
        INFO("frame " << frame);
        film.real->seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame, 24000, 1001)));
        film.window->setStartFromVideoAction()->trigger();

        film.real->seek(Timestamp::fromMilliseconds(0));
        const auto cellText =
            film.window->table()
                ->model()
                ->data(film.window->table()->model()->index(0, 1), Qt::DisplayRole)
                .toString();
        const Timestamp marked =
            Timestamp::parse(cellText.toStdString()).value_or(Timestamp::origin());
        REQUIRE(marked != Timestamp::origin());
        film.real->seek(marked);

        CHECK(film.shown() == frame);
    }
}

TEST_CASE("a step shows the frame N away, at 25 as at 23.976, and none is skipped or repeated",
          "[gui][numbered][GUI-STEP-01][GUI-STEP-02][GUI-STEP-04]") {
    struct Rate {
        const char* video;
        std::int64_t numerator;
        std::int64_t denominator;
    };

    for (const Rate& rate :
         {Rate{.video = "images-25.mp4", .numerator = 25, .denominator = 1},
          Rate{.video = "images-23-976.mp4", .numerator = 24000, .denominator = 1001}}) {
        for (const int step : {1, 7}) {
            INFO(rate.video << " by " << step);
            const RealFilm film{rate.video, step};
            film.real->seek(Timestamp::fromMilliseconds(
                subedit::test::startOf(30, rate.numerator, rate.denominator)));
            REQUIRE(film.shown() == 30);

            // Forward, one gesture at a time: every picture is the previous plus the step.
            for (int gesture = 1; gesture <= 6; ++gesture) {
                film.window->stepForwardAction()->trigger();
                CHECK(reaches(film, 30 + (gesture * step)));
            }
            // And back to where it started, by the same road.
            for (int gesture = 5; gesture >= 0; --gesture) {
                film.window->stepBackwardAction()->trigger();
                CHECK(reaches(film, 30 + (gesture * step)));
            }
        }
    }
}

TEST_CASE("a step stops at the ends of the film rather than going past them",
          "[gui][numbered][GUI-STEP-01][GUI-STEP-02]") {
    const RealFilm film{"images-25.mp4", 1000};
    film.real->seek(Timestamp::origin());
    const int first = film.shown();

    film.window->stepBackwardAction()->trigger();
    CHECK(reaches(film, first));

    // The step before this one closed the gate: this one waits for it to open, and the case waits
    // for the picture it reaches. The step is the size of the whole film, so it lands on the last
    // frame.
    film.window->stepForwardAction()->trigger();
    CHECK(subedit::test::waitUntil([&film, first] { return film.shown() > first; }));
    const int last = film.shown();

    // A step from the last frame stays on it. There is **nothing to wait for** — the picture does
    // not change, so no event says the step was made — and the case looks after the gate has had
    // more than its time to open. A step that went past the end would show a black frame, and this
    // can fail; it cannot fail for being too early.
    QTest::qWait(100);
    film.window->stepForwardAction()->trigger();
    QTest::qWait(100);
    CHECK(film.shown() == last);
}
