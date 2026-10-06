#pragma once

// Reading the number a frame of a numbered video carries — issue #610.
//
// `src/test/data/videos/images-*.mp4` are made by `video-fixtures.sh`: every frame
// shows its own number, eight bars of 16 pixels, each one light for a bit of 1 and dark
// for a bit of 0, the least significant at the left. A test that has seeked reads the
// picture the player shows and compares the number to the frame it asked for — an
// oracle that does not rest on what the player says of itself.

#include <subedit/gui/mpv_player.hpp>

#include <cstdint>

namespace subedit::test {

/// The number the picture carries, 0 to 255.
[[nodiscard]] inline int frameNumberOf(const subedit::gui::Picture& picture) {
    constexpr int kBars = 8;
    constexpr int kLight = 128;
    const int bar = picture.width / kBars;
    const int row = picture.height / 2;

    int number = 0;
    for (int bit = 0; bit < kBars; ++bit) {
        if (picture.greenAt((bit * bar) + (bar / 2), row) > kLight)
            number |= 1 << bit;
    }
    return number;
}

/// The millisecond frame `frame` of a video of `numerator` / `denominator` images a
/// second starts at, **as the player says it**: the start in seconds, rounded to the
/// nearest millisecond.
[[nodiscard]] inline std::int64_t
startOf(int frame, std::int64_t numerator, std::int64_t denominator) {
    constexpr std::int64_t kMillisecondsPerSecond = 1000;
    const std::int64_t scaled =
        static_cast<std::int64_t>(frame) * denominator * kMillisecondsPerSecond;
    return (scaled + (numerator / 2)) / numerator;
}

} // namespace subedit::test
