// The chain a player test needs, proved before any player exists.
//
// It tests no feature — there is none yet, and the seam is #173's work. It
// tests that what a phase-6 test will rely on is in place: libmpv links, a
// player initialises **where there is no screen**, a video opens, and what it
// then knows about itself is readable. Each of those fails at run time and
// never at compilation, so nothing but running them can tell us. It is the
// same proof `window_harness_test.cpp` carries for the window, and it answers
// the same question one library later.
//
// **`vo=null` is what makes this work, and it was measured rather than taken
// on promise.** With `vo=auto` and no display, the same sequence loads nothing:
// mpv answers `end-file`, and `duration` comes back « property unavailable ».
// The option is load-bearing, not decorative — which is what the issue asked
// to settle before a line of player was written.
//
// **What a test can expect from a player with no output.** The duration and
// the position after an exact seek, both below. Not a decoded image:
// `screenshot-raw` answers « error running command » under `vo=null`, there
// being no output to take a frame from — so drawing the current line over the
// image, which phase 6 owes, will be proved in the window and not here.
// Playback does advance in real time, and the picture's geometry is readable;
// neither is asserted here, a test that sleeps to watch a clock being a flake
// waiting to happen.
//
// It lives in `subedit_core_test` because that is where the seam's own tests
// will live: the player belongs to the core, which knows no Qt, and proving
// the chain anywhere else would prove it for a binary nobody is going to use.

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <mpv/client.h>

#include <array>
#include <memory>
#include <string>
#include <utility>

namespace {

using Catch::Matchers::WithinAbs;

/// What makes a player headless, and the whole of it.
///
/// One place, named once, because #173 will need the same list to drive the
/// real seam under test — and a list copied into a second file diverges at the
/// first option added.
constexpr std::array<std::pair<const char*, const char*>, 5> kHeadlessOptions{{
    // The developer's ~/.config/mpv must not decide whether the gate passes.
    {"config", "no"},
    // Nothing of mpv's own on the test's output.
    {"terminal", "no"},
    // The point of the whole file. See the note above on what it is worth.
    {"vo", "null"},
    // A continuous integration runner has no sound device either.
    {"ao", "null"},
    // Opening is not playing.
    {"pause", "yes"},
}};

/// Where a player stops waiting for what it was told to expect.
///
/// Generous on purpose: it is not a performance budget but a way out of an
/// event loop that would otherwise hang the gate. Opening these fixtures takes
/// milliseconds.
constexpr double kEventTimeoutSeconds = 5.0;
constexpr int kMaxEventsAwaited = 100;

/// A millisecond — the unit the project has measured positions in since
/// ADR 0006. Comparing finer would compare what nothing in subedit tells apart.
constexpr double kTolerance = 0.001;

/// libmpv hands out a handle to be given back — a resource owner in the sense
/// of docs/principes-de-conception.md, and one line is enough for a test.
/// The seam will own it in its own class, which is #173's decision to make.
struct TerminateAndDestroy {
    void operator()(mpv_handle* player) const noexcept { mpv_terminate_destroy(player); }
};

using Player = std::unique_ptr<mpv_handle, TerminateAndDestroy>;

[[nodiscard]] Player headlessPlayer() {
    Player player{mpv_create()};
    REQUIRE(player != nullptr);

    for (const auto& [name, value] : kHeadlessOptions)
        REQUIRE(mpv_set_option_string(player.get(), name, value) >= 0);

    REQUIRE(mpv_initialize(player.get()) >= 0);
    return player;
}

/// Waits for one event, and gives up rather than waiting forever.
///
/// `MPV_EVENT_NONE` is what a timeout looks like, and `MPV_EVENT_END_FILE`
/// means mpv gave up on the file: both are answers, and neither is the one
/// asked for.
[[nodiscard]] bool awaits(mpv_handle* player, mpv_event_id wanted) {
    for (int seen = 0; seen < kMaxEventsAwaited; ++seen) {
        const mpv_event* event = mpv_wait_event(player, kEventTimeoutSeconds);
        if (event->event_id == wanted)
            return true;
        if (event->event_id == MPV_EVENT_NONE || event->event_id == MPV_EVENT_END_FILE)
            return false;
    }
    return false;
}

[[nodiscard]] bool opens(mpv_handle* player, const std::string& file) {
    std::array<const char*, 3> load{"loadfile", file.c_str(), nullptr};
    return mpv_command(player, load.data()) >= 0 && awaits(player, MPV_EVENT_FILE_LOADED);
}

/// Reads a numeric property, and fails the case rather than answering for a
/// player that does not know it. Before a file is loaded most properties are
/// « property unavailable », and a test that read one would be asking the
/// wrong question rather than finding a defect.
[[nodiscard]] double number(mpv_handle* player, const char* name) {
    double value = 0.0;
    REQUIRE(mpv_get_property(player, name, MPV_FORMAT_DOUBLE, &value) >= 0);
    return value;
}

/// Reads a string property, and gives libmpv back the buffer it hands out.
[[nodiscard]] std::string text(mpv_handle* player, const char* name) {
    char* value = mpv_get_property_string(player, name);
    REQUIRE(value != nullptr);
    const std::string held{value};
    mpv_free(value);
    return held;
}

[[nodiscard]] std::string fixture(const std::string& name) {
    return std::string{SUBEDIT_TEST_DATA_DIR} + "/videos/" + name;
}

} // namespace

TEST_CASE("a player initialises where there is no screen", "[video][player]") {
    const Player player = headlessPlayer();

    // Initialised, and initialised with the option the rest of this file rests
    // on: what mpv answers is the video output it really took, not the one it
    // was handed.
    CHECK(text(player.get(), "vo") == "null");
}

TEST_CASE("a video opens and tells its duration", "[video][player]") {
    const Player player = headlessPlayer();

    // The two fixtures of #163, and both because the pair is the point: one
    // frame rate is whole and the other is not.
    const auto [name, seconds] =
        GENERATE(std::pair{"cadence-25.mp4", 2.000}, std::pair{"cadence-23-976.mp4", 2.002});

    REQUIRE(opens(player.get(), fixture(name)));

    CHECK_THAT(number(player.get(), "duration"), WithinAbs(seconds, kTolerance));
}

TEST_CASE("a video that is not there does not open", "[video][player]") {
    const Player player = headlessPlayer();

    // What a window will have to say out loud, and the reason `opens` reports
    // rather than asserts: mpv answers `end-file`, it does not fail the call.
    CHECK_FALSE(opens(player.get(), fixture("aucun-fichier.mp4")));
}

// The claim ADR 0020 chose libmpv on, exercised at its smallest: `absolute+exact`
// seeks to the position asked for, not to the keyframe before it. Phase 14 rests
// on this, and it costs one test case to know it holds with no screen.
TEST_CASE("an exact seek lands where it was asked", "[video][player]") {
    const Player player = headlessPlayer();
    REQUIRE(opens(player.get(), fixture("cadence-25.mp4")));

    std::array<const char*, 4> seek{"seek", "1.0", "absolute+exact", nullptr};
    REQUIRE(mpv_command(player.get(), seek.data()) >= 0);
    REQUIRE(awaits(player.get(), MPV_EVENT_PLAYBACK_RESTART));

    CHECK_THAT(number(player.get(), "time-pos"), WithinAbs(1.0, kTolerance));
}
