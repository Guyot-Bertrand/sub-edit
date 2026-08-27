#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <string_view>

using subedit::core::FrameRate;
using subedit::core::nameOf;
using subedit::core::StandardFrameRate;

namespace {

/// A rate outside the eight standards, unwrapped where the test can say so.
FrameRate rateOf(std::int64_t numerator, std::int64_t denominator) {
    const std::optional<FrameRate> created = FrameRate::create(numerator, denominator);
    if (!created.has_value()) {
        FAIL("a frame rate with two positive terms must be accepted");
        return FrameRate{StandardFrameRate::Fps25};
    }
    return *created;
}

} // namespace

TEST_CASE("every kind of anomaly has a clause", "[cli][wording]") {
    // Named one by one rather than looped over, for the same reason as below.
    // The clauses follow « subtitle 12 », so each starts with its verb.
    using subedit::core::AnomalyKind;
    CHECK(nameOf(AnomalyKind::EndBeforeStart) == "ends before it starts");
    CHECK(nameOf(AnomalyKind::OverlappingSubtitles) == "starts before the previous one ends");
    CHECK(nameOf(AnomalyKind::OutOfOrder) == "starts before the previous one starts");
}

TEST_CASE("every kind of diagnostic has a phrase", "[cli][wording]") {
    // Named one by one rather than looped over: a new enumerator must fail to
    // compile here, not fall through to an empty line in a report.
    using subedit::core::DiagnosticKind;
    CHECK(nameOf(DiagnosticKind::IgnoredLine) == "a line that fits nowhere");
    CHECK(nameOf(DiagnosticKind::MalformedTimestamp) == "a timing line that could not be read");
    CHECK(nameOf(DiagnosticKind::MissingNumbering) == "a SubRip block without its number");
    CHECK(nameOf(DiagnosticKind::InconsistentNumbering) == "SubRip numbers that do not follow");
    CHECK(nameOf(DiagnosticKind::TextBeforeAnyTimestamp) == "text before the first timing line");
    CHECK(nameOf(DiagnosticKind::UnknownBlock) == "a WebVTT block of an unknown kind");
    CHECK(nameOf(DiagnosticKind::MixedNewlines) == "more than one kind of line ending");
}

TEST_CASE("what was done about an anomaly is named too", "[cli][wording]") {
    using subedit::core::Severity;
    // The whole point of the distinction: one of the two needs a human.
    CHECK(nameOf(Severity::Warning) == "left as it stands");
    CHECK(nameOf(Severity::Recovered) == "settled by the reader");
}

TEST_CASE("a whole rate is named by its number alone", "[cli][wording]") {
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps25}) == "25");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps24}) == "24");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps60}) == "60");
}

TEST_CASE("a rate a decimal can write exactly is named as a decimal", "[cli][wording]") {
    CHECK(nameOf(rateOf(239, 10)) == "23.9");
    CHECK(nameOf(rateOf(25, 2)) == "12.5");
    CHECK(nameOf(rateOf(23976, 1000)) == "23.976");
}

TEST_CASE("an NTSC rate is named by its fraction", "[cli][wording]") {
    // Writing "23.976" here would be the lie the grammar refuses to tell in the
    // other direction: 24000/1001 has no terminating decimal, so naming it by
    // one would report a conversion that did not happen.
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps23976}) == "24000/1001");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps29970}) == "30000/1001");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps59940}) == "60000/1001");
}

TEST_CASE("a rate below one frame per second is named all the same", "[cli][wording]") {
    CHECK(nameOf(rateOf(1, 8)) == "0.125");
}

TEST_CASE("every kind of command has a name of its own", "[wording]") {
    // Deux noms identiques rendraient une action d'annulation ambiguë, et un
    // nom vide la rendrait muette. Le compilateur tient l'exhaustivité du
    // `switch` ; ce test tient ce qu'il ne peut pas voir.
    constexpr std::array kEveryKind = {
        subedit::core::CommandKind::SetText,
        subedit::core::CommandKind::SetStart,
        subedit::core::CommandKind::SetEnd,
        subedit::core::CommandKind::Insert,
        subedit::core::CommandKind::Remove,
        subedit::core::CommandKind::Shift,
        subedit::core::CommandKind::Transform,
        subedit::core::CommandKind::ConvertFrameRate,
        subedit::core::CommandKind::Snap,
        subedit::core::CommandKind::Sort,
        subedit::core::CommandKind::RemoveHearingImpaired,
    };

    std::set<std::string_view> seen;
    for (const subedit::core::CommandKind kind : kEveryKind) {
        CHECK_FALSE(nameOf(kind).empty());
        seen.insert(nameOf(kind));
    }

    CHECK(seen.size() == kEveryKind.size());
}

TEST_CASE("a length is written in signed seconds, to the millisecond", "[wording]") {
    using subedit::core::Duration;
    using subedit::core::secondsOf;

    CHECK(secondsOf(Duration::fromMilliseconds(2999)) == "2.999 s");
    CHECK(secondsOf(Duration::zero()) == "0.000 s");
    CHECK(secondsOf(Duration::fromMilliseconds(-7001)) == "-7.001 s");
    // A length between −1 s and 0 has a whole part of zero, which carries no
    // sign of its own. It was already right in the command line, and moving it
    // here is what keeps it right in both places.
    CHECK(secondsOf(Duration::fromMilliseconds(-500)) == "-0.500 s");
}

// The three formulations issue #174 asks for. What differs between them is the
// operation named — the two numbers are the same two in all three, and writing
// the sentence three times would have been three copies of one rule.
TEST_CASE("each operation says in its own name what it left past the end", "[wording]") {
    using subedit::core::BeyondEnd;
    using subedit::core::CommandKind;
    using subedit::core::Duration;
    using subedit::core::noticeOf;

    const BeyondEnd beyond{.count = 3, .overshoot = Duration::fromMilliseconds(4200)};

    CHECK(noticeOf(CommandKind::Shift, beyond) ==
          "shifting leaves 3 subtitles past the end of the video, by 4.200 s at most");
    CHECK(noticeOf(CommandKind::Transform, beyond) ==
          "transforming leaves 3 subtitles past the end of the video, by 4.200 s at most");
    CHECK(noticeOf(CommandKind::ConvertFrameRate, beyond) ==
          "converting the frame rate leaves 3 subtitles past the end of the video, "
          "by 4.200 s at most");
}

// One is the common case, and « the furthest by » would read wrong for it.
TEST_CASE("a single subtitle past the end is said in the singular", "[wording]") {
    using subedit::core::BeyondEnd;
    using subedit::core::CommandKind;
    using subedit::core::Duration;
    using subedit::core::noticeOf;

    CHECK(noticeOf(CommandKind::Shift,
                   BeyondEnd{.count = 1, .overshoot = Duration::fromMilliseconds(500)}) ==
          "shifting leaves 1 subtitle past the end of the video, by 0.500 s at most");
}

TEST_CASE("every grid verdict has a word of its own", "[wording]") {
    // The same three words the window shows in its status bar. Two surfaces
    // wording one verdict twice is how they start to disagree, which is why
    // this lives in `wording` rather than in the report that first needed it.
    using subedit::core::GridVerdict;

    CHECK(nameOf(GridVerdict::Clean) == "clean");
    CHECK(nameOf(GridVerdict::Partial) == "partial");
    CHECK(nameOf(GridVerdict::Silent) == "none");
}

TEST_CASE("a concentration is written with one decimal", "[wording]") {
    using subedit::core::percentOf;

    CHECK(percentOf(100.0) == "100.0%");
    CHECK(percentOf(99.87) == "99.9%");
    CHECK(percentOf(15.34) == "15.3%");
    // One decimal and not more: the third digit says nothing a reader can act
    // on, and a clean grid reads better as 99.9% than as 99.87342%.
    CHECK(percentOf(0.0) == "0.0%");
}

TEST_CASE("the status line names the film and the rate it declares", "[wording]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;
    using subedit::core::videoStatusOf;

    const std::filesystem::path film{"/films/le-canot.mkv"};

    CHECK(videoStatusOf(std::nullopt) == "No video");
    CHECK(videoStatusOf(film) == "Video: le-canot.mkv");
    // The rate goes with the film rather than beside it: they are one fact,
    // and a third widget would put it at the same rank as the grid deduced
    // from the positions, which is another fact entirely.
    CHECK(videoStatusOf(film, FrameRate{StandardFrameRate::Fps23976}) ==
          "Video: le-canot.mkv, 24000/1001 fps");
}
