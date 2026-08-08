#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/web_vtt_reader.hpp>
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
using subedit::core::Timestamp;
using subedit::core::WebVttExtras;
using subedit::core::WebVttReader;

ReadResult readOrFail(std::string_view content) {
    const WebVttReader reader;
    std::expected<ReadResult, ReadError> result = reader.read(content);
    if (!result.has_value()) {
        FAIL("la lecture a échoué alors qu'elle devait aboutir");
        return {};
    }
    return *std::move(result);
}

WebVttExtras extrasOf(const ReadResult& result, std::size_t index) {
    if (index >= result.subtitles.size())
        return {};
    const auto* extras = std::get_if<WebVttExtras>(&result.subtitles[index].extras);
    return extras == nullptr ? WebVttExtras{} : *extras;
}

bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

} // namespace

TEST_CASE("a minimal file gives its cue", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start == Timestamp::fromMilliseconds(1000));
    CHECK(result.subtitles[0].end == Timestamp::fromMilliseconds(2000));
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(result.header == "WEBVTT");
}

TEST_CASE("the header keeps whatever follows the signature", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT - Dialogue français\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    CHECK(result.header == "WEBVTT - Dialogue français");
}

TEST_CASE("hours may be written or left out", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:00:01.000 --> 00:00:02.000\n"
                                         "Avec heures.\n"
                                         "\n"
                                         "00:03.000 --> 00:04.000\n"
                                         "Sans heures.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start == Timestamp::fromMilliseconds(1000));
    CHECK(result.subtitles[1].start == Timestamp::fromMilliseconds(3000));
}

TEST_CASE("the cue identifier is the line above the timestamps", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "chapitre-1\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(extrasOf(result, 0).id == "chapitre-1");
}

TEST_CASE("the settings follow the timestamps on their line", "[format][webvtt]") {
    // Losing them loses the position of the cue on screen, which is the whole
    // reason the format-specific data exists.
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000 align:start position:10%\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(extrasOf(result, 0).settings == "align:start position:10%");
}

TEST_CASE("a STYLE block is bound to the cue that follows it", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "STYLE\n"
                                         "::cue { color: yellow }\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(extrasOf(result, 0).style == "STYLE\n::cue { color: yellow }");
}

TEST_CASE("a NOTE block is bound to the cue that follows it", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "NOTE traduction à revoir\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(extrasOf(result, 0).comment == "NOTE traduction à revoir");
}

TEST_CASE("style and comment go to the cue that follows, not the one before", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Premier.\n"
                                         "\n"
                                         "NOTE pour le second\n"
                                         "\n"
                                         "00:03.000 --> 00:04.000\n"
                                         "Second.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(extrasOf(result, 0).comment.empty());
    CHECK(extrasOf(result, 1).comment == "NOTE pour le second");
}

TEST_CASE("the tags of the format are left alone", "[format][webvtt]") {
    // ADR 0009: the text stays a raw string, tags included. Interpreting them
    // is the business of phase 4, and a reader that normalised them would make
    // the file come back changed.
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "<v Marie>Bonjour.</v>\n"
                                         "<c.jaune>En couleur.</c>\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "<v Marie>Bonjour.</v>\n<c.jaune>En couleur.</c>");
}

TEST_CASE("a cue keeps its several lines of text", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Première\n"
                                         "Deuxième\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Première\nDeuxième");
}

TEST_CASE("a cue ending before it starts is kept and reported", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:05.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::EndBeforeStart));
}

TEST_CASE("cues that overlap are reported", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:05.000\n"
                                         "Premier.\n"
                                         "\n"
                                         "00:03.000 --> 00:06.000\n"
                                         "Second.\n");

    CHECK(hasDiagnostic(result, DiagnosticKind::OverlappingSubtitles));
    CHECK_FALSE(hasDiagnostic(result, DiagnosticKind::OutOfOrder));
}

TEST_CASE("cues out of order are reported", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:05.000 --> 00:07.000\n"
                                         "Premier.\n"
                                         "\n"
                                         "00:01.000 --> 00:03.000\n"
                                         "Second.\n");

    CHECK(hasDiagnostic(result, DiagnosticKind::OutOfOrder));
}

TEST_CASE("a comment spanning several lines keeps them all", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "NOTE\n"
                                         "Deux lignes de remarque,\n"
                                         "et voici la seconde.\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(extrasOf(result, 0).comment == "NOTE\nDeux lignes de remarque,\net voici la seconde.");
}

TEST_CASE("a timestamp line that cannot be read is reported and kept as text", "[format][webvtt]") {
    // The same choice as the SubRip reader, for the same reason: destroying a
    // line we did not understand would be worse than showing it as it stands.
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n"
                                         "00:xx.000 --> 00:04.000\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::MalformedTimestamp));
    CHECK(result.subtitles[0].mainText == "Bonjour.\n00:xx.000 --> 00:04.000");
}

TEST_CASE("an end timestamp that cannot be read is reported too", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n"
                                         "00:03.000 --> pas une heure\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::MalformedTimestamp));
}

TEST_CASE("a block that is neither a cue nor a known keyword is reported", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\n"
                                         "\n"
                                         "REGION\n"
                                         "id:fond\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::UnknownBlock));
}

TEST_CASE("a file that does not begin with the signature is not WebVTT", "[format][webvtt]") {
    // Refusing beats guessing: the detection of phase 8 needs a reader that
    // says no, so that no format is ever assumed.
    const WebVttReader reader;

    const std::expected<ReadResult, ReadError> result =
        reader.read("1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("a signature without a single cue cannot be read", "[format][webvtt]") {
    const WebVttReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("WEBVTT\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("an empty file cannot be read", "[format][webvtt]") {
    const WebVttReader reader;

    const std::expected<ReadResult, ReadError> result = reader.read("");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("carriage returns do not end up in the text", "[format][webvtt]") {
    const ReadResult result = readOrFail("WEBVTT\r\n"
                                         "\r\n"
                                         "00:01.000 --> 00:02.000\r\n"
                                         "Bonjour.\r\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
}
