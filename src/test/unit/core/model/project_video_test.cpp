// What a document remembers of the film it is watched against, and the one
// rule that state carries: a choice is never overwritten by a guess (D5).

#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>

namespace {

using subedit::core::AssociatedVideo;
using subedit::core::Project;
using subedit::core::VideoOrigin;

} // namespace

TEST_CASE("a fresh project is associated with no video", "[video][model]") {
    const Project project;

    CHECK_FALSE(project.video().has_value());
}

TEST_CASE("choosing a video associates it, and says it was chosen", "[video][model]") {
    Project project;

    project.chooseVideo("/films/film.mkv");

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/film.mkv", .origin = VideoOrigin::Chosen});
}

TEST_CASE("a proposal is taken when nothing is associated yet", "[video][model]") {
    Project project;

    CHECK(project.proposeVideo("/films/film.mkv"));

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/film.mkv", .origin = VideoOrigin::Guessed});
}

TEST_CASE("a proposal replaces an earlier proposal", "[video][model]") {
    Project project;
    project.proposeVideo("/films/premier.mkv");

    CHECK(project.proposeVideo("/films/second.mkv"));

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/second.mkv", .origin = VideoOrigin::Guessed});
}

// D5, and the whole reason the origin is remembered. Opening another subtitle
// file, or saving under a name the convention reads differently, runs the
// convention again — and a user who has said which film they meant must not
// have to say it twice.
TEST_CASE("a proposal never overwrites a choice", "[video][model]") {
    Project project;
    project.chooseVideo("/films/le-bon.mkv");

    CHECK_FALSE(project.proposeVideo("/films/le-voisin.mkv"));

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/le-bon.mkv", .origin = VideoOrigin::Chosen});
}

TEST_CASE("a choice overwrites a proposal, and another choice", "[video][model]") {
    Project project;
    project.proposeVideo("/films/devine.mkv");

    project.chooseVideo("/films/voulu.mkv");
    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/voulu.mkv", .origin = VideoOrigin::Chosen});

    project.chooseVideo("/films/finalement.mkv");
    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/finalement.mkv", .origin = VideoOrigin::Chosen});
}

// What the container declares, kept beside the path — the third thing decision
// D6 rests on, and the reason it is remembered rather than recomputed.
TEST_CASE("a project remembers what the film declares", "[video][model]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;

    Project project;
    project.chooseVideo("/films/film.mkv");

    project.setDeclaredFrameRate(FrameRate{StandardFrameRate::Fps23976});

    CHECK(project.video() == AssociatedVideo{.path = "/films/film.mkv",
                                             .origin = VideoOrigin::Chosen,
                                             .declared = FrameRate{StandardFrameRate::Fps23976}});
}

// Nothing is an ordinary answer: no `ffprobe`, or a file that declares no video
// stream. It is said, and it replaces whatever was known before.
TEST_CASE("a film that declares nothing says so", "[video][model]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;

    Project project;
    project.chooseVideo("/films/film.mkv");
    project.setDeclaredFrameRate(FrameRate{StandardFrameRate::Fps25});

    project.setDeclaredFrameRate(std::nullopt);

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/film.mkv", .origin = VideoOrigin::Chosen});
}

// A film that has just been chosen has not been asked yet, and a rate read from
// the one before it would be a rate attributed to the wrong film.
TEST_CASE("choosing another film forgets what the last one declared", "[video][model]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;

    Project project;
    project.chooseVideo("/films/film.mkv");
    project.setDeclaredFrameRate(FrameRate{StandardFrameRate::Fps25});

    project.chooseVideo("/films/autre.mkv");

    CHECK(project.video() ==
          AssociatedVideo{.path = "/films/autre.mkv", .origin = VideoOrigin::Chosen});
}

// Nothing to record it on. Said out loud because the alternative — inventing an
// association to hang a rate on — is the kind of thing that happens by accident.
TEST_CASE("a document with no film records no declared rate", "[video][model]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;

    Project project;

    project.setDeclaredFrameRate(FrameRate{StandardFrameRate::Fps25});

    CHECK_FALSE(project.video().has_value());
}
