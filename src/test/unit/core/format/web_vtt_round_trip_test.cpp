#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/web_vtt_reader.hpp>
#include <subedit/core/format/web_vtt_writer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::WebVttReader;
using subedit::core::WebVttWriter;
using subedit::core::WriteRequest;

/// Reads then writes, which is what opening and saving a file amounts to.
std::string roundTrip(std::string_view content) {
    const WebVttReader reader;
    std::expected<ReadResult, ReadError> read = reader.read(content);
    if (!read.has_value()) {
        FAIL("la lecture a échoué alors qu'elle devait aboutir");
        return {};
    }

    const WebVttWriter writer;
    return writer.write(WriteRequest{.subtitles = read->subtitles, .header = read->header});
}

/// Everything the format can carry, in the shape the writer produces.
constexpr std::string_view kComplete = "WEBVTT - Dialogue\n"
                                       "\n"
                                       "STYLE\n"
                                       "::cue { color: yellow }\n"
                                       "\n"
                                       "NOTE traduction à revoir\n"
                                       "\n"
                                       "chapitre-1\n"
                                       "00:01.000 --> 00:02.500 align:start position:10%\n"
                                       "<v Marie>Bonjour à tous.</v>\n"
                                       "\n"
                                       "00:03.000 --> 00:04.000\n"
                                       "Deux lignes\n"
                                       "de texte.\n";

} // namespace

TEST_CASE("a complete file comes back byte for byte", "[format][webvtt][roundtrip]") {
    // Identifier, settings, STYLE and NOTE all survive — losing the settings
    // would lose the position of the cue on screen.
    CHECK(roundTrip(kComplete) == kComplete);
}

TEST_CASE("a second round trip changes nothing more", "[format][webvtt][roundtrip]") {
    const std::string once = roundTrip("WEBVTT\n"
                                       "\n"
                                       "00:00:01.000 --> 00:00:02.000\n"
                                       "Bonjour.\n");

    CHECK(roundTrip(once) == once);
}

TEST_CASE("a text with tags comes back untouched", "[format][webvtt][roundtrip]") {
    // ADR 0009 seen from the outside: what the user typed is what comes back.
    const std::string result = roundTrip("WEBVTT\n"
                                         "\n"
                                         "00:01.000 --> 00:02.000\n"
                                         "<v Marie>Bonjour</v> <c.jaune>ici</c>\n"
                                         "<ruby>漢<rt>かん</rt></ruby>\n");

    CHECK(result.contains("<v Marie>Bonjour</v> <c.jaune>ici</c>\n"
                          "<ruby>漢<rt>かん</rt></ruby>"));
}

TEST_CASE("a file written with hours is normalised to the shorter shape",
          "[format][webvtt][roundtrip]") {
    // The one normalisation of the format, and it is the one Gaupol makes: a
    // file where nothing reaches an hour is written without its hours field.
    const std::string result = roundTrip("WEBVTT\n"
                                         "\n"
                                         "00:00:01.000 --> 00:00:02.000\n"
                                         "Bonjour.\n");

    CHECK(result == "WEBVTT\n\n00:01.000 --> 00:02.000\nBonjour.\n");
}

TEST_CASE("a file that came with carriage returns is written back with line feeds",
          "[format][webvtt][roundtrip]") {
    const std::string result = roundTrip("WEBVTT\r\n"
                                         "\r\n"
                                         "00:01.000 --> 00:02.000\r\n"
                                         "Bonjour.\r\n");

    CHECK(result == "WEBVTT\n\n00:01.000 --> 00:02.000\nBonjour.\n");
}
