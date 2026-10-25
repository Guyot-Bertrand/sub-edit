// Turning what an engine cut into subtitles.

#include <subedit/core/transcription/subtitle_assembly.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::assembleSubtitles;
using subedit::core::AssemblySettings;
using subedit::core::Duration;
using subedit::core::Subtitle;
using subedit::core::TimedWord;
using subedit::core::Timestamp;
using subedit::core::TranscribedSegment;

Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

TranscribedSegment segment(std::int64_t start, std::int64_t end, std::string text) {
    return {.text = std::move(text), .start = at(start), .end = at(end)};
}

TimedWord word(std::string text, std::int64_t start, std::int64_t end) {
    return TimedWord{.text = std::move(text), .start = at(start), .end = at(end)};
}

std::vector<std::string> textsOf(const std::vector<Subtitle>& subtitles) {
    std::vector<std::string> texts;
    texts.reserve(subtitles.size());
    for (const Subtitle& subtitle : subtitles)
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("nothing in, nothing out", "[assembly]") {
    CHECK(assembleSubtitles({}).empty());
}

TEST_CASE("a segment that fits is a subtitle of its own", "[assembly]") {
    const std::vector<TranscribedSegment> segments{segment(1000, 3000, "Hello there.")};

    const auto subtitles = assembleSubtitles(segments);

    REQUIRE(subtitles.size() == 1);
    CHECK(subtitles.front().start == at(1000));
    CHECK(subtitles.front().end == at(3000));
    CHECK(subtitles.front().mainText == "Hello there.");
}

TEST_CASE("two close pieces of one sentence are one subtitle", "[assembly]") {
    const std::vector<TranscribedSegment> segments{segment(0, 1500, "I was going to"),
                                                   segment(1700, 3000, "tell you something.")};

    const auto subtitles = assembleSubtitles(segments);

    REQUIRE(subtitles.size() == 1);
    CHECK(subtitles.front().mainText == "I was going to tell you something.");
    CHECK(subtitles.front().start == at(0));
    CHECK(subtitles.front().end == at(3000));
}

TEST_CASE("the end of a sentence is never merged across", "[assembly]") {
    for (const char* ending : {"Done.", "Done!", "Done?", "Done\xE2\x80\xA6"}) {
        const std::vector<TranscribedSegment> segments{segment(0, 1000, ending),
                                                       segment(1100, 2000, "Next.")};
        CHECK(assembleSubtitles(segments).size() == 2);
    }
}

TEST_CASE("pieces too far apart stay apart", "[assembly]") {
    const std::vector<TranscribedSegment> segments{segment(0, 1000, "one"),
                                                   segment(2000, 3000, "two")};

    CHECK(textsOf(assembleSubtitles(segments)) == std::vector<std::string>{"one", "two"});
}

TEST_CASE("pieces of different speakers stay apart", "[assembly]") {
    TranscribedSegment first = segment(0, 1000, "one");
    TranscribedSegment second = segment(1100, 2000, "two");
    first.speaker = "A";
    second.speaker = "B";
    const std::vector<TranscribedSegment> segments{first, second};

    CHECK(assembleSubtitles(segments).size() == 2);
}

TEST_CASE("pieces of the same speaker are merged", "[assembly]") {
    TranscribedSegment first = segment(0, 1000, "one");
    TranscribedSegment second = segment(1100, 2000, "two");
    first.speaker = "A";
    second.speaker = "A";
    const std::vector<TranscribedSegment> segments{first, second};

    CHECK(textsOf(assembleSubtitles(segments)) == std::vector<std::string>{"one two"});
}

TEST_CASE("a merge that would exceed the limits is not made", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(3000),
                                    .maxCharacters = 20};

    const std::vector<TranscribedSegment> tooLong{segment(0, 2500, "one"),
                                                  segment(2600, 4000, "two")};
    const std::vector<TranscribedSegment> tooWide{segment(0, 1000, "twelve chars"),
                                                  segment(1100, 1500, "and more")};

    CHECK(assembleSubtitles(tooLong, settings).size() == 2);
    CHECK(assembleSubtitles(tooWide, settings).size() == 2);
}

TEST_CASE("a long segment is split at the timed words", "[assembly]") {
    TranscribedSegment wordy = segment(0, 10000, "one two three four five six");
    wordy.words = {word("one", 0, 1500),
                   word("two", 1600, 3000),
                   word("three", 3100, 4500),
                   word("four", 4600, 6000),
                   word("five", 6100, 7500),
                   word("six", 7600, 10000)};
    const std::vector<TranscribedSegment> segments{wordy};

    const auto subtitles = assembleSubtitles(segments);

    REQUIRE(subtitles.size() == 2);
    CHECK(subtitles[0].mainText == "one two three four");
    CHECK(subtitles[0].start == at(0));
    CHECK(subtitles[0].end == at(6000));
    CHECK(subtitles[1].mainText == "five six");
    CHECK(subtitles[1].start == at(6100));
    CHECK(subtitles[1].end == at(10000));
}

TEST_CASE("a long segment without words is split in proportion to the letters", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(60000),
                                    .maxCharacters = 10};
    const std::vector<TranscribedSegment> segments{segment(0, 10000, "aaaa bbbb cccc dddd")};

    const auto subtitles = assembleSubtitles(segments, settings);

    REQUIRE(subtitles.size() == 2);
    CHECK(subtitles[0].mainText == "aaaa bbbb");
    CHECK(subtitles[0].start == at(0));
    CHECK(subtitles[0].end == at(5000));
    CHECK(subtitles[1].mainText == "cccc dddd");
    CHECK(subtitles[1].end == at(10000));
}

TEST_CASE("a word longer than the limit is kept whole", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(60000),
                                    .maxCharacters = 5};
    const std::vector<TranscribedSegment> segments{segment(0, 4000, "extraordinary ok")};

    CHECK(textsOf(assembleSubtitles(segments, settings)) ==
          std::vector<std::string>{"extraordinary", "ok"});
}

TEST_CASE("the characters of a text are counted, not its bytes", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(60000),
                                    .maxCharacters = 6};
    // Six characters, twelve bytes: it fits.
    const std::vector<TranscribedSegment> segments{
        segment(0, 1000, "\xC3\xA9\xC3\xA9\xC3\xA9\xC3\xA9\xC3\xA9\xC3\xA9")};

    CHECK(assembleSubtitles(segments, settings).size() == 1);
}

TEST_CASE("a segment with no text at all gives no subtitle", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(100),
                                    .maxCharacters = 84};
    const std::vector<TranscribedSegment> segments{segment(0, 5000, "")};

    CHECK(assembleSubtitles(segments, settings).empty());
}

TEST_CASE("a huge piece is split without overflowing", "[assembly]") {
    const AssemblySettings settings{.maxGap = Duration::fromMilliseconds(400),
                                    .maxDuration = Duration::fromMilliseconds(1000000000000),
                                    .maxCharacters = 5};
    const std::vector<TranscribedSegment> segments{segment(0, 1000000000000, "aaaaa bbbbb ccccc")};

    const auto subtitles = assembleSubtitles(segments, settings);

    REQUIRE(subtitles.size() == 3);
    CHECK(subtitles.front().start == at(0));
    CHECK(subtitles.back().end == at(1000000000000));
}

// ## Placeholders

TEST_CASE("a piece with no letter and no digit is dropped", "[assembly][placeholders]") {
    const std::vector<TranscribedSegment> segments{segment(0, 5000, "..."),
                                                   segment(5000, 8000, "Hello there."),
                                                   segment(8000, 15000, "\xE2\x80\xA6"),
                                                   segment(15000, 16000, "\xE2\x99\xAA"),
                                                   segment(16000, 17000, " - ")};

    CHECK(textsOf(assembleSubtitles(segments)) == std::vector<std::string>{"Hello there."});
}

TEST_CASE("a piece with a letter or a digit in any script is kept", "[assembly][placeholders]") {
    const std::vector<TranscribedSegment> segments{
        segment(0, 1000, "...2024..."),
        segment(5000, 6000, "\xCE\xB1\xCE\xB2"),
        segment(10000, 11000, "\xE4\xBD\xA0\xE5\xA5\xBD")};

    CHECK(assembleSubtitles(segments).size() == 3);
}

TEST_CASE("a dropped piece does not stand between two pieces that merge",
          "[assembly][placeholders]") {
    const std::vector<TranscribedSegment> segments{segment(0, 1500, "I was going to"),
                                                   segment(1550, 1600, "..."),
                                                   segment(1700, 3000, "tell you.")};

    CHECK(textsOf(assembleSubtitles(segments)) ==
          std::vector<std::string>{"I was going to tell you."});
}

// ## Where a subtitle comes from

TEST_CASE("each subtitle names the pieces it was made from", "[assembly][sources]") {
    const std::vector<TranscribedSegment> segments{segment(0, 1500, "I was going to"),
                                                   segment(1600, 3000, "tell you something."),
                                                   segment(20000, 21000, "..."),
                                                   segment(30000, 32000, "Another one.")};

    const auto made = subedit::core::assemble(segments);

    REQUIRE(made.subtitles.size() == 2);
    REQUIRE(made.sources.size() == 2);
    // Merged: pieces 0 and 1. The dropped placeholder (2) is the source of none.
    CHECK(made.sources[0] == std::vector<std::size_t>{0, 1});
    CHECK(made.sources[1] == std::vector<std::size_t>{3});
}

TEST_CASE("a piece cut in several subtitles is the source of each", "[assembly][sources]") {
    AssemblySettings settings;
    settings.maxCharacters = 12;
    const std::vector<TranscribedSegment> segments{
        segment(0, 6000, "alpha beta gamma delta epsilon zeta")};

    const auto made = subedit::core::assemble(segments, settings);

    REQUIRE(made.subtitles.size() > 1);
    REQUIRE(made.sources.size() == made.subtitles.size());
    for (const auto& from : made.sources)
        CHECK(from == std::vector<std::size_t>{0});
}

TEST_CASE("the subtitles of assemble are those of assembleSubtitles", "[assembly][sources]") {
    const std::vector<TranscribedSegment> segments{segment(0, 1500, "I was going to"),
                                                   segment(1600, 3000, "tell you."),
                                                   segment(9000, 9500, "...")};

    CHECK(textsOf(subedit::core::assemble(segments).subtitles) ==
          textsOf(assembleSubtitles(segments)));
}
