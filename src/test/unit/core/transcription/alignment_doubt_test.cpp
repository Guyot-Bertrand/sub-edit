// What is doubtful in an alignment, and the figures that tell its story (issue #811, phase 23).

#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "alignment_cases.hpp"

namespace {

using subedit::core::AlignmentReport;
using subedit::core::LineDoubt;
using subedit::core::LineMatch;
using subedit::core::Timestamp;

Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

/// A line found at `start`, with `matched` of its `words` heard.
LineMatch found(std::int64_t start, std::size_t words = 5, std::size_t matched = 5) {
    return {
        .place = {.start = at(start), .end = at(start + 1000)}, .words = words, .matched = matched};
}

LineMatch notFound() {
    return {.place = {}, .words = 5, .matched = 1};
}

/// The file's own starts, `spacing` apart.
std::vector<Timestamp> startsOf(std::size_t count, std::int64_t spacing = 3000) {
    std::vector<Timestamp> starts;
    starts.reserve(count);
    for (std::size_t line = 0; line < count; ++line)
        starts.push_back(at(static_cast<std::int64_t>(line) * spacing));
    return starts;
}

/// The matching of a case, judged against the times its file gives.
AlignmentReport reportOf(const std::string& name) {
    const subedit::test::AlignmentCase wanted = subedit::test::loadAlignmentCase(name);
    const std::string text = subedit::test::detail::readText(wanted.heard);
    const auto output =
        subedit::core::readWhisperJson(text).value_or(subedit::core::WhisperOutput{});
    std::vector<subedit::core::TimedWord> words;
    for (const auto& segment : output.transcription.segments)
        words.insert(words.end(), segment.words.begin(), segment.words.end());
    std::vector<Timestamp> given;
    given.reserve(wanted.given.size());
    for (const auto& place : wanted.given)
        given.push_back(place.start);
    return subedit::core::judgeAlignment(subedit::core::matchLinesToWords(wanted.lines, words),
                                         given);
}

} // namespace

TEST_CASE("a clean alignment has nothing to flag", "[alignment][doubt]") {
    const std::vector<LineMatch> matches{found(1000), found(4000), found(7000)};

    const AlignmentReport report = subedit::core::judgeAlignment(matches, startsOf(3));

    CHECK(report.sure == 3);
    CHECK(report.partial == 0);
    CHECK(report.outOfLine == 0);
    CHECK(report.notFound == 0);
    CHECK(report.positioned() == 3);
    CHECK(report.lines[0].shiftMilliseconds == 1000);
}

TEST_CASE("a line not found is flagged and has no shift", "[alignment][doubt]") {
    const std::vector<LineMatch> matches{found(0), notFound(), found(6000)};

    const AlignmentReport report = subedit::core::judgeAlignment(matches, startsOf(3));

    CHECK(report.notFound == 1);
    CHECK(report.lines[1].doubt == LineDoubt::NotFound);
    CHECK_FALSE(report.lines[1].shiftMilliseconds.has_value());
    CHECK(report.positioned() == 2);
}

TEST_CASE("a line found on a few of its words is partial", "[alignment][doubt]") {
    // Four of five is 0.8: enough. Three of five is not.
    const std::vector<LineMatch> matches{found(0, 5, 4), found(3000, 5, 3), found(6000)};

    const AlignmentReport report = subedit::core::judgeAlignment(matches, startsOf(3));

    CHECK(report.lines[0].doubt == LineDoubt::None);
    CHECK(report.lines[1].doubt == LineDoubt::Partial);
    CHECK(report.partial == 1);
    CHECK(report.sure == 2);
}

TEST_CASE("a line whose shift departs from its neighbours' is out of line", "[alignment][doubt]") {
    // Everything moves by 500 ms, but the fourth line lands five seconds further.
    const std::vector<LineMatch> matches{found(500),
                                         found(3500),
                                         found(6500),
                                         found(14500),
                                         found(12500),
                                         found(15500),
                                         found(18500)};

    const AlignmentReport report = subedit::core::judgeAlignment(matches, startsOf(7));

    CHECK(report.lines[3].doubt == LineDoubt::OutOfLine);
    CHECK(report.outOfLine == 1);
    // The ones next to it are not dragged along.
    CHECK(report.lines[2].doubt == LineDoubt::None);
    CHECK(report.lines[4].doubt == LineDoubt::None);
}

TEST_CASE("the gap is the caller's", "[alignment][doubt]") {
    const std::vector<LineMatch> matches{
        found(0), found(3000), found(7000), found(9000), found(12000)};

    CHECK(subedit::core::judgeAlignment(matches, startsOf(5)).outOfLine == 0);
    CHECK(subedit::core::judgeAlignment(matches, startsOf(5), {.gapMilliseconds = 500}).outOfLine ==
          1);
}

TEST_CASE("a partial line is not called out of line as well", "[alignment][doubt]") {
    const std::vector<LineMatch> matches{
        found(0), found(3000), found(15000, 5, 2), found(9000), found(12000)};

    const AlignmentReport report = subedit::core::judgeAlignment(matches, startsOf(5));

    CHECK(report.lines[2].doubt == LineDoubt::Partial);
    CHECK(report.outOfLine == 0);
}

TEST_CASE("an offset reads as a median and a drift as a difference", "[alignment][doubt]") {
    const AlignmentReport late = reportOf("offset-plus-2400");
    CHECK(late.medianShiftMilliseconds == -2400);
    CHECK(late.driftMilliseconds == 0);
    CHECK(late.notFound == 0);
    CHECK(late.outOfLine == 0);

    const AlignmentReport drifting = reportOf("drift");
    CHECK(drifting.driftMilliseconds < -2000);
    CHECK(drifting.outOfLine == 0);
}

TEST_CASE("the lines nobody says are the ones flagged, and nothing else", "[alignment][doubt]") {
    const AlignmentReport extra = reportOf("extra-line");
    CHECK(extra.notFound == 1);
    CHECK(extra.lines[5].doubt == LineDoubt::NotFound);

    const AlignmentReport unheard = reportOf("unheard-line");
    CHECK(unheard.notFound == 1);
    CHECK(unheard.outOfLine == 0);

    // Half of the file is right and half two seconds late: a step, not a flaw in either half.
    const AlignmentReport half = reportOf("partial");
    CHECK(half.notFound == 0);
    CHECK(half.outOfLine == 0);
}

TEST_CASE("nothing to judge gives empty figures", "[alignment][doubt]") {
    const AlignmentReport none = subedit::core::judgeAlignment({}, {});
    CHECK(none.lines.empty());
    CHECK(none.medianShiftMilliseconds == 0);
    CHECK(none.driftMilliseconds == 0);

    // One positioned line has no drift; a missing original time is read as no position.
    const std::vector<LineMatch> one{found(1000), found(4000)};
    const AlignmentReport single = subedit::core::judgeAlignment(one, startsOf(1));
    CHECK(single.driftMilliseconds == 0);
    CHECK(single.lines[1].doubt == LineDoubt::NotFound);
}
