// What the window makes of the two things the container and the player know —
// issue #177, decisions D4 and D6.
//
// Both are read through seams: what a film declares comes from a
// `FrameRateReader` rather than from `ffprobe`, and how long it lasts from the
// player double rather than from libmpv. Neither is a convenience — a machine
// with no `ffmpeg` is an ordinary machine, and these cases are how that state
// stays walked.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QAction>
#include <QDialog>
#include <QItemSelectionModel>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::StandardFrameRate;
using subedit::core::VideoPlayer;
using subedit::gui::FrameRateDialog;
using subedit::gui::FrameRateReader;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::PlayerFactory;
using subedit::gui::ShiftDialog;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

/// Two subtitles, the second ending at four seconds.
constexpr const char* kTwo = "1\n"
                             "00:00:01,000 --> 00:00:02,000\n"
                             "[soupir] Un.\n"
                             "\n"
                             "2\n"
                             "00:00:03,000 --> 00:00:04,000\n"
                             "Deux.\n"
                             "\n";

/// A player kept hold of, as `window_player_test.cpp` does.
struct Projectionist {
    FakeVideoPlayer* player = nullptr;
};

[[nodiscard]] PlayerFactory projecting(Projectionist& booth) {
    return [&booth](std::uintptr_t) -> std::unique_ptr<VideoPlayer> {
        auto made = std::make_unique<FakeVideoPlayer>();
        booth.player = made.get();
        return made;
    };
}

/// What a film declares, and how many times it was asked.
struct Container {
    std::optional<FrameRate> declares;
    int asked = 0;
};

[[nodiscard]] FrameRateReader declaring(Container& container) {
    return [&container](const std::filesystem::path&) {
        ++container.asked;
        return container.declares;
    };
}

[[nodiscard]] InMemoryFileSystem directoryHolding(std::initializer_list<const char*> names) {
    InMemoryFileSystem files;
    for (const char* name : names)
        files.addFile(std::filesystem::path{"/films"} / name, "");
    files.addFile("/films/film.fr.srt", kTwo);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "/films/film.fr.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// What a frame rate dialog showed, read while it was still alive.
///
/// **The dialog is a local of the window**, so `lastDialog` dangles the moment
/// the operation returns; what a case wants to know has to be taken while the
/// window is inside `run`. That is what `fill` is for, and every other dialog
/// test uses it the same way.
struct Seen {
    std::string declared;
    std::optional<FrameRate> output;
};

[[nodiscard]] Seen openFrameRateDialog(FakePrompts& prompts, MainWindow& window) {
    Seen seen;
    // Cancelled: these cases read what the dialog offers, and offer nothing back.
    prompts.nextRun = false;
    prompts.fill = [&seen](QDialog& dialog) {
        auto& rates = dynamic_cast<FrameRateDialog&>(dialog);
        seen.declared = rates.declaredLabel().toStdString();
        seen.output = rates.output();
    };

    window.frameRateAction()->trigger();
    return seen;
}

/// Fills the shift dialog with `typed` and accepts it.
void shiftBy(FakePrompts& prompts, const char* typed) {
    prompts.nextRun = true;
    prompts.fill = [typed](QDialog& dialog) {
        dynamic_cast<ShiftDialog&>(dialog).setTyped(QString::fromUtf8(typed));
    };
}

} // namespace

// GUI-FRAMERATE-02 — the rate is read once per film and reaches the dialog.
TEST_CASE("the rate the film declares is proposed in the dialog", "[gui][GUI-FRAMERATE-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    container.declares = FrameRate{StandardFrameRate::Fps23976};
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();
    REQUIRE(container.asked == 1);

    const Seen seen = openFrameRateDialog(prompts, window);

    CHECK(seen.declared == "24000/1001");
    CHECK(seen.output == FrameRate{StandardFrameRate::Fps23976});
}

// The reading costs a process. A film that has not changed is not asked again,
// however many times the dialog is opened.
TEST_CASE("the film is asked once, not at every dialog", "[gui][GUI-FRAMERATE-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    container.declares = FrameRate{StandardFrameRate::Fps25};
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();

    (void)openFrameRateDialog(prompts, window);
    (void)openFrameRateDialog(prompts, window);

    CHECK(container.asked == 1);
}

// Without `ffprobe` — or with one that has nothing to say about this file — the
// row is absent and the dialog is exactly the one from before.
TEST_CASE("a film that declares nothing proposes nothing", "[gui][GUI-FRAMERATE-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();

    CHECK(openFrameRateDialog(prompts, window).declared.empty());
}

// No film at all: nothing is asked of anybody, and the dialog opens as it
// always did.
TEST_CASE("a document with no film asks no container", "[gui][GUI-FRAMERATE-02]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    container.declares = FrameRate{StandardFrameRate::Fps23976};
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();

    const Seen seen = openFrameRateDialog(prompts, window);

    CHECK(container.asked == 0);
    CHECK(seen.declared.empty());
}

// And a window given no reader at all — which is what a `subedit-gui` built
// without one would be — behaves the same way.
TEST_CASE("a window with no reader proposes nothing", "[gui][GUI-FRAMERATE-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files), prompts, projecting(booth)};
    window.show();

    CHECK(openFrameRateDialog(prompts, window).declared.empty());
}

// GUI-BOUNDS-01 — the notice, which prevents nothing.
TEST_CASE("an operation that passes the end of the film says so", "[gui][GUI-BOUNDS-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();
    REQUIRE(booth.player != nullptr);
    booth.player->length = Duration::fromMilliseconds(5000);

    shiftBy(prompts, "00:00:02,000");
    window.shiftAction()->trigger();

    REQUIRE_FALSE(prompts.outcomes.empty());
    CHECK(prompts.outcomes.back() ==
          "shifting leaves 1 subtitle past the end of the video, by 1.000 s at most");
    // Nothing was prevented: the shift happened, and the history holds it.
    CHECK(window.undoAction()->isEnabled());
}

TEST_CASE("an operation that stays inside the film says nothing", "[gui][GUI-BOUNDS-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();
    REQUIRE(booth.player != nullptr);
    booth.player->length = Duration::fromMilliseconds(60000);

    shiftBy(prompts, "00:00:02,000");
    window.shiftAction()->trigger();

    CHECK(prompts.outcomes.empty());
}

// The length is what the player knows. With no film open there is nothing to be
// past the end of, and no operation behaves differently for it.
TEST_CASE("with no film open nothing is said of the end", "[gui][GUI-BOUNDS-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();

    shiftBy(prompts, "00:10:00,000");
    window.shiftAction()->trigger();

    CHECK(prompts.outcomes.empty());
}

// A subtitle already past the end because the film is the wrong one is nobody's
// doing — least of all that of an operation that moves no position.
TEST_CASE("an operation that moves nothing is not accused of the end", "[gui][GUI-BOUNDS-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    Container container;
    MainWindow window{files, fileIn(files), prompts, projecting(booth), declaring(container)};
    window.show();
    REQUIRE(booth.player != nullptr);
    // Shorter than the document, so both subtitles are already past the end.
    booth.player->length = Duration::fromMilliseconds(500);

    prompts.nextRun = true;
    prompts.fill = [](QDialog&) {};
    window.hearingImpairedAction()->trigger();

    REQUIRE_FALSE(prompts.outcomes.empty());
    // What it says is what it did, and nothing about the end of the film.
    CHECK(prompts.outcomes.back() == "1 subtitle cleaned, 0 removed");
}
