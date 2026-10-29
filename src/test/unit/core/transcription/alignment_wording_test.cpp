// What both surfaces say of an alignment (issue #813, phase 23).

#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/wording/alignment.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::AlignmentReport;
using subedit::core::LineDoubt;

} // namespace

TEST_CASE("an alignment is told by what was placed, what is to check and the figures",
          "[wording][alignment]") {
    AlignmentReport report;
    report.lines.resize(42);
    report.sure = 38;
    report.partial = 1;
    report.outOfLine = 1;
    report.notFound = 2;
    report.medianShiftMilliseconds = -2400;
    report.driftMilliseconds = 150;

    const std::string said = subedit::core::noticeOfAlignment(report);

    CHECK_THAT(said, ContainsSubstring("40 of 42 subtitles placed on the audio, 4 to check"));
    CHECK_THAT(said, ContainsSubstring("median shift -2.400 s"));
    CHECK_THAT(said, ContainsSubstring("drift 0.150 s"));
}

TEST_CASE("every doubt has words, and being sure has none", "[wording][alignment]") {
    CHECK(subedit::core::doubtText(LineDoubt::None).empty());
    for (const LineDoubt doubt : {LineDoubt::NotFound, LineDoubt::Partial, LineDoubt::OutOfLine})
        CHECK_FALSE(subedit::core::doubtText(doubt).empty());
    CHECK_FALSE(subedit::core::lineCutBackText().empty());
    CHECK_FALSE(subedit::core::lineOverlapsText().empty());
}

TEST_CASE("the reasons it did not happen name what to change", "[wording][alignment]") {
    CHECK_THAT(subedit::core::noSuchAlignmentEngine("nope"), ContainsSubstring("nope"));
    CHECK_THAT(subedit::core::needsOneAudioSource(), ContainsSubstring("--video or --audio"));
    CHECK_THAT(subedit::core::noLineFoundInTheAudio(1), ContainsSubstring("1 subtitle"));
    CHECK_THAT(subedit::core::noLineFoundInTheAudio(12), ContainsSubstring("none of the 12"));
}
