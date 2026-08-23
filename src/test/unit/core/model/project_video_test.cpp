// What a document remembers of the film it is watched against, and the one
// rule that state carries: a choice is never overwritten by a guess (D5).

#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/project.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

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
