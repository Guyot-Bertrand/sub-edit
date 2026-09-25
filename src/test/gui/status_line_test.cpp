// The four standing facts of the status bar, without the window — issue #485.
//
// `StatusLine` puts on screen what `core/wording.hpp` says; these cases give it
// a bare status bar and a project, and hold each line to the words the command
// line reads too. What the window does around it — when it refreshes, which
// text it aims at — is proved by the window's own cases, unchanged.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/status_line.hpp>
#include <subedit/gui/target.hpp>

#include <QLabel>
#include <QList>
#include <QStatusBar>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <string>

#include "grid_fixtures.hpp"

namespace {

using subedit::core::Document;
using subedit::core::FrameRate;
using subedit::core::Project;
using subedit::core::StandardFrameRate;
using subedit::gui::StatusLine;

[[nodiscard]] QString worded(const std::string& words) {
    return QString::fromStdString(words);
}

/// A project read from a MicroDVD file at `rate`.
[[nodiscard]] Project countedInFrames(FrameRate rate) {
    Project project;
    project.setSourceFile(subedit::core::SourceFile{.extras = subedit::core::MicroDvdFile{rate}});
    project.setFrameRate(rate);
    return project;
}

} // namespace

TEST_CASE("the four lines sit on the bar, the target first and hidden", "[gui][GUI-TRANS-05]") {
    QStatusBar bar;
    const StatusLine line{bar};

    const QList<QLabel*> labels = bar.findChildren<QLabel*>(Qt::FindDirectChildrenOnly);
    CHECK(labels.contains(line.target()));
    CHECK(labels.contains(line.encoding()));
    CHECK(labels.contains(line.grid()));
    CHECK(labels.contains(line.video()));
    CHECK(line.target()->isHidden());
}

TEST_CASE("the target is named only while there are two texts", "[gui][GUI-TRANS-05]") {
    QStatusBar bar;
    StatusLine line{bar};

    line.showTarget(Document::Translation);
    CHECK_FALSE(line.target()->isHidden());
    CHECK(line.target()->text() ==
          QStringLiteral("Text: %1").arg(subedit::gui::documentName(Document::Translation)));

    line.showTarget(std::nullopt);
    CHECK(line.target()->isHidden());
    CHECK(line.target()->text().isEmpty());
}

TEST_CASE("the encoding line says what the document carries", "[gui][GUI-ENC-01]") {
    QStatusBar bar;
    StatusLine line{bar};
    Project project;
    const subedit::core::Encoding marked =
        subedit::core::Encoding::utf8(subedit::core::ByteOrderMark::Present);
    project.setSourceFile(subedit::core::SourceFile{.encoding = marked});

    line.refreshEncoding(project);

    CHECK(line.encoding()->text() == worded(subedit::core::encodingStatusOf(marked)));
}

TEST_CASE("the grid line says the grid the positions sit on", "[gui][GUI-GRID-01]") {
    QStatusBar bar;
    StatusLine line{bar};
    const FrameRate fps24{StandardFrameRate::Fps24};

    line.refreshGrid(subedit::test::gridProject("grille-24.srt", fps24));
    CHECK(line.grid()->text() ==
          worded(subedit::core::gridStatusOf(subedit::core::GridVerdict::Clean, fps24)));

    // Nothing to deduce from: the verdict is silent and names no rate.
    line.refreshGrid(Project{});
    CHECK(line.grid()->text() ==
          worded(subedit::core::gridStatusOf(subedit::core::GridVerdict::Silent, std::nullopt)));
}

TEST_CASE("a document counted in frames gets its rate, not a grid", "[gui][GUI-GRID-01]") {
    QStatusBar bar;
    StatusLine line{bar};
    const FrameRate fps25{StandardFrameRate::Fps25};

    CHECK_FALSE(subedit::gui::rateReadInFrames(Project{}).has_value());
    CHECK(subedit::gui::rateReadInFrames(countedInFrames(fps25)) == fps25);

    line.refreshGrid(countedInFrames(fps25));
    CHECK(line.grid()->text() == worded(subedit::core::framesStatusOf(fps25)));
}

TEST_CASE("the video line names the film and the rate it declares", "[gui][GUI-VIDEO-01]") {
    QStatusBar bar;
    StatusLine line{bar};
    Project project;

    line.refreshVideo(project);
    CHECK(line.video()->text() == worded(subedit::core::videoStatusOf(std::nullopt)));

    const FrameRate fps25{StandardFrameRate::Fps25};
    project.chooseVideo("/films/film.mkv");
    project.setDeclaredFrameRate(fps25);
    line.refreshVideo(project);
    CHECK(line.video()->text() ==
          worded(subedit::core::videoStatusOf(std::filesystem::path{"/films/film.mkv"}, fps25)));
}
