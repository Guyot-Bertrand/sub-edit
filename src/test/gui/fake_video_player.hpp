#pragma once

// A player a window can be driven through, with no film and no screen.
//
// **This is what the seam of `core::VideoPlayer` was for**, and the spec of
// phase 6 said so before it existed: the real player is proved on the fixtures
// of #163, in `mpv_player_test.cpp`, and the window is proved against this
// one. The picture the window paints has its own cases, with a real player, in
// `video_surface_test.cpp` (ADR 0041); what belongs here is everything else the
// window decides, and deciding it does not need a film decoded every time.
//
// It answers what the interface promises and remembers what it was asked, so
// that a case can read « the window opened this film », « it placed playback
// here », « it drew that line » without a picture existing anywhere.

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::test {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
//   **The state a test double keeps *is* its interface.** A case reads
//   `player.opened` to say "the window opened this film", `player.seeks` to say
//   "it put playback here". Hiding them behind accessors would add seven
//   methods that protect nothing: there is no invariant to hold, and nobody but
//   the test builds this class.
//
//   `src/test/.clang-tidy` would have been the place, and it cannot be: a
//   configuration file bears on a directory, and for a diagnostic raised in a
//   **header** it is the configuration of the translation unit including it
//   that applies. See the "NOLINT" paragraph of `.clang-tidy` at the root —
//   issue #269.
//
//   The `IgnoreClassesWithAllMemberVariablesBeingPublic` option does not do
//   either: `m_open` stays private, and rightly — whether a film is open is a
//   detail the methods enforce. Making it public to satisfy the check would be
//   cheating.
class FakeVideoPlayer final : public core::VideoPlayer {

public:
    /// Why the next `open` fails, or nothing for a film that opens.
    std::optional<core::PlayerError> refusal{};

    /// Every film this was asked to open, in order.
    std::vector<std::filesystem::path> opened{};

    /// How long the open film lasts.
    core::Duration length = core::Duration::fromMilliseconds(600000);

    /// Where playback stands. A case moves it by hand: nothing here advances
    /// on its own, which is what makes these tests answer the same twice.
    core::Timestamp where = core::Timestamp::origin();

    /// Every position playback was placed at, in order.
    std::vector<core::Timestamp> seeks{};

    bool playing = false;

    /// How long a frame lasts for `stepFrames`. The real player reads it from
    /// the video; here a case that steps says it.
    core::Duration frame = core::Duration::fromMilliseconds(40);

    /// Every count of frames playback was stepped by, in order.
    std::vector<int> steps{};

    /// Every position `playUntil` was asked to stop at, in order.
    std::vector<core::Timestamp> stops{};

    int level = 100;

    /// What the open video offers. The track playing is the one marked
    /// `selected`; a case that wants none leaves it empty.
    std::vector<core::AudioTrack> tracks{};

    /// Every line the overlay was handed, in order. An empty one clears it.
    std::vector<std::string> shown{};

    /// Called when the player is destroyed, for a case to see when that
    /// happens — issue #470.
    std::function<void()> onDestroyed{};

    FakeVideoPlayer() = default;
    FakeVideoPlayer(const FakeVideoPlayer&) = delete;
    FakeVideoPlayer& operator=(const FakeVideoPlayer&) = delete;
    FakeVideoPlayer(FakeVideoPlayer&&) = delete;
    FakeVideoPlayer& operator=(FakeVideoPlayer&&) = delete;

    ~FakeVideoPlayer() override {
        if (onDestroyed)
            onDestroyed();
    }

    [[nodiscard]] std::expected<void, core::PlayerError>
    open(const std::filesystem::path& video) override {
        opened.push_back(video);
        if (refusal.has_value())
            return std::unexpected(*refusal);

        m_open = true;
        where = core::Timestamp::origin();
        playing = false;
        return {};
    }

    [[nodiscard]] std::optional<core::Duration> duration() const override {
        return m_open ? std::optional{length} : std::nullopt;
    }

    [[nodiscard]] std::optional<core::Timestamp> position() const override {
        return m_open ? std::optional{where} : std::nullopt;
    }

    void seek(core::Timestamp position) override {
        if (!m_open)
            return;

        seeks.push_back(position);
        where = position;
    }

    void stepFrames(int frames) override {
        if (!m_open || frames == 0)
            return;

        steps.push_back(frames);
        const std::int64_t at = where.milliseconds() + (frames * frame.milliseconds());
        where = core::Timestamp::fromMilliseconds(
            std::clamp<std::int64_t>(at, 0, length.milliseconds()));
        playing = false;
    }

    void play() override { playing = m_open; }

    void playUntil(core::Timestamp end) override {
        if (!m_open)
            return;

        stops.push_back(end);
        playing = true;
    }

    void pause() override { playing = false; }

    void showSubtitle(std::string_view line) override {
        if (m_open)
            shown.emplace_back(line);
    }

    [[nodiscard]] bool isPlaying() const override { return playing; }

    [[nodiscard]] int volume() const override { return level; }

    void setVolume(int volume) override { level = std::clamp(volume, 0, 100); }

    [[nodiscard]] std::vector<core::AudioTrack> audioTracks() const override {
        return m_open ? tracks : std::vector<core::AudioTrack>{};
    }

    void selectAudioTrack(int id) override {
        const bool known =
            std::ranges::any_of(tracks, [id](const core::AudioTrack& t) { return t.id == id; });
        if (!m_open || !known)
            return;

        for (core::AudioTrack& track : tracks)
            track.selected = track.id == id;
    }

    /// The line the picture carries now, or nothing drawn yet.
    [[nodiscard]] std::string onScreen() const {
        return shown.empty() ? std::string{} : shown.back();
    }

private:
    bool m_open = false;
};

// NOLINTEND(misc-non-private-member-variables-in-classes)

} // namespace subedit::test
