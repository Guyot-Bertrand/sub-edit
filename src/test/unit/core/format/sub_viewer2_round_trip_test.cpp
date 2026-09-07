// A SubViewer 2 file read and written back is the same file.
//
// The property the phase rests on, and the one `docs/mesures/conversion.md`
// measures from the outside: what a reading kept, a writing puts back.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::SubtitleFormat;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// Reads then writes, asking for nothing that was not in the file.
[[nodiscard]] std::string roundTrip(std::string_view original) {
    const std::expected<ReadResult, ReadError> read = readSubtitles(original);
    if (!read.has_value()) {
        FAIL("the file was meant to open");
        return {};
    }

    const std::expected<std::string, WriteError> written =
        writeSubtitles(read->format,
                       WriteRequest{
                           .subtitles = read->subtitles,
                           .newline = read->newline,
                           .encoding = read->encoding,
                           .header = read->header,
                       });
    if (!written.has_value()) {
        FAIL("the writing was meant to succeed");
        return {};
    }
    return *written;
}

constexpr std::string_view kFile = "[INFORMATION]\n"
                                   "[TITLE]Le port\n"
                                   "[AUTHOR]\n"
                                   "[END INFORMATION]\n"
                                   "[SUBTITLE]\n"
                                   "[COLF]&HFFFFFF,[STYLE]bd,[SIZE]18,[FONT]Sans\n"
                                   "\n"
                                   "-00:00:06.84,-00:00:02.85\n"
                                   "Avant le début[br]du film.\n"
                                   "\n"
                                   "00:00:01.00,00:00:03.50\n"
                                   "<i>Le vent se lève.</i>\n";

} // namespace

TEST_CASE("a SubViewer 2 file comes back byte for byte", "[format][subviewer2][roundtrip]") {
    // Header, negative positions, a break marker and a tag, in one file: each
    // of them is something the reading had to keep rather than understand.
    CHECK(roundTrip(kFile) == kFile);
}

TEST_CASE("the file is detected as SubViewer 2 and not as something else",
          "[format][subviewer2][roundtrip]") {
    const std::expected<ReadResult, ReadError> read = readSubtitles(kFile);

    REQUIRE(read.has_value());
    CHECK(read->format == SubtitleFormat::SubViewer2);
}
