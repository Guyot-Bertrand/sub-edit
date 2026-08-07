#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_rip_reader.hpp>
#include <subedit/core/format/sub_rip_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/document.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::SubRipReader;
using subedit::core::SubRipWriter;
using subedit::core::WriteRequest;

/// Reads then writes, which is what opening and saving a file amounts to.
std::string roundTrip(std::string_view content) {
    const SubRipReader reader;
    std::expected<ReadResult, ReadError> read = reader.read(content);
    if (!read.has_value()) {
        FAIL("la lecture a échoué alors qu'elle devait aboutir");
        return {};
    }

    const SubRipWriter writer;
    return writer.write(WriteRequest{.subtitles = read->subtitles});
}

/// A file already in the shape the writer produces: numbering from one, a
/// blank line closing every block.
constexpr std::string_view kCanonical =
    "1\n"
    "00:00:01,000 --> 00:00:02,500\n"
    "Bonjour à tous.\n"
    "\n"
    "2\n"
    "00:00:03,000 --> 00:00:04,000\n"
    "Deux lignes\n"
    "de texte.\n"
    "\n"
    "3\n"
    "00:00:05,000 --> 00:00:06,000  X1:040 X2:600 Y1:020 Y2:460\n"
    "Placé à l'écran.\n"
    "\n";

} // namespace

TEST_CASE("a canonical file comes back byte for byte", "[format][subrip][roundtrip]") {
    CHECK(roundTrip(kCanonical) == kCanonical);
}

TEST_CASE("the extended coordinates survive the round trip", "[format][subrip][roundtrip]") {
    CHECK(roundTrip(kCanonical).contains("  X1:040 X2:600 Y1:020 Y2:460"));
}

TEST_CASE("a second round trip changes nothing more", "[format][subrip][roundtrip]") {
    // What matters for a file that was not canonical to begin with: the first
    // save normalises it, and no later save touches it again.
    const std::string once = roundTrip("7\n"
                                       "00:00:01,000 --> 00:00:02,000\n"
                                       "Bonjour.\n");

    CHECK(roundTrip(once) == once);
}

TEST_CASE("the numbering is the one thing that does not come back", "[format][subrip][roundtrip]") {
    // The documented exception: numbering is regenerated, which is how a file
    // whose numbers do not follow heals itself.
    const std::string result = roundTrip("7\n"
                                         "00:00:01,000 --> 00:00:02,000\n"
                                         "Bonjour.\n"
                                         "\n"
                                         "9\n"
                                         "00:00:03,000 --> 00:00:04,000\n"
                                         "Au revoir.\n"
                                         "\n");

    CHECK(result == "1\n"
                    "00:00:01,000 --> 00:00:02,000\n"
                    "Bonjour.\n"
                    "\n"
                    "2\n"
                    "00:00:03,000 --> 00:00:04,000\n"
                    "Au revoir.\n"
                    "\n");
}

TEST_CASE("a file that came with carriage returns is written back with line feeds",
          "[format][subrip][roundtrip]") {
    // The roadmap asks for Unix endings on writing. The ending a file arrived
    // with is not lost — it is kept in SourceFile — but it is the caller who
    // decides to put it back, not the writer that does it behind their back.
    const std::string result = roundTrip("1\r\n"
                                         "00:00:01,000 --> 00:00:02,000\r\n"
                                         "Bonjour.\r\n"
                                         "\r\n");

    CHECK(result == "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n");
}
