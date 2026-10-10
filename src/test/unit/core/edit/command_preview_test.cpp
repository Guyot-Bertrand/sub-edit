#include <subedit/core/edit/command_preview.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

using subedit::core::CommandPreview;
using subedit::core::Duration;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::ShiftCommand;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Project fiveSubtitles() {
    std::vector<Subtitle> subtitles;
    for (std::int64_t second = 1; second <= 5; ++second) {
        subtitles.push_back(Subtitle{.start = Timestamp::fromMilliseconds(second * 1000),
                                     .end = Timestamp::fromMilliseconds((second * 1000) + 500)});
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

} // namespace

TEST_CASE("a preview says what a command would change, and changes nothing", "[edit][preview]") {
    const Project project = fiveSubtitles();
    ShiftCommand shift{Selection::all(project), Duration::fromMilliseconds(250)};

    const CommandPreview preview = previewOf(project, shift, 10);

    CHECK(preview.changed == 5);
    CHECK(preview.shown.size() == 5);
    CHECK(preview.shown.front().before.start == Timestamp::fromMilliseconds(1000));
    CHECK(preview.shown.front().after.start == Timestamp::fromMilliseconds(1250));
    // The project itself is where it was.
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).start ==
          Timestamp::fromMilliseconds(1000));
}

TEST_CASE("a preview shows the first changes and counts them all", "[edit][preview]") {
    const Project project = fiveSubtitles();
    ShiftCommand shift{Selection::all(project), Duration::fromMilliseconds(250)};

    const CommandPreview preview = previewOf(project, shift, 2);

    CHECK(preview.changed == 5);
    CHECK(preview.shown.size() == 2);
    CHECK(preview.shown.back().index == SubtitleIndex::fromValue(1));
}

TEST_CASE("a preview of what changes nothing is empty", "[edit][preview]") {
    const Project project = fiveSubtitles();
    ShiftCommand shift{Selection::all(project), Duration::fromMilliseconds(0)};

    const CommandPreview preview = previewOf(project, shift, 10);

    CHECK(preview.changed == 0);
    CHECK(preview.shown.empty());
}

TEST_CASE("a preview leaves out what the selection leaves out", "[edit][preview]") {
    const Project project = fiveSubtitles();
    ShiftCommand shift{Selection::range(SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2)),
                       Duration::fromMilliseconds(250)};

    const CommandPreview preview = previewOf(project, shift, 10);

    CHECK(preview.changed == 2);
    CHECK(preview.shown.front().index == SubtitleIndex::fromValue(1));
}
