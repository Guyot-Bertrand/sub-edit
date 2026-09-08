// MicroDVD — the only one of the nine that does not count in time.
//
// Its positions *are* frame numbers, and no MicroDVD file states the rate they
// were counted at. The rate comes from outside, travels with the document, and
// is what makes the round trip exact whatever it was — ADR 0030.

#include <subedit/core/format/micro_dvd_reader.hpp>
#include <subedit/core/format/micro_dvd_syntax.hpp>
#include <subedit/core/format/micro_dvd_writer.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <expected>
#include <string>
#include <string_view>
#include <variant>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::FrameRate;
using subedit::core::MicroDvdFile;
using subedit::core::MicroDvdReader;
using subedit::core::MicroDvdWriter;
using subedit::core::parseMicroDvdFrameLine;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadingChoices;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

constexpr FrameRate kPal{StandardFrameRate::Fps25};

[[nodiscard]] ReadResult read(std::string_view content, FrameRate rate = kPal, bool chosen = true) {
    const std::expected<ReadResult, ReadError> result = MicroDvdReader{rate, chosen}.read(content);
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

} // namespace

TEST_CASE("frame numbers become positions at the rate they were counted at", "[format][microdvd]") {
    const ReadResult result = read("{25}{75}Le vent se lève.\n{100}{150}Et retombe.\n");

    CHECK(result.format == SubtitleFormat::MicroDvd);
    REQUIRE(result.subtitles.size() == 2);
    // Forty milliseconds a frame, exactly, at twenty-five a second.
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[0].end.milliseconds() == 3000);
    CHECK(result.subtitles[1].start.milliseconds() == 4000);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");
}

TEST_CASE("the same file at another rate is another film", "[format][microdvd]") {
    // **What « conditional » means.** The bytes are the same, the reading
    // succeeds either way, and the two answers are minutes apart on a feature.
    const ReadResult pal = read("{25}{75}Une réplique.\n", kPal);
    const ReadResult ntsc = read("{25}{75}Une réplique.\n", FrameRate{StandardFrameRate::Fps23976});

    CHECK(pal.subtitles[0].start.milliseconds() == 1000);
    CHECK(ntsc.subtitles[0].start.milliseconds() == 1043);
}

TEST_CASE("the rate travels with the document", "[format][microdvd]") {
    // Without it, writing would have nothing to count the frames back with.
    const ReadResult result = read("{25}{75}Une réplique.\n");

    const auto* file = std::get_if<MicroDvdFile>(&result.extras);
    REQUIRE(file != nullptr);
    CHECK(file->rate == kPal);
}

TEST_CASE("a rate nobody chose is said, and one that was chosen is not", "[format][microdvd]") {
    // The single place where every position on screen rests on an assumption
    // the file cannot confirm — ADR 0008 asks for that to be said.
    const ReadResult assumed = read("{25}{75}Une réplique.\n", kPal, false);
    CHECK(hasDiagnostic(assumed, DiagnosticKind::AssumedFrameRate));

    const ReadResult chosen = read("{25}{75}Une réplique.\n", kPal, true);
    CHECK_FALSE(hasDiagnostic(chosen, DiagnosticKind::AssumedFrameRate));
}

TEST_CASE("the one header line this format has comes back", "[format][microdvd]") {
    const ReadResult result = read("{DEFAULT}{}{Sans,18}\n{25}{75}Une réplique.\n");

    CHECK(result.header == "{DEFAULT}{}{Sans,18}");
    REQUIRE(result.subtitles.size() == 1);
}

TEST_CASE("a pipe becomes a real line break", "[format][microdvd]") {
    const ReadResult result = read("{25}{75}La côte est loin|et le vent tombe.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "La côte est loin\net le vent tombe.");
}

TEST_CASE("a frame before the first is read with its sign", "[format][microdvd]") {
    const ReadResult result = read("{-25}{25}Avant le début.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == -1000);
}

TEST_CASE("a line that is not a subtitle is reported", "[format][microdvd]") {
    const ReadResult result = read("{25}{75}Une réplique.\nune ligne qui traîne\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::IgnoredLine));
}

TEST_CASE("braces that hold no number, or no second pair, are not a subtitle",
          "[format][microdvd]") {
    for (const std::string_view line : {"{}{75}Une.\n",
                                        "{25}Une.\n",
                                        "{25}{75Une.\n",
                                        "{25)[75]Une.\n",
                                        "{12345678901}{75}Une.\n"}) {
        INFO("ligne : " << line);
        CHECK_FALSE(MicroDvdReader{kPal, true}.read(line).has_value());
    }
}

TEST_CASE("a file with no braced line is refused", "[format][microdvd]") {
    const std::expected<ReadResult, ReadError> refused =
        MicroDvdReader{kPal, true}.read("du texte\net rien\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("writing counts the positions back at the rate the document carries",
          "[format][microdvd]") {
    const std::array<Subtitle, 1> subtitles = {Subtitle{
        .start = Timestamp::fromMilliseconds(1000),
        .end = Timestamp::fromMilliseconds(3000),
        .mainText = "Une\nsur deux lignes.",
    }};

    const std::string written = MicroDvdWriter{}.write(WriteRequest{
        .subtitles = subtitles,
        .extras = MicroDvdFile{.rate = kPal},
    });

    CHECK(written == "{25}{75}Une|sur deux lignes.\n");
}

TEST_CASE("a document that carries no rate is written at the declared default",
          "[format][microdvd]") {
    // A document from a time format has none. The command line refuses that
    // rather than choose — the library's own default is the last resort, and
    // it is the one ADR 0030 names.
    const std::array<Subtitle, 1> subtitles = {Subtitle{
        .start = Timestamp::fromMilliseconds(1000),
        .end = Timestamp::fromMilliseconds(3000),
        .mainText = "Une.",
    }};

    const std::string written = MicroDvdWriter{}.write(WriteRequest{.subtitles = subtitles});

    // A thousand milliseconds is twenty-four frames at 24000/1001.
    CHECK(written == "{24}{72}Une.\n");
}

TEST_CASE("a MicroDVD file comes back byte for byte, at any rate",
          "[format][microdvd][roundtrip]") {
    // **The two conversions cancel.** Whatever rate read the frames writes them
    // back, so the bytes return — and that is true of a rate nobody chose as
    // much as of the right one. It is why the promise is conditional and not
    // false: what depends on the rate is what the positions mean, not what the
    // file becomes.
    constexpr std::string_view kFile = "{DEFAULT}{}{Sans}\n"
                                       "{-25}{25}Avant le début|sur deux lignes.\n"
                                       "{1194}{1285}{Y:i}Une réplique en italique.\n";

    for (const StandardFrameRate standard :
         {StandardFrameRate::Fps23976, StandardFrameRate::Fps25, StandardFrameRate::Fps60}) {
        const std::expected<ReadResult, ReadError> read =
            readSubtitles(kFile, ReadingChoices{.frameRate = FrameRate{standard}});
        REQUIRE(read.has_value());
        CHECK(read->format == SubtitleFormat::MicroDvd);

        const std::expected<std::string, WriteError> written =
            writeSubtitles(read->format,
                           WriteRequest{
                               .subtitles = read->subtitles,
                               .newline = read->newline,
                               .header = read->header,
                               .extras = read->extras,
                           });
        REQUIRE(written.has_value());
        CHECK(*written == kFile);
    }
}

TEST_CASE("the header line is not taken for a subtitle", "[format][microdvd]") {
    // It opens on a brace like every other line, so it is recognised first.
    CHECK_FALSE(parseMicroDvdFrameLine("{DEFAULT}{}{Sans}").has_value());
    CHECK(subedit::core::isMicroDvdHeader("{DEFAULT}{}{Sans}"));
    CHECK_FALSE(subedit::core::isMicroDvdHeader("{25}{75}Une."));
}
