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
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/replica.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "full_length_project.hpp"
#include "numbered_frames.hpp"

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

// What the follower pays at every tick a subtitle is on screen — issue #408: the text through the
// pivot, tags understood and written for the overlay, before it is compared with what is drawn.
TEST_CASE("translating a tagged text for the overlay", "[benchmark]") {
    const std::string line =
        "<i>Il n'y a pas de quoi</i> en faire une <b>histoire</b>,\net tu le sais.";

    BENCHMARK("traduire une réplique balisée pour l'image") {
        return subedit::core::replicaOf(line, subedit::core::SubtitleFormat::SubRip);
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

// What painting a picture costs the window — ADR 0041, issue #613. The decision was taken
// on a figure measured once, on a 1080p film: 2.2 to 3.4 ms an image whatever the size of
// the buffer. This is that figure, taken again at every version, at the three sizes a
// picture is likely to have.
//
// **The film is the 128×64 fixture, scaled up to the buffer**: it is versioned and every
// machine has it, and a buffer of 1920×1080 costs the same to fill from it as from a
// film of that size — what dominates is the scaling and the conversion into the buffer,
// not the decoding, which a paused film does not repeat. A film at the size of a real one
// is a measure of the machine's libmpv, not of what `subedit` does around it.
TEST_CASE("rendering a picture into the buffer of the surface", "[benchmark]") {
    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    MpvPlayer player = std::move(built.value());
    REQUIRE(player.open(fixture("images-25.mp4")).has_value());
    player.seek(Timestamp::fromMilliseconds(1000));

    constexpr std::size_t kBytesAPixel = 4;
    constexpr std::size_t kSmall = 640;
    constexpr std::size_t kMedium = 1280;
    constexpr std::size_t kLarge = 1920;
    std::vector<unsigned char> small(kSmall * 360 * kBytesAPixel);
    std::vector<unsigned char> medium(kMedium * 720 * kBytesAPixel);
    std::vector<unsigned char> large(kLarge * 1080 * kBytesAPixel);

    BENCHMARK("rendre une image de 640×360") {
        return player.render(small, 640, 360, kSmall * kBytesAPixel);
    };
    BENCHMARK("rendre une image de 1280×720") {
        return player.render(medium, 1280, 720, kMedium * kBytesAPixel);
    };
    BENCHMARK("rendre une image de 1920×1080") {
        return player.render(large, 1920, 1080, kLarge * kBytesAPixel);
    };
}

// ## Seeking and stepping on a film-sized video — issue #611

// **The cost of a seek grows with the distance to the keyframe, and "back one frame" is
// what the phase makes a gesture of** — held under a finger, it either keeps up or it does
// not. Nothing measured it: the fixtures are 13 kB and their costs are a few milliseconds.
//
// **The video is made by `make bench`, in the build tree** (`video-fixtures.sh --film`): 720p,
// 250 frames, one keyframe at the start, each frame carrying its number. Not versioned — a
// video of that size has no place in the repository — and given here by
// `SUBEDIT_BENCH_FILM`. Without it, because `ffmpeg` is missing, **the benchmark says so and
// abstains** rather than measuring something else.
//
// **Every gesture is checked before it is timed**, against the number the picture carries: a
// measure of a gesture that lands on the wrong frame is worth nothing.
//
// What this does not say: `mpeg4`, 720p, one keyframe — a real film (H.264, HEVC, 1080p) may
// cost something else, and the measure on one's own film is the user's.
namespace {

constexpr std::int64_t kFilmFrameRate = 25;

[[nodiscard]] Timestamp startOfFrame(int frame) {
    return Timestamp::fromMilliseconds(subedit::test::startOf(frame, kFilmFrameRate, 1));
}

/// The number the picture the player shows carries.
[[nodiscard]] int shownFrame(const MpvPlayer& player) {
    return subedit::test::frameNumberOf(player.picture().value_or(subedit::gui::Picture{}));
}

} // namespace

TEST_CASE("seeking and stepping on a film-sized video", "[benchmark][film]") {
    const char* named = std::getenv("SUBEDIT_BENCH_FILM");
    if (named == nullptr || !std::filesystem::exists(named))
        SKIP("no film-sized video: SUBEDIT_BENCH_FILM is unset or points at nothing, "
             "which is what a machine without ffmpeg gives — `make bench` makes it");

    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    MpvPlayer player = std::move(built.value());
    REQUIRE(player.open(std::filesystem::path{named}).has_value());

    // The distance to the keyframe, in frames: the keyframe itself, then up to the far end.
    for (const int frame : {0, 100, 249}) {
        player.seek(startOfFrame(frame));
        REQUIRE(shownFrame(player) == frame);

        BENCHMARK("chercher l'image " + std::to_string(frame) + " (vidéo 720p)") {
            player.seek(startOfFrame(frame));
            return player.position();
        };
    }

    // A step is timed from the frame next to its target, put there before the clock starts.
    // One step a run: at these costs Catch2 asks for a single iteration, and a sample that
    // took more would only repeat the step from where the first one left it.
    //
    // Forward arrives at the frames the seeks were timed at — the near and the far one — and
    // back arrives one frame short of them: from the last frame there is no frame to step
    // back from, so the far end is measured at 248.
    for (const int frame : {100, 249}) {
        player.seek(startOfFrame(frame - 1));
        player.stepFrames(1);
        REQUIRE(shownFrame(player) == frame);

        BENCHMARK_ADVANCED("pas avant vers l'image " + std::to_string(frame) + " (vidéo 720p)")
        (Catch::Benchmark::Chronometer meter) {
            player.seek(startOfFrame(frame - 1));
            meter.measure([&] {
                player.stepFrames(1);
                return player.position();
            });
        };
    }

    for (const int frame : {99, 248}) {
        player.seek(startOfFrame(frame + 1));
        player.stepFrames(-1);
        REQUIRE(shownFrame(player) == frame);

        BENCHMARK_ADVANCED("pas arrière vers l'image " + std::to_string(frame) + " (vidéo 720p)")
        (Catch::Benchmark::Chronometer meter) {
            player.seek(startOfFrame(frame + 1));
            meter.measure([&] {
                player.stepFrames(-1);
                return player.position();
            });
        };
    }
}

// ## A real film — the relecture of phase 14, ADR 0041
//
// **The decision of ADR 0041 was taken on 1080p H.264, and what it did not measure was the rest**:
// 4K, a heavy codec, ten bits. This is the measure that was missing, taken on **the user's own
// film**, given by `SUBEDIT_BENCH_REAL_FILM` and never versioned: the repository has no film of
// that size, and none is made, because a synthetic one is as easy to decode as a test card and
// says nothing of the grain of a real one.
//
// What is timed is what the user does with the film — a seek, then one step and the painting of the
// picture it lands on at 1080p — **at a fixed distance from the keyframe**, as the benchmark of
// the film-sized video above does it: the cost of a step grows with that distance, and a loop of
// steps would measure a distance that grows with the loop. **The figure to compare it to is the
// length of a frame** — 40 ms at 25 images a second: a step that costs more is felt under a
// finger. Without the variable the benchmark says so and abstains.
//
// **It is played by hand, once in a while, and never by `make bench`**: at the default hundred
// samples a film of 4K takes an hour a series. `--benchmark-samples 10` gives the order of
// magnitude in a few minutes, and `SUBEDIT_BENCH_HWDEC=no` is the series without the card.
TEST_CASE("seeking and stepping on a real film", "[benchmark][realfilm]") {
    const char* named = std::getenv("SUBEDIT_BENCH_REAL_FILM");
    if (named == nullptr || !std::filesystem::exists(named))
        SKIP("no real film: SUBEDIT_BENCH_REAL_FILM is unset or points at nothing");

    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    MpvPlayer player = std::move(built.value());
    // `SUBEDIT_BENCH_HWDEC=no` is the series without the card — issue #647 — so that the two are
    // taken on the same film by the same binary.
    const char* hardware = std::getenv("SUBEDIT_BENCH_HWDEC");
    player.setHardwareDecoding(hardware == nullptr || std::string_view{hardware} != "no");
    REQUIRE(player.open(std::filesystem::path{named}).has_value());

    constexpr std::size_t kBytesAPixel = 4;
    constexpr std::size_t kWidth = 1920;
    constexpr int kHeight = 1080;
    std::vector<unsigned char> buffer(kWidth * kHeight * kBytesAPixel);
    constexpr std::int64_t kFrameRate = 25;
    const auto startOf = [](int frame) {
        return Timestamp::fromMilliseconds(subedit::test::startOf(frame, kFrameRate, 1));
    };

    // The distance to the keyframe, in frames: a short one, and one near the far end of ten
    // seconds.
    for (const int frame : {25, 100, 225}) {
        BENCHMARK("chercher l'image " + std::to_string(frame) + " et la rendre (vrai film)") {
            player.seek(startOf(frame));
            return player.render(buffer, static_cast<int>(kWidth), kHeight, kWidth * kBytesAPixel);
        };

        BENCHMARK_ADVANCED("pas avant vers l'image " + std::to_string(frame) +
                           " et son rendu (vrai film)")
        (Catch::Benchmark::Chronometer meter) {
            player.seek(startOf(frame - 1));
            meter.measure([&] {
                player.stepFrames(1);
                return player.render(
                    buffer, static_cast<int>(kWidth), kHeight, kWidth * kBytesAPixel);
            });
        };
    }
}
