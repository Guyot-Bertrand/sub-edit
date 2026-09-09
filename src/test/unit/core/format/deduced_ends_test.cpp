// The ends two of the nine formats do not carry, and the rule that fills them.
//
// LRC and TMPlayer hold one position per line. `deduceEnds` is what both
// readers call once they have their starts, and it is tested here rather than
// twice over: what it decides — ADR 0029 — belongs to neither format.

#include <subedit/core/format/deduced_ends.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

namespace {

using subedit::core::deduceEnds;
using subedit::core::DiagnosticKind;
using subedit::core::ReadResult;
using subedit::core::Severity;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle startingAt(std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start)};
}

} // namespace

TEST_CASE("each end is the next start, and the last one is five seconds later", "[format][ends]") {
    ReadResult result;
    result.subtitles = {startingAt(1000), startingAt(4000), startingAt(8000)};

    deduceEnds(result);

    REQUIRE(result.subtitles.size() == 3);
    CHECK(result.subtitles[0].end.milliseconds() == 4000);
    CHECK(result.subtitles[1].end.milliseconds() == 8000);
    CHECK(result.subtitles[2].end.milliseconds() == 13000);
}

TEST_CASE("a single subtitle gets the five seconds and nothing else", "[format][ends]") {
    ReadResult result;
    result.subtitles = {startingAt(1000)};

    deduceEnds(result);

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].end.milliseconds() == 6000);
}

TEST_CASE("the deduction says it happened, about the file rather than a line", "[format][ends]") {
    ReadResult result;
    result.subtitles = {startingAt(1000)};

    deduceEnds(result);

    REQUIRE(result.diagnostics.size() == 1);
    CHECK(result.diagnostics.front().kind == DiagnosticKind::DeducedEnds);
    CHECK(result.diagnostics.front().severity == Severity::Recovered);
    CHECK(result.diagnostics.front().line == subedit::core::kWholeFile);
}

TEST_CASE("a reading that found nothing is left alone, and says nothing", "[format][ends]") {
    // **The guard, and why it is here rather than left to the callers.** Both
    // readers refuse a file they found no subtitle in, so nothing reaches this
    // today — but the last subtitle is read unconditionally just below, and a
    // function that would be undefined on an empty vector is not one to hand
    // out. Saying nothing matters as much: a diagnostic about ends that were
    // not deduced would be a sentence about nothing.
    ReadResult result;

    deduceEnds(result);

    CHECK(result.subtitles.empty());
    CHECK(result.diagnostics.empty());
}
