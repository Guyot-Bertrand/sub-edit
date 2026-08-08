#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::ConvertFrameRateCommand;
using subedit::core::Document;
using subedit::core::FrameRate;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

constexpr SubtitleIndex kFirst = SubtitleIndex::fromValue(0);
constexpr SubtitleIndex kSecond = SubtitleIndex::fromValue(1);

const FrameRate kPal{StandardFrameRate::Fps25};
const FrameRate kNtscFilm{StandardFrameRate::Fps23976};

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

/// A project timed at 25 frames per second, holding the three positions the
/// measurement of ADR 0013 was taken on.
[[nodiscard]] Project palProject() {
    Project project;
    project.setSubtitles({at(1010, 1020), at(3600017, 3600020)});
    project.setFrameRate(kPal);
    return project;
}

[[nodiscard]] std::vector<std::int64_t> boundsOf(const Project& project) {
    std::vector<std::int64_t> bounds;
    bounds.reserve(project.count() * 2);
    for (const Subtitle& subtitle : project.subtitles()) {
        bounds.push_back(subtitle.start.milliseconds());
        bounds.push_back(subtitle.end.milliseconds());
    }
    return bounds;
}

} // namespace

TEST_CASE("a conversion multiplies the positions by the exact ratio of the rates",
          "[edit][framerate]") {
    // The three measurements of ADR 0013: a file timed at 25 and read at
    // 23.976 runs slower, so every position moves later.
    Project project = palProject();
    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{1053, 1064, 3753768, 3753771});
}

TEST_CASE("a conversion does not answer what the frames would", "[edit][framerate]") {
    // The whole point of ADR 0013, asserted against the path it rejects:
    // going through the frame grid quantises each position and lands up to
    // half a frame away — 1043 instead of 1053, 3753750 instead of 3753768.
    Project project = palProject();
    const std::vector<std::int64_t> throughFrames = {
        Timestamp::fromFrame(Timestamp::fromMilliseconds(1010).toFrame(kPal), kNtscFilm)
            .milliseconds(),
        Timestamp::fromFrame(Timestamp::fromMilliseconds(3600017).toFrame(kPal), kNtscFilm)
            .milliseconds()};
    REQUIRE(throughFrames == std::vector<std::int64_t>{1043, 3753750});

    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};
    command.apply(project);

    CHECK(project.subtitleAt(kFirst).start.milliseconds() != throughFrames[0]);
    CHECK(project.subtitleAt(kSecond).start.milliseconds() != throughFrames[1]);
}

TEST_CASE("a conversion sets the frame rate of the project", "[edit][framerate]") {
    Project project = palProject();
    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    command.apply(project);

    CHECK(project.frameRate() == kNtscFilm);
}

TEST_CASE("undoing a conversion restores the positions and the frame rate", "[edit][framerate]") {
    // Both, which is what the issue asks: a conversion that gave the positions
    // back but left the project claiming the new rate would leave it lying
    // about what its numbers mean.
    Project project = palProject();
    const std::vector<std::int64_t> before = boundsOf(project);
    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    command.apply(project);
    command.revert(project);

    CHECK(boundsOf(project) == before);
    CHECK(project.frameRate() == kPal);
}

TEST_CASE("undoing restores the rate the project had, not the one declared as input",
          "[edit][framerate]") {
    // The two can differ: the user may declare that the file was really timed
    // at another rate than the project assumed. Undoing has to put back the
    // state that was, not the one that was asserted.
    Project project = palProject();
    project.setFrameRate(FrameRate{StandardFrameRate::Fps30});
    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    command.apply(project);
    command.revert(project);

    CHECK(project.frameRate() == FrameRate{StandardFrameRate::Fps30});
}

TEST_CASE("converting to the rate already in use changes nothing", "[edit][framerate]") {
    Project project = palProject();
    const std::vector<std::int64_t> before = boundsOf(project);
    ConvertFrameRateCommand command{project, Selection::all(project), kPal, kPal};

    command.apply(project);

    CHECK(boundsOf(project) == before);
}

TEST_CASE("a conversion the other way undoes the first one on round positions",
          "[edit][framerate]") {
    // Not a general property — the operation rounds — but it holds where
    // nothing is lost, and it says the direction is the one intended.
    Project project;
    project.setSubtitles({at(0, 24000)});
    project.setFrameRate(kPal);

    ConvertFrameRateCommand there{project, Selection::all(project), kPal, kNtscFilm};
    there.apply(project);
    CHECK(project.subtitleAt(kFirst).end.milliseconds() == 25025);

    ConvertFrameRateCommand back{project, Selection::all(project), kNtscFilm, kPal};
    back.apply(project);
    CHECK(project.subtitleAt(kFirst).end.milliseconds() == 24000);
}

TEST_CASE("a conversion leaves the subtitles outside the selection alone", "[edit][framerate]") {
    Project project = palProject();
    const std::array<SubtitleIndex, 1> selected = {kFirst};
    ConvertFrameRateCommand command{project, Selection::of(selected), kPal, kNtscFilm};

    command.apply(project);

    CHECK(project.subtitleAt(kSecond).start.milliseconds() == 3600017);
    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 1053);
}

TEST_CASE("a conversion reports the positions it moved", "[edit][framerate]") {
    const Project project = palProject();
    const ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    const std::vector<subedit::core::Change> changes = command.describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Positions);
    CHECK(changes[0].indices == std::vector<SubtitleIndex>{kFirst, kSecond});
}

TEST_CASE("a conversion says what it is", "[edit][framerate]") {
    const Project project = palProject();
    const ConvertFrameRateCommand command{project, Selection::all(project), kPal, kNtscFilm};

    CHECK(command.kind() == CommandKind::ConvertFrameRate);
}

TEST_CASE("a conversion marks both documents", "[edit][framerate]") {
    Session session{palProject()};

    session.apply(std::make_unique<ConvertFrameRateCommand>(
        session.project(), Selection::all(session.project()), kPal, kNtscFilm));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing a conversion restores the exact state", "[edit][framerate]") {
    Session session{palProject()};
    const std::vector<std::int64_t> before = boundsOf(session.project());

    session.apply(std::make_unique<ConvertFrameRateCommand>(
        session.project(), Selection::all(session.project()), kPal, kNtscFilm));
    session.undo();
    CHECK(boundsOf(session.project()) == before);
    CHECK(session.project().frameRate() == kPal);

    session.redo();
    CHECK(session.project().subtitleAt(kFirst).start.milliseconds() == 1053);
    CHECK(session.project().frameRate() == kNtscFilm);
}
