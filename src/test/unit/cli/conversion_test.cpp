#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::convertAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::WriteShape;
using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::InMemoryFileSystem;
using subedit::core::Newline;
using subedit::core::SubtitleFormat;

namespace {

// In the shape the writer produces: a blank line closes every block, the last
// one included. That is what lets the round trip be compared byte for byte
// rather than approximately.
const std::string kSubRip = "1\n"
                            "00:00:01,000 --> 00:00:03,500\n"
                            "First.\n"
                            "\n"
                            "2\n"
                            "00:00:04,000 --> 00:00:06,200\n"
                            "Second.\n"
                            "\n";

/// Converts `content` and hands back what was written, or nothing when the run
/// failed.
struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run convert(const std::string& content,
            SubtitleFormat target,
            const WriteShape& shape,
            const std::string& outputDir = "out") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", content);

    std::ostringstream errors;
    const Destination destination = Destination::from("", outputDir, false, 1).value();
    const ExitCode code = convertAll(files,
                                     {"in/a.srt"},
                                     subedit::core::ReadingChoices{},
                                     target,
                                     shape,
                                     destination,
                                     Reporter{errors, 0});

    const std::string extension = target == SubtitleFormat::WebVtt ? ".vtt" : ".srt";
    return {.code = code,
            .written = files.contentOf(outputDir + "/a" + extension).value_or(""),
            .errors = errors.str()};
}

} // namespace

TEST_CASE("converting produces the format asked for", "[cli][conversion]") {
    const Run run = convert(kSubRip, SubtitleFormat::WebVtt, {});

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("WEBVTT"));
    // WebVTT leaves the hours field out below one hour, and the decimal mark
    // is its own.
    CHECK_THAT(run.written, ContainsSubstring("00:01.000 --> 00:03.500"));
}

TEST_CASE("the written file lands under the extension of its format", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", kSubRip);
    std::ostringstream errors;

    CHECK(convertAll(files,
                     {"in/a.srt"},
                     subedit::core::ReadingChoices{},
                     SubtitleFormat::WebVtt,
                     {},
                     Destination::from("", "out", false, 1).value(),
                     Reporter{errors, 0}) == ExitCode::Success);

    CHECK(files.contentOf("out/a.vtt").has_value());
    CHECK_FALSE(files.contentOf("out/a.srt").has_value());
}

TEST_CASE("line endings are kept as the source had them", "[cli][conversion]") {
    const std::string windows = "1\r\n00:00:01,000 --> 00:00:03,000\r\nOnly one.\r\n";

    CHECK_THAT(convert(windows, SubtitleFormat::SubRip, {}).written, ContainsSubstring("\r\n"));
}

TEST_CASE("line endings are imposed when asked", "[cli][conversion]") {
    const std::string windows = "1\r\n00:00:01,000 --> 00:00:03,000\r\nOnly one.\r\n";
    const Run run = convert(windows, SubtitleFormat::SubRip, {.newline = Newline::Lf});

    CHECK_FALSE(run.written.contains('\r'));
}

TEST_CASE("the three line endings can each be asked for", "[cli][conversion]") {
    CHECK_THAT(convert(kSubRip, SubtitleFormat::SubRip, {.newline = Newline::CrLf}).written,
               ContainsSubstring("\r\n"));

    const std::string mac =
        convert(kSubRip, SubtitleFormat::SubRip, {.newline = Newline::Cr}).written;
    CHECK(mac.contains('\r'));
    CHECK_FALSE(mac.contains('\n'));
}

TEST_CASE("a byte order mark is kept as the source had it", "[cli][conversion]") {
    const std::string marked = "\xEF\xBB\xBF"
                               "1\n00:00:01,000 --> 00:00:03,000\nOnly one.\n";

    CHECK_THAT(convert(marked, SubtitleFormat::SubRip, {}).written,
               ContainsSubstring("\xEF\xBB\xBF"));
    CHECK_FALSE(convert(kSubRip, SubtitleFormat::SubRip, {}).written.starts_with("\xEF\xBB\xBF"));
}

TEST_CASE("a byte order mark can be added and removed", "[cli][conversion]") {
    const std::string marked = "\xEF\xBB\xBF"
                               "1\n00:00:01,000 --> 00:00:03,000\nOnly one.\n";

    CHECK(convert(kSubRip, SubtitleFormat::SubRip, {.bom = ByteOrderMark::Present})
              .written.starts_with("\xEF\xBB\xBF"));
    CHECK_FALSE(convert(marked, SubtitleFormat::SubRip, {.bom = ByteOrderMark::Absent})
                    .written.starts_with("\xEF\xBB\xBF"));
}

TEST_CASE("the encoding asked for is the one written", "[cli][conversion]") {
    // Latin-1 in, UTF-8 out: the accented letter goes from one byte to two.
    const std::string latin = "1\n00:00:01,000 --> 00:00:03,000\nUn caf\xE9.\n";

    const Run run =
        convert(latin, SubtitleFormat::SubRip, {.encoding = Encoding::utf8(ByteOrderMark::Absent)});

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("caf\xC3\xA9"));
}

TEST_CASE("without a word, the encoding written is the one read", "[cli][conversion]") {
    // In the shape the writer produces — a blank line closes every block — so
    // that the comparison is byte for byte rather than approximate.
    const std::string latin = "1\n00:00:01,000 --> 00:00:03,000\nUn caf\xE9.\n\n";

    CHECK(convert(latin, SubtitleFormat::SubRip, {}).written == latin);
}

TEST_CASE("a character the encoding asked for cannot write stops the file", "[cli][conversion]") {
    // `ł` has no place in Latin-1, and a `?` written in its stead would be text
    // lost between reading and writing.
    const std::string polish = "1\n00:00:01,000 --> 00:00:03,000\nPrzyszedł późno.\n";

    const Run run =
        convert(polish,
                SubtitleFormat::SubRip,
                {.encoding = Encoding::create("iso-8859-1", ByteOrderMark::Absent).value()});

    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("holds a character the chosen encoding cannot write"));
}

TEST_CASE("a mark asked of an encoding that has none is refused", "[cli][conversion]") {
    // A byte order mark exists for the Unicode encodings and for no other.
    // Writing the file without the mark would answer a question that was asked.
    const std::string latin = "1\n00:00:01,000 --> 00:00:03,000\nUn caf\xE9.\n";

    const Run run = convert(latin, SubtitleFormat::SubRip, {.bom = ByteOrderMark::Present});

    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("ISO-8859-1 has no byte order mark to write"));
}

TEST_CASE("a round trip through the other format keeps the timings", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kSubRip);
    std::ostringstream errors;
    const Reporter quiet{errors, 0};

    CHECK(convertAll(files,
                     {"a.srt"},
                     subedit::core::ReadingChoices{},
                     SubtitleFormat::WebVtt,
                     {},
                     Destination::from("", "one", false, 1).value(),
                     quiet) == ExitCode::Success);
    CHECK(convertAll(files,
                     {"one/a.vtt"},
                     subedit::core::ReadingChoices{},
                     SubtitleFormat::SubRip,
                     {},
                     Destination::from("", "two", false, 1).value(),
                     quiet) == ExitCode::Success);

    // Byte for byte: what SubRip carries, WebVTT carries too. Only what a
    // format cannot hold would be lost, and neither of these holds anything the
    // other does not for a file this plain.
    CHECK(files.contentOf("two/a.srt") == kSubRip);
}

TEST_CASE("a file that cannot be read is named and the others go on", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("good.srt", kSubRip);
    std::ostringstream errors;

    const ExitCode code = convertAll(files,
                                     {"absent.srt", "good.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::WebVtt,
                                     {},
                                     Destination::from("", "out", false, 2).value(),
                                     Reporter{errors, 1});

    CHECK(code == ExitCode::SomeFailed);
    CHECK(files.contentOf("out/good.vtt").has_value());
    CHECK_THAT(errors.str(), ContainsSubstring("absent.srt"));
}

TEST_CASE("a write that fails is reported and counted", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kSubRip);
    files.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    const ExitCode code = convertAll(files,
                                     {"a.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::WebVtt,
                                     {},
                                     Destination::from("", "out", false, 1).value(),
                                     Reporter{errors, 0});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt"));
}

TEST_CASE("the narration says what was written and where", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kSubRip);
    std::ostringstream errors;

    static_cast<void>(convertAll(files,
                                 {"a.srt"},
                                 subedit::core::ReadingChoices{},
                                 SubtitleFormat::WebVtt,
                                 {},
                                 Destination::from("", "out", false, 1).value(),
                                 Reporter{errors, 1}));

    CHECK(errors.str() == "a.srt: 2 subtitles written as WebVTT -> out/a.vtt\n");
}

TEST_CASE("a file without its closing blank line gains one", "[cli][conversion]") {
    // Not a loss, and not a surprise either once it is written down: the SubRip
    // writer closes every block with a blank line, the last one included. A
    // file that arrived without it comes back one byte longer, and that is the
    // only difference.
    const std::string unclosed = "1\n00:00:01,000 --> 00:00:03,000\nOnly one.\n";
    const Run run = convert(unclosed, SubtitleFormat::SubRip, {});

    CHECK(run.written == unclosed + "\n");
}

TEST_CASE("writing the format a name already carries misnames nothing", "[cli][conversion]") {
    CHECK_FALSE(subedit::cli::wouldMisname({"a.srt", "b.srt"}, SubtitleFormat::SubRip));
    CHECK_FALSE(subedit::cli::wouldMisname({"a.vtt"}, SubtitleFormat::WebVtt));
}

TEST_CASE("writing another format over a name would misname it", "[cli][conversion]") {
    CHECK(subedit::cli::wouldMisname({"a.srt"}, SubtitleFormat::WebVtt));
    // One file is enough: the batch is refused whole rather than half done.
    CHECK(subedit::cli::wouldMisname({"a.vtt", "b.srt"}, SubtitleFormat::WebVtt));
}

TEST_CASE("the extension is read whatever its case", "[cli][conversion]") {
    CHECK_FALSE(subedit::cli::wouldMisname({"A.SRT"}, SubtitleFormat::SubRip));
}

TEST_CASE("a name without an extension would be misnamed", "[cli][conversion]") {
    CHECK(subedit::cli::wouldMisname({"soustitres"}, SubtitleFormat::SubRip));
}

TEST_CASE("a readable file in no known format is refused", "[cli][conversion]") {
    // Told apart from a file that is not there: one is a file system failure,
    // the other a reading one, and the messages differ.
    InMemoryFileSystem files;
    files.addFile("a.srt", "nothing any reader claims\n");
    std::ostringstream errors;

    const ExitCode code = convertAll(files,
                                     {"a.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::WebVtt,
                                     {},
                                     Destination::from("", "out", false, 1).value(),
                                     Reporter{errors, 0});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("is in no format this tool knows"));
}

namespace {

/// A file whose positions fall on a grid at twenty-five frames a second.
///
/// **Long enough for the deduction to answer.** Three subtitles fall on every
/// grid at once; the measurement of phase 16 says so rather than guess, and a
/// test that gave it three would be measuring its silence.
[[nodiscard]] std::string onAGridOf(int subtitles) {
    constexpr int kFrame = 40; // milliseconds, at twenty-five a second
    std::string text;
    for (int index = 1; index <= subtitles; ++index) {
        const int start = index * 13 * kFrame;
        const int end = start + (25 * kFrame);
        text += std::to_string(index) + "\n" +
                subedit::core::Timestamp::fromMilliseconds(start).format(
                    subedit::core::DecimalMark::Comma) +
                " --> " +
                subedit::core::Timestamp::fromMilliseconds(end).format(
                    subedit::core::DecimalMark::Comma) +
                "\nSubtitle " + std::to_string(index) + ".\n\n";
    }
    return text;
}

/// A file whose positions fall on nothing: a millisecond off, every time.
const std::string kOnNoGrid = "1\n"
                              "00:00:01,001 --> 00:00:03,003\n"
                              "First.\n"
                              "\n"
                              "2\n"
                              "00:00:04,007 --> 00:00:06,013\n"
                              "Second.\n"
                              "\n";

/// Converts into frames, saying a rate or leaving it to be worked out.
Run convertToFrames(const std::string& content,
                    const std::optional<subedit::core::FrameRate>& rate,
                    int verbosity = 0) {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", content);

    std::ostringstream errors;
    const Destination destination = Destination::from("", "out", false, 1).value();
    const ExitCode code = convertAll(files,
                                     {"in/a.srt"},
                                     subedit::core::ReadingChoices{.frameRate = rate},
                                     SubtitleFormat::MicroDvd,
                                     {},
                                     destination,
                                     Reporter{errors, verbosity});

    return {
        .code = code, .written = files.readFile("out/a.sub").value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("writing frames takes the rate that was given", "[cli][convert][frames]") {
    const Run run = convertToFrames(
        kOnNoGrid, subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25});

    CHECK(run.code == ExitCode::Success);
    CHECK(run.written == "{25}{75}First.\n{100}{150}Second.\n");
}

TEST_CASE("without a rate, writing frames takes the grid the positions fall on",
          "[cli][convert][frames]") {
    // **The one place the deduction of phase 16 decides rather than informs.**
    // The rate a time-based file was timed at *is* its grid, and taking it is
    // the only answer that does not move a single subtitle.
    const Run run = convertToFrames(onAGridOf(40), std::nullopt, 2);

    CHECK(run.code == ExitCode::Success);
    CHECK(run.written.starts_with("{13}{38}Subtitle 1.\n{26}{51}Subtitle 2.\n"));
    CHECK_THAT(run.errors, ContainsSubstring("counted in frames at 25"));
}

TEST_CASE("without a rate and without a grid, writing frames is refused",
          "[cli][convert][frames]") {
    // The only move left would be to invent a number, and every subtitle in the
    // file would move by it. Refusing names the option that settles it.
    const Run run = convertToFrames(kOnNoGrid, std::nullopt);

    CHECK(run.code == ExitCode::AllFailed);
    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("--frame-rate"));
}
