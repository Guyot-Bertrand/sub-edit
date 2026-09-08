// Reading SubViewer 2.0, the first of the seven formats phase 9 adds.
//
// What it brings that the first two did not: a header of bracketed sections
// that has to come back verbatim, positions written to the hundredth, and a
// text held on one line where `[br]` stands for the break.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_viewer2_reader.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::SubtitleFormat;
using subedit::core::SubViewer2Reader;

[[nodiscard]] ReadResult read(std::string_view content) {
    const std::expected<ReadResult, ReadError> result = SubViewer2Reader{}.read(content);
    if (!result.has_value()) {
        FAIL("the reading was meant to succeed");
        return {};
    }
    return *result;
}

[[nodiscard]] bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

constexpr std::string_view kHeader = "[INFORMATION]\n[TITLE]Le port\n[END INFORMATION]\n";

} // namespace

TEST_CASE("a SubViewer 2 file gives its subtitles and says what it is", "[format][subviewer2]") {
    const ReadResult result =
        read(std::string{kHeader} + "\n00:00:01.00,00:00:03.00\nLe vent se lève.\n"
                                    "\n00:00:04.50,00:00:06.25\nEt retombe.\n");

    CHECK(result.format == SubtitleFormat::SubViewer2);
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[0].end.milliseconds() == 3000);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");

    // The hundredth is the finest this format writes, and it lands on whole
    // tens of milliseconds — nothing rounds on the way in.
    CHECK(result.subtitles[1].start.milliseconds() == 4500);
    CHECK(result.subtitles[1].end.milliseconds() == 6250);
    CHECK(result.diagnostics.empty());
}

TEST_CASE("the bracketed header comes back as it was", "[format][subviewer2]") {
    // Verbatim, and joined without a trailing break: it is what the writer puts
    // back, and nothing in the library reads inside it.
    const ReadResult result =
        read(std::string{kHeader} + "\n00:00:01.00,00:00:03.00\nUne réplique.\n");

    CHECK(result.header == "[INFORMATION]\n[TITLE]Le port\n[END INFORMATION]");
}

TEST_CASE("a file with no header at all is read all the same", "[format][subviewer2]") {
    // The header is what the format promises about itself; the timestamp line
    // is the format being spoken. A file trimmed of its header still opens.
    const ReadResult result = read("00:00:01.00,00:00:03.00\nUne réplique.\n");

    CHECK(result.header.empty());
    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Une réplique.");
}

TEST_CASE("a break marker becomes a real line break", "[format][subviewer2]") {
    const ReadResult result =
        read("00:00:01.00,00:00:03.00\nLa côte est loin[br]et le vent tombe.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "La côte est loin\net le vent tombe.");
}

TEST_CASE("a position before the origin is read with its sign", "[format][subviewer2]") {
    // Real files carry them — Gaupol's own sample opens on two. The model has
    // allowed them since phase 1, and this is the first format to write them.
    const ReadResult result = read("-00:00:06.84,-00:00:02.85\nAvant le début.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == -6840);
    CHECK(result.subtitles[0].end.milliseconds() == -2850);
}

TEST_CASE("a line that fits nowhere is reported rather than swallowed", "[format][subviewer2]") {
    // Gaupol drops it without a word. Keeping it is not an option either —
    // there is no answer to which subtitle it would belong to — so it is
    // dropped and said, which is ADR 0008.
    const ReadResult result = read("00:00:01.00,00:00:03.00\nUne réplique.\n"
                                   "une ligne qui traîne\n"
                                   "00:00:04.00,00:00:06.00\nUne autre.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(hasDiagnostic(result, DiagnosticKind::IgnoredLine));
    REQUIRE_FALSE(result.diagnostics.empty());
    CHECK(result.diagnostics[0].detail == "une ligne qui traîne");
    CHECK(result.diagnostics[0].line == 3);
}

TEST_CASE("a timestamp line closing the file leaves a subtitle without text",
          "[format][subviewer2]") {
    // Gaupol raises an exception here — `lines[i+1]` on the last line. The
    // positions were written, so the subtitle exists; only its text is missing.
    const ReadResult result = read("00:00:01.00,00:00:03.00\nUne réplique.\n"
                                   "00:00:04.00,00:00:06.00\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[1].mainText.empty());
    CHECK(result.subtitles[1].end.milliseconds() == 6000);
}

TEST_CASE("a file without a single timestamp line is refused", "[format][subviewer2]") {
    const std::expected<ReadResult, ReadError> result =
        SubViewer2Reader{}.read(std::string{kHeader});

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a timestamp that is not of this format is not read as one", "[format][subviewer2]") {
    // `Timestamp::parse` is deliberately permissive — one or two digits, one to
    // three decimals, hours optional — and that permissiveness is what would
    // let a SubRip line pass for a SubViewer 2 one. The shape is checked before
    // the value is read.
    const std::expected<ReadResult, ReadError> refused =
        SubViewer2Reader{}.read("00:01,500,00:03,000\nUne réplique.\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a line shaped like a timestamp but holding no time is not one", "[format][subviewer2]") {
    // The shape and the value are two questions, and both are asked: `00:99:00`
    // has the digits in the right places and names a minute that does not
    // exist.
    const std::expected<ReadResult, ReadError> refused =
        SubViewer2Reader{}.read("00:99:00.00,00:00:03.00\nUne réplique.\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a line of the right length with the wrong punctuation is not a timestamp",
          "[format][subviewer2]") {
    // The shape is eleven characters with colons and a mark at three fixed
    // places. A line that has the length and not the punctuation is text that
    // happens to be eleven characters long, and nothing more.
    const std::expected<ReadResult, ReadError> refused =
        SubViewer2Reader{}.read("00:00:01x00,00:00:03.00\nUne réplique.\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}
