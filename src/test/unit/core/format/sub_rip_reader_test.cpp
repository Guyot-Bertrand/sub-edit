#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_rip_reader.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <string_view>
#include <variant>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::Severity;
using subedit::core::SubRipExtras;
using subedit::core::SubRipReader;
using subedit::core::Timestamp;

ReadResult readOrFail(std::string_view content) {
    const SubRipReader reader;
    std::expected<ReadResult, ReadError> result = reader.read(content);
    if (!result.has_value()) {
        FAIL("la lecture a échoué alors qu'elle devait aboutir");
        return {};
    }
    return *std::move(result);
}

bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

constexpr std::string_view kTwoSubtitles = "1\n"
                                           "00:00:01,000 --> 00:00:02,500\n"
                                           "Bonjour.\n"
                                           "\n"
                                           "2\n"
                                           "00:00:03,000 --> 00:00:04,000\n"
                                           "Au revoir.\n";

} // namespace

TEST_CASE("a well-formed file gives its subtitles", "[format][subrip]") {
    const ReadResult result = readOrFail(kTwoSubtitles);

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start == Timestamp::fromMilliseconds(1000));
    CHECK(result.subtitles[0].end == Timestamp::fromMilliseconds(2500));
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(result.subtitles[1].mainText == "Au revoir.");
    CHECK(result.diagnostics.empty());
}

TEST_CASE("a subtitle keeps its several lines of text", "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Première ligne\n"
                                         "Deuxième ligne\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Première ligne\nDeuxième ligne");
}

TEST_CASE("a blank line inside a subtitle does not end it early", "[format][subrip]") {
    // Blank lines separate subtitles, but what makes a subtitle start is a
    // timestamp line — not the blank line before it.
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Avant\n"
                                         "\n"
                                         "Après\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Avant\n\nAprès");
}

TEST_CASE("carriage returns do not end up in the text", "[format][subrip]") {
    const ReadResult result = readOrFail("1\r\n"
                                         "00:00:01,000 --> 00:00:02,000\r\n"
                                         "Bonjour.\r\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
}

TEST_CASE("timestamps written without their padding are accepted", "[format][subrip]") {
    // Gaupol accepts them, and files in the wild contain them.
    const ReadResult result = readOrFail("1\n"
                                         "0:0:1,5 --> 0:0:2,25\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start == Timestamp::fromMilliseconds(1500));
    CHECK(result.subtitles[0].end == Timestamp::fromMilliseconds(2250));
}

TEST_CASE("the extended coordinates are kept", "[format][subrip]") {
    const ReadResult result =
        readOrFail("1\n"
                   "00:00:01,000 --> 00:00:02,000  X1:040 X2:600 Y1:020 Y2:460\n"
                   "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    REQUIRE(std::holds_alternative<SubRipExtras>(result.subtitles[0].extras));
    const SubRipExtras extras = std::get<SubRipExtras>(result.subtitles[0].extras);
    if (!extras.coordinates.has_value()) {
        FAIL("les coordonnées étendues n'ont pas été lues");
        return;
    }

    CHECK(*extras.coordinates ==
          subedit::core::Rectangle{.x1 = 40, .x2 = 600, .y1 = 20, .y2 = 460});
}

TEST_CASE("a subtitle without coordinates carries none", "[format][subrip]") {
    const ReadResult result = readOrFail(kTwoSubtitles);

    REQUIRE(result.subtitles.size() == 2);
    REQUIRE(std::holds_alternative<SubRipExtras>(result.subtitles[0].extras));
    CHECK_FALSE(std::get<SubRipExtras>(result.subtitles[0].extras).coordinates.has_value());
}

TEST_CASE("text before any timestamp is reported, not fatal", "[format][subrip]") {
    // The pitfall found in Gaupol: its reader appends every non-timestamp line
    // to the previous subtitle, and raises an undocumented exception when
    // there is no previous subtitle. Ours says so and looks for the next
    // block.
    const ReadResult result = readOrFail("Une note de traduction\n"
                                         "\n"
                                         "1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(hasDiagnostic(result, DiagnosticKind::TextBeforeAnyTimestamp));
}

TEST_CASE("a missing number is recovered from", "[format][subrip]") {
    const ReadResult result = readOrFail("00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::MissingNumbering));
    CHECK(result.diagnostics.front().severity == Severity::Recovered);
}

TEST_CASE("numbers that do not follow are recovered from", "[format][subrip]") {
    // Regenerated on writing, so the file heals itself; the reading says so
    // rather than passing over it.
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n"
                                         "\n"
                                         "7\n"
                                         "00:00:03,000 --> 00:00:04,000\n"
                                         "Au revoir.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(hasDiagnostic(result, DiagnosticKind::InconsistentNumbering));
}

TEST_CASE("a subtitle ending before it starts is kept and reported", "[format][subrip]") {
    // ADR 0008: the user has to see it to fix it.
    const ReadResult result = readOrFail("1\n"
                                         "00:00:05,000 --> 00:00:02,000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].duration().milliseconds() == -3000);
    CHECK(hasDiagnostic(result, DiagnosticKind::EndBeforeStart));
}

TEST_CASE("subtitles that overlap are reported", "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:05,000\n"
                                         "Bonjour.\n"
                                         "\n"
                                         "2\n"
                                         "00:00:03,000 --> 00:00:06,000\n"
                                         "Au revoir.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(hasDiagnostic(result, DiagnosticKind::OverlappingSubtitles));
}

TEST_CASE("a timestamp line that cannot be read is reported and kept as text", "[format][subrip]") {
    // Destroying a line we failed to understand would be worse than keeping
    // it: the user can still see what was there.
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n"
                                         "00:00:xx,000 --> 00:00:99,000\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::MalformedTimestamp));
    CHECK(result.subtitles[0].mainText == "Bonjour.\n00:00:xx,000 --> 00:00:99,000");
}

TEST_CASE("a diagnostic points at the line it came from", "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "00:00:05,000 --> 00:00:02,000\n"
                                         "Bonjour.\n");

    REQUIRE_FALSE(result.diagnostics.empty());
    CHECK(result.diagnostics.front().line == 2);
}

TEST_CASE("a second subtitle without its number is recovered from", "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n"
                                         "\n"
                                         "00:00:03,000 --> 00:00:04,000\n"
                                         "Au revoir.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(result.subtitles[1].mainText == "Au revoir.");
    CHECK(hasDiagnostic(result, DiagnosticKind::MissingNumbering));
}

TEST_CASE("a blank line between the number and the timestamps is stepped over",
          "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(result.diagnostics.empty());
}

TEST_CASE("a number too long to be one is not taken for one", "[format][subrip]") {
    // Ten digits is not a subtitle number, it is a line of text that happens
    // to be all digits — a telephone number, a reference. It stays in the text
    // of the subtitle it belongs to, and the one that follows is reported as
    // having no number.
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "0123456789\n"
                                         "00:00:03,000 --> 00:00:04,000\n"
                                         "Au revoir.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].mainText == "0123456789");
    CHECK(hasDiagnostic(result, DiagnosticKind::MissingNumbering));
}

TEST_CASE("an end timestamp that cannot be read is reported", "[format][subrip]") {
    const ReadResult result = readOrFail("1\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n"
                                         "00:00:03,000 --> pas une heure\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::MalformedTimestamp));
}

TEST_CASE("coordinates that do not read as coordinates disqualify the line", "[format][subrip]") {
    // Half-written coordinates make the whole line suspect: keeping the
    // timestamps and dropping the rest would be a decision we have no ground
    // to take.
    const ReadResult first =
        readOrFail("1\n"
                   "00:00:01,000 --> 00:00:02,000\n"
                   "Bonjour.\n"
                   "00:00:03,000 --> 00:00:04,000  Z1:040 X2:600 Y1:020 Y2:460\n");
    CHECK(hasDiagnostic(first, DiagnosticKind::MalformedTimestamp));

    const ReadResult second =
        readOrFail("1\n"
                   "00:00:01,000 --> 00:00:02,000\n"
                   "Bonjour.\n"
                   "00:00:03,000 --> 00:00:04,000  X1: X2:600 Y1:020 Y2:460\n");
    CHECK(hasDiagnostic(second, DiagnosticKind::MalformedTimestamp));

    const ReadResult third =
        readOrFail("1\n"
                   "00:00:01,000 --> 00:00:02,000\n"
                   "Bonjour.\n"
                   "00:00:03,000 --> 00:00:04,000  X1:040 X2:600 Y1:020 Y2:460 et le reste\n");
    CHECK(hasDiagnostic(third, DiagnosticKind::MalformedTimestamp));
}

TEST_CASE("an empty file cannot be read at all", "[format][subrip]") {
    const SubRipReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a file without a single timestamp cannot be read at all", "[format][subrip]") {
    const SubRipReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("des notes\nsans horodatage\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}
