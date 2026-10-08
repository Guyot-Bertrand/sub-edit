#pragma once

// The double of the player a window is given, and the file it opens — written once.
//
// Issue #646. Five test files each carried their own copy of the projector, and each copy was
// retouched when the player factory changed in phase 14. This is the superset of those copies:
// a case that needs less ignores the rest.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "fake_video_player.hpp"

namespace subedit::test {

/// The window owns its player, so a case cannot hand one in and keep it. It hands in a factory
/// instead and reads this afterwards — which is also how « the window never asked for a player »
/// becomes something to assert.
struct Projectionist {
    /// Whether libmpv would give a player at all.
    bool gives = true;

    /// Why the film the next player is given will not open, if it will not.
    std::optional<core::PlayerError> refusal;

    /// The audio tracks of the film the next player is given — none for a film without sound.
    std::vector<core::AudioTrack> tracks;

    /// What came out, and what it was built for.
    FakeVideoPlayer* player = nullptr;
    int built = 0;
};

/// The factory `booth` answers, which must outlive the window taking it.
[[nodiscard]] inline gui::PlayerFactory projecting(Projectionist& booth) {
    return [&booth]() -> std::unique_ptr<core::VideoPlayer> {
        ++booth.built;
        if (!booth.gives)
            return nullptr;

        auto made = std::make_unique<FakeVideoPlayer>();
        made->refusal = booth.refusal;
        made->tracks = booth.tracks;
        booth.player = made.get();
        return made;
    };
}

/// The file at `path` of `files`, opened as a user would open it.
[[nodiscard]] inline core::OpenedFile fileIn(const core::InMemoryFileSystem& files,
                                             const char* path) {
    auto opened = core::openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

} // namespace subedit::test
