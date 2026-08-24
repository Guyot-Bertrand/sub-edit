// What watching a film costs the window — the two measures phase 6 asked for.
//
// **Not the decoding.** What would be timed there is libmpv, which does not
// belong to this project and whose figure would say nothing anyone here could
// act on. What is timed is the two things `subedit` does around it: the
// question the follower asks ten times a second, and the road a line of
// dialogue takes to the picture.
//
// Opening and seeking are the exception, and the spec named them: they are what
// happens at every change of row, so they are the one path the user feels.

#include <subedit/core/model/project.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include "full_length_project.hpp"

namespace {

using subedit::core::PlayerError;
using subedit::core::showingAt;
using subedit::core::Timestamp;
using subedit::gui::assEventOf;
using subedit::gui::MpvPlayer;
using subedit::test::fullLengthProject;

/// Somewhere in the middle of the film, which is where a walk over a document
/// out of order costs what it costs whatever the answer is.
constexpr int kMidFilmMilliseconds = 5400000;

[[nodiscard]] std::filesystem::path fixture(const char* name) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "videos" / name;
}

} // namespace

TEST_CASE("finding the subtitle showing at a position", "[benchmark]") {
    const subedit::core::Project project = fullLengthProject();
    const Timestamp when = Timestamp::fromMilliseconds(kMidFilmMilliseconds);

    BENCHMARK("la réplique en cours, sur 4000 sous-titres") {
        return showingAt(project, when);
    };
}

TEST_CASE("composing the replica handed to the overlay", "[benchmark]") {
    const std::string line = "Il n'y a pas de quoi en faire une histoire,\net tu le sais.";

    BENCHMARK("composer une réplique de deux lignes") {
        return assEventOf(line);
    };
}

// Two seconds of film, which is all a fixture needs to be: what is timed is the
// call and the wait, not the length of what is behind it.
TEST_CASE("opening a video and seeking into it", "[benchmark]") {
    BENCHMARK("ouvrir une vidéo") {
        std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
        MpvPlayer player = std::move(built.value());
        return player.open(fixture("cadence-25.mp4")).has_value();
    };

    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    MpvPlayer player = std::move(built.value());
    REQUIRE(player.open(fixture("cadence-25.mp4")).has_value());

    BENCHMARK("chercher une position") {
        player.seek(Timestamp::fromMilliseconds(1000));
        return player.position();
    };
}
