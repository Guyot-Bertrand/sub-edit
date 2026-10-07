// What a frame lasts, for a step of frames — issue #618, decision D6.

#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/frame_step.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

using subedit::core::countedFrameRateOf;
using subedit::core::FrameRate;
using subedit::core::FrameRateSource;
using subedit::core::MicroDvdFile;
using subedit::core::movedByFrames;
using subedit::core::Project;
using subedit::core::SourceFile;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;

/// A project whose starts all sit on the 25-frame grid, irregularly enough to be a grid and not a
/// pattern: frame numbers that no multiple of another rate explains.
[[nodiscard]] Project onTheGridOf25() {
    std::vector<Subtitle> subtitles;
    for (const std::int64_t frame : {3,   11,  17,  29,  41,  53,  67,  79,  97,  101, 131, 149,
                                     173, 199, 211, 241, 263, 281, 307, 331, 359, 389, 401, 433}) {
        const Timestamp start = Timestamp::fromMilliseconds(frame * 40);
        subtitles.push_back(Subtitle{.start = start, .end = start});
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

} // namespace

TEST_CASE("the video's own rate counts first", "[video][frame-step][GUI-STEP-03]") {
    Project project = onTheGridOf25();
    SourceFile counted;
    counted.format = SubtitleFormat::MicroDvd;
    counted.extras = MicroDvdFile{.rate = FrameRate{StandardFrameRate::Fps24}};
    project.setSourceFile(counted);
    project.chooseVideo("/films/film.mkv");
    project.setDeclaredFrameRate(FrameRate{StandardFrameRate::Fps23976});

    const auto found = countedFrameRateOf(project);

    CHECK((found.has_value() && found->source == FrameRateSource::Video));
    CHECK((found.has_value() && found->rate == FrameRate{StandardFrameRate::Fps23976}));
}

TEST_CASE("without a declared rate, a document counted in frames gives its own",
          "[video][frame-step][GUI-STEP-03]") {
    Project project = onTheGridOf25();
    SourceFile counted;
    counted.format = SubtitleFormat::MicroDvd;
    counted.extras = MicroDvdFile{.rate = FrameRate{StandardFrameRate::Fps24}};
    project.setSourceFile(counted);
    project.chooseVideo("/films/film.mkv");

    const auto found = countedFrameRateOf(project);

    CHECK((found.has_value() && found->source == FrameRateSource::Document));
    CHECK((found.has_value() && found->rate == FrameRate{StandardFrameRate::Fps24}));
}

TEST_CASE("without either, the grid the positions fall on", "[video][frame-step][GUI-STEP-03]") {
    const auto found = countedFrameRateOf(onTheGridOf25());

    CHECK((found.has_value() && found->source == FrameRateSource::Grid));
    CHECK((found.has_value() && found->rate == FrameRate{StandardFrameRate::Fps25}));
}

// Choosing a rate at random would move every edge by something that is not a frame.
TEST_CASE("with none of the three there is no rate, which is an answer",
          "[video][frame-step][GUI-STEP-03]") {
    Project project;
    project.setSubtitles({Subtitle{.start = Timestamp::fromMilliseconds(1000),
                                   .end = Timestamp::fromMilliseconds(2000)},
                          Subtitle{.start = Timestamp::fromMilliseconds(2500),
                                   .end = Timestamp::fromMilliseconds(3500)}});

    CHECK_FALSE(countedFrameRateOf(project).has_value());
    CHECK_FALSE(countedFrameRateOf(Project{}).has_value());
}

TEST_CASE("a step of N frames is N frames at any rate", "[video][frame-step][GUI-STEP-04]") {
    const FrameRate pal{StandardFrameRate::Fps25};
    const FrameRate ntsc{StandardFrameRate::Fps23976};

    CHECK(movedByFrames(Timestamp::fromMilliseconds(1000), pal, 1).milliseconds() == 1040);
    CHECK(movedByFrames(Timestamp::fromMilliseconds(1000), pal, 5).milliseconds() == 1200);
    CHECK(movedByFrames(Timestamp::fromMilliseconds(1000), pal, -5).milliseconds() == 800);
    // 24 frames of 23.976 images a second last 1001 milliseconds, exactly.
    CHECK(movedByFrames(Timestamp::fromMilliseconds(0), ntsc, 24).milliseconds() == 1001);
    // And a frame of it is a fraction of a millisecond: 41.708, rounded once.
    CHECK(movedByFrames(Timestamp::fromMilliseconds(417), ntsc, 1).milliseconds() == 459);
}

TEST_CASE("a step back never goes before the origin", "[video][frame-step][GUI-NUDGE-01]") {
    const FrameRate pal{StandardFrameRate::Fps25};

    CHECK(movedByFrames(Timestamp::fromMilliseconds(30), pal, -1).milliseconds() == 0);
    CHECK(movedByFrames(Timestamp::fromMilliseconds(0), pal, -100).milliseconds() == 0);
}
