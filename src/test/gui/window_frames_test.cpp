// The positions shown as frame numbers — issue #620, decision D8.
//
// One setting of the window, `View ▸ Positions in Frames`: the columns `Start` and `End` show the
// number of the frame a position falls in, and take a number when they are edited. The model is
// told the rate and the project is not touched, so nothing goes through the history and the file
// writes what it wrote.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording/video.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QLineEdit>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QValidator>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::StandardFrameRate;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr const char* kSrt = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                             "2\n00:00:02,500 --> 00:00:03,500\nDeux.\n\n";

/// A MicroDVD file at the default rate, which is 24000/1001: its numbers are the file's own.
constexpr const char* kMicroDvd = "{25}{50}Un.\n{75}{100}Deux.\n";

/// What a position cell of the window holds, as it is shown.
[[nodiscard]] std::string cell(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// A window on one file, the film of which declares `declared`, if it declares anything.
struct Framed {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window;

    Framed(const char* name, const char* text, std::optional<FrameRate> declared)
        : files{filesWith(name, text, declared.has_value())},
          window{files, open(files, name), prompts, {}, [declared](const std::filesystem::path&) {
                     return declared;
                 }} {
        window.show();
    }

    static InMemoryFileSystem filesWith(const char* name, const char* text, bool withFilm) {
        InMemoryFileSystem made;
        made.addFile(std::filesystem::path{"/films"} / name, text);
        if (withFilm)
            made.addFile("/films/film.mkv", "");
        return made;
    }

    static subedit::core::OpenedFile open(const InMemoryFileSystem& in, const char* name) {
        auto opened = subedit::core::openProject(in, std::filesystem::path{"/films"} / name);
        REQUIRE(opened.has_value());
        return std::move(*opened);
    }
};

} // namespace

TEST_CASE("the setting is out with no rate to count by, and says why", "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, std::nullopt};

    CHECK_FALSE(framed.window.framePositionsAction()->isEnabled());
    CHECK(framed.window.framePositionsAction()->toolTip().toStdString() ==
          subedit::core::noFrameRateToShow());

    // Asked all the same, it changes nothing: the columns stay timestamps.
    framed.window.framePositionsAction()->setChecked(true);
    CHECK(cell(framed.window, 0, 1) == "00:00:01,000");
}

TEST_CASE("a film's rate lights the setting, and the columns show frame numbers",
          "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, FrameRate{StandardFrameRate::Fps25}};
    REQUIRE(framed.window.framePositionsAction()->isEnabled());
    CHECK(cell(framed.window, 0, 1) == "00:00:01,000");

    framed.window.framePositionsAction()->setChecked(true);

    // 1000 ms and 2000 ms at 25 images a second.
    CHECK(cell(framed.window, 0, 1) == "25");
    CHECK(cell(framed.window, 0, 2) == "50");
    CHECK(cell(framed.window, 1, 1) == "63");
    CHECK(framed.window.table()->model()->headerData(1, Qt::Horizontal).toString() ==
          QStringLiteral("Start (frames)"));
    CHECK(framed.window.table()->model()->headerData(2, Qt::Horizontal).toString() ==
          QStringLiteral("End (frames)"));
    CHECK(framed.window.framePositionsAction()->toolTip().toStdString() ==
          "Positions are frame numbers, counted at 25 fps, the rate the video declares");

    framed.window.framePositionsAction()->setChecked(false);
    CHECK(cell(framed.window, 0, 1) == "00:00:01,000");
    CHECK(framed.window.table()->model()->headerData(1, Qt::Horizontal).toString() ==
          QStringLiteral("Start"));
}

// Redrawn and nothing more: the history, the title's modified mark and the file are as they were.
TEST_CASE("switching the setting does not touch the document", "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, FrameRate{StandardFrameRate::Fps25}};
    const std::string title = framed.window.windowTitle().toStdString();

    framed.window.framePositionsAction()->setChecked(true);
    framed.window.framePositionsAction()->setChecked(false);

    CHECK_FALSE(framed.window.undoAction()->isEnabled());
    CHECK(framed.window.windowTitle().toStdString() == title);
    CHECK(cell(framed.window, 1, 2) == "00:00:03,500");
}

TEST_CASE("a MicroDVD file shows the numbers its file contains", "[gui][GUI-FRAMES-02]") {
    // Even with a film that declares another rate: the numbers are the file's own.
    const Framed framed{"film.sub", kMicroDvd, FrameRate{StandardFrameRate::Fps25}};
    REQUIRE(framed.window.framePositionsAction()->isEnabled());

    framed.window.framePositionsAction()->setChecked(true);

    CHECK(cell(framed.window, 0, 1) == "25");
    CHECK(cell(framed.window, 0, 2) == "50");
    CHECK(cell(framed.window, 1, 1) == "75");
    CHECK(cell(framed.window, 1, 2) == "100");
    CHECK(framed.window.framePositionsAction()->toolTip().toStdString() ==
          "Positions are frame numbers, counted at 24000/1001 fps, the rate of the file");
}

TEST_CASE("a MicroDVD file needs no film to show its frames", "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.sub", kMicroDvd, std::nullopt};

    CHECK(framed.window.framePositionsAction()->isEnabled());
}

// 37 frames of 23.976 images a second last 1543.04 milliseconds: rounded once, from the exact
// rational — a path through the whole millisecond of each frame would give 1542 or 1544.
TEST_CASE("typing a frame number sets the instant of that frame, rounded once",
          "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, FrameRate{StandardFrameRate::Fps23976}};
    framed.window.framePositionsAction()->setChecked(true);
    QAbstractItemModel* model = framed.window.table()->model();

    REQUIRE(model->setData(model->index(0, 1), QStringLiteral("37"), Qt::EditRole));

    framed.window.framePositionsAction()->setChecked(false);
    CHECK(cell(framed.window, 0, 1) == "00:00:01,543");
    // One entry of the history, as typing a timestamp is.
    CHECK(framed.window.undoAction()->isEnabled());
}

TEST_CASE("a frame number that is not one leaves the cell as it was", "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, FrameRate{StandardFrameRate::Fps25}};
    framed.window.framePositionsAction()->setChecked(true);
    QAbstractItemModel* model = framed.window.table()->model();

    for (const char* typed : {"00:00:01,000", "3.5", "abc", "", "12 13"})
        CHECK_FALSE(model->setData(model->index(0, 1), QString::fromUtf8(typed), Qt::EditRole));

    CHECK(cell(framed.window, 0, 1) == "25");
    CHECK_FALSE(framed.window.undoAction()->isEnabled());
}

TEST_CASE("the editor of a position cell takes numbers in frames and timestamps otherwise",
          "[gui][GUI-FRAMES-02]") {
    const Framed framed{"film.srt", kSrt, FrameRate{StandardFrameRate::Fps25}};
    subedit::gui::PositionDelegate delegate;
    const QStyleOptionViewItem option;
    const auto accepts = [&](const char* typed) {
        const QModelIndex at = framed.window.table()->model()->index(0, 1);
        const std::unique_ptr<QWidget> made{delegate.createEditor(nullptr, option, at)};
        const auto* line = qobject_cast<const QLineEdit*>(made.get());
        REQUIRE(line != nullptr);
        QString text = QString::fromUtf8(typed);
        int where = 0;
        const bool ok = line->validator()->validate(text, where) == QValidator::Acceptable;
        return ok;
    };

    CHECK(accepts("00:00:01,000"));
    CHECK_FALSE(accepts("37"));

    framed.window.framePositionsAction()->setChecked(true);
    CHECK(accepts("37"));
    CHECK(accepts("-12"));
    CHECK_FALSE(accepts("00:00:01,000"));
}
