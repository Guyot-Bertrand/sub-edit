// The matching of a text to the words an engine heard (issue #810, phase 23).
//
// Held against the cases of #803: each has a file at a known fault, what an engine would have
// heard and the truth, and the matching is judged by how far it lands from the truth, how many
// lines it misses and — above all — how many it invents.

#include <subedit/core/transcription/whisper_cpp_output.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "alignment_cases.hpp"

namespace {

using subedit::core::AlignedLine;
using subedit::core::LineMatch;
using subedit::core::TimedWord;
using subedit::core::Timestamp;
using subedit::test::AlignmentCase;

/// Every word the case's engine heard, in order.
std::vector<TimedWord> heardIn(const AlignmentCase& wanted) {
    const std::string text = subedit::test::detail::readText(wanted.heard);
    const auto output =
        subedit::core::readWhisperJson(text).value_or(subedit::core::WhisperOutput{});
    std::vector<TimedWord> words;
    for (const auto& segment : output.transcription.segments)
        words.insert(words.end(), segment.words.begin(), segment.words.end());
    return words;
}

std::vector<AlignedLine> placesOf(const std::vector<LineMatch>& matches) {
    std::vector<AlignedLine> places;
    places.reserve(matches.size());
    for (const LineMatch& match : matches)
        places.push_back(match.place);
    return places;
}

TimedWord word(const std::string& text, std::int64_t start, std::int64_t end) {
    return {.text = text,
            .start = Timestamp::fromMilliseconds(start),
            .end = Timestamp::fromMilliseconds(end)};
}

} // namespace

TEST_CASE("every case is matched to the truth, with nothing invented", "[alignment][matching]") {
    for (const std::string name : {"offset-plus-2400",
                                   "offset-minus-900",
                                   "drift",
                                   "partial",
                                   "extra-line",
                                   "unheard-line"}) {
        INFO(name);
        const AlignmentCase wanted = subedit::test::loadAlignmentCase(name);
        const auto matches = subedit::core::matchLinesToWords(wanted.lines, heardIn(wanted));

        const auto quality = subedit::test::measure(placesOf(matches), wanted);

        CHECK(quality.worst <= 500);
        CHECK(quality.invented == 0);
        CHECK(quality.missed == (name == "unheard-line" ? 1U : 0U));
    }
}

TEST_CASE("a line is placed from its first heard word to its last", "[alignment][matching]") {
    const std::vector<TimedWord> heard{
        word("Hello,", 1000, 1400), word("old", 1500, 1800), word("friend.", 1900, 2600)};

    const auto matches = subedit::core::matchLinesToWords({"Hello, old friend."}, heard);

    REQUIRE(matches.size() == 1);
    CHECK(matches[0].words == 3);
    CHECK(matches[0].matched == 3);
    CHECK(matches[0].place.start == Timestamp::fromMilliseconds(1000));
    CHECK(matches[0].place.end == Timestamp::fromMilliseconds(2600));
}

TEST_CASE("case, accents, punctuation and markup do not matter", "[alignment][matching]") {
    const std::vector<TimedWord> heard{
        word("Cafe", 100, 400), word("noir", 500, 900), word("tonight", 1000, 1500)};

    const auto matches =
        subedit::core::matchLinesToWords({"<i>CAFÉ</i> — noir, [music] tonight!"}, heard);

    CHECK(matches[0].words == 3);
    CHECK(matches[0].matched == 3);
    CHECK(matches[0].place.start == Timestamp::fromMilliseconds(100));
}

TEST_CASE("a slip of the ear on a long word still counts as heard", "[alignment][matching]") {
    const std::vector<TimedWord> heard{
        word("the", 0, 100), word("lighthouse", 200, 900), word("keeper", 1000, 1500)};

    const auto near = subedit::core::matchLinesToWords({"the lighthouze keeper"}, heard);
    CHECK(near[0].matched == 3);

    // A short word one letter off is another word.
    const std::vector<TimedWord> other{word("the", 0, 100)};
    const auto far = subedit::core::matchLinesToWords({"he"}, other);
    CHECK(far[0].matched == 0);
    CHECK_FALSE(far[0].place.start.has_value());
}

TEST_CASE("a line is found only when enough of its words are heard", "[alignment][matching]") {
    const std::vector<TimedWord> heard{word("quiet", 0, 300), word("harbour", 400, 900)};

    // Two words of four: half, so found; one of four: not found.
    const auto half = subedit::core::matchLinesToWords({"quiet harbour again today"}, heard);
    CHECK(half[0].matched == 2);
    CHECK(half[0].place.start.has_value());

    const auto less = subedit::core::matchLinesToWords({"quiet window again today"}, heard);
    CHECK(less[0].matched == 1);
    CHECK_FALSE(less[0].place.start.has_value());

    // The limit is the caller's.
    const auto strict =
        subedit::core::matchLinesToWords({"quiet harbour again today"}, heard, 0.75);
    CHECK_FALSE(strict[0].place.start.has_value());
}

TEST_CASE("a line without a word, or with nothing heard, has no place", "[alignment][matching]") {
    const auto matches = subedit::core::matchLinesToWords({"♪ ♪", "Something"}, {});

    REQUIRE(matches.size() == 2);
    CHECK(matches[0].words == 0);
    CHECK_FALSE(matches[0].place.start.has_value());
    CHECK_FALSE(matches[1].place.start.has_value());
    const std::vector<TimedWord> alone{word("alone", 0, 10)};
    CHECK(subedit::core::matchLinesToWords({}, alone).empty());
}

TEST_CASE("the places never go back, whatever the text repeats", "[alignment][matching]") {
    std::vector<std::string> lines;
    std::vector<TimedWord> heard;
    for (std::int64_t at = 0; at < 40; ++at) {
        const std::int64_t base = at * 1000;
        const std::string odd = at % 2 == 0 ? "never" : "always";
        lines.push_back("No, no, " + odd + " again");
        heard.push_back(word("no", base, base + 200));
        heard.push_back(word("no", base + 250, base + 450));
        heard.push_back(word(odd, base + 500, base + 800));
        heard.push_back(word("again", base + 850, base + 990));
    }

    const auto matches = subedit::core::matchLinesToWords(lines, heard);

    std::optional<Timestamp> previous;
    for (const LineMatch& match : matches) {
        REQUIRE(match.place.start.has_value());
        const Timestamp start = match.place.start.value_or(Timestamp::fromMilliseconds(0));
        CHECK(previous.value_or(start) <= start);
        previous = start;
    }
}

TEST_CASE("an override of a subtitle format is not said", "[alignment][matching]") {
    const std::vector<TimedWord> heard{word("hello", 0, 300), word("there", 400, 800)};

    const auto matches = subedit::core::matchLinesToWords({"{\\an8}Hello there"}, heard);

    CHECK(matches[0].words == 2);
    CHECK(matches[0].matched == 2);
}

TEST_CASE("words said in another order than the text are not forced into it",
          "[alignment][matching]") {
    const std::vector<TimedWord> heard{word("marigold", 0, 400), word("tangerine", 500, 900)};

    const auto matches = subedit::core::matchLinesToWords({"tangerine marigold"}, heard);

    // The two cannot both hold in order: one is heard, and the line is found on it.
    CHECK(matches[0].matched == 1);
    CHECK(matches[0].place.start.has_value());
}

TEST_CASE("a word the text does not have is skipped over", "[alignment][matching]") {
    const std::vector<TimedWord> heard{
        word("he", 0, 100), word("um", 150, 250), word("is", 300, 400)};

    const auto matches = subedit::core::matchLinesToWords({"he is"}, heard);

    CHECK(matches[0].matched == 2);
    CHECK(matches[0].place.end == Timestamp::fromMilliseconds(400));
}
