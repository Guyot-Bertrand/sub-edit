#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::convertAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::WriteShape;
using subedit::core::InMemoryFileSystem;
using subedit::core::Newline;
using subedit::core::SubtitleFormat;
using subedit::core::Utf8Bom;

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
            WriteShape shape,
            const std::string& outputDir = "out") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", content);

    std::ostringstream errors;
    const Destination destination = Destination::from("", outputDir, false, 1).value();
    const ExitCode code =
        convertAll(files, {"in/a.srt"}, target, shape, destination, Reporter{errors, 0});

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

    CHECK(convert(kSubRip, SubtitleFormat::SubRip, {.bom = Utf8Bom::Present})
              .written.starts_with("\xEF\xBB\xBF"));
    CHECK_FALSE(convert(marked, SubtitleFormat::SubRip, {.bom = Utf8Bom::Absent})
                    .written.starts_with("\xEF\xBB\xBF"));
}

TEST_CASE("a round trip through the other format keeps the timings", "[cli][conversion]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kSubRip);
    std::ostringstream errors;
    const Reporter quiet{errors, 0};

    CHECK(convertAll(files,
                     {"a.srt"},
                     SubtitleFormat::WebVtt,
                     {},
                     Destination::from("", "one", false, 1).value(),
                     quiet) == ExitCode::Success);
    CHECK(convertAll(files,
                     {"one/a.vtt"},
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
                                     SubtitleFormat::WebVtt,
                                     {},
                                     Destination::from("", "out", false, 1).value(),
                                     Reporter{errors, 0});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("is in no format this tool knows"));
}
