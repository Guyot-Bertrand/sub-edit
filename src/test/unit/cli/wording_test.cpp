#include <subedit/cli/wording.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <optional>

using subedit::cli::nameOf;
using subedit::core::FrameRate;
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
