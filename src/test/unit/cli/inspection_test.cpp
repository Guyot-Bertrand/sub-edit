#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <grid_fixtures.hpp>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ExitCode;
using subedit::cli::inspectAll;
using subedit::cli::inspectFile;
using subedit::cli::Reporter;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kTwoSubtitles = "1\n"
                                  "00:00:01,000 --> 00:00:03,500\n"
                                  "First.\n"
                                  "\n"
                                  "2\n"
                                  "00:00:04,000 --> 00:00:06,200\n"
                                  "Second.\n";

// Two subtitles, the second starting before the first.
const std::string kOutOfOrder = "1\n"
                                "00:00:05,000 --> 00:00:07,000\n"
                                "Later, but written first.\n"
                                "\n"
                                "2\n"
                                "00:00:01,000 --> 00:00:03,000\n"
                                "Earlier.\n";

// Starts 0, 4000, 2000, 3000: the third breaks the order against the second,
// and the fourth follows the third while still landing before the 4000 already
// seen. The one case where the two readings part.
const std::string kTwoLateLines = "1\n00:00:00,000 --> 00:00:00,500\nFirst.\n"
                                  "\n2\n00:00:04,000 --> 00:00:04,500\nSecond.\n"
                                  "\n3\n00:00:02,000 --> 00:00:02,500\nThird.\n"
                                  "\n4\n00:00:03,000 --> 00:00:03,500\nFourth.\n";

InMemoryFileSystem withFile(const std::string& path, const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(path, content);
    return files;
}

} // namespace

TEST_CASE("the report says what the file is made of", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoSubtitles);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK(out.str() == "a.srt\n"
                       "  format: SubRip\n"
                       "  encoding: UTF-8\n"
                       "  byte order mark: absent\n"
                       "  line endings: LF\n"
                       "  subtitles: 2\n"
                       "  span: 00:00:01.000 -> 00:00:06.200\n"
                       // Two starts always look perfectly concentrated: the
                       // noise floor is one over the square root of the count.
                       // A verdict drawn from two points would be a coin toss
                       // wearing a number, so the report says which of the two
                       // silences this is.
                       "  frame rate grid: none (too few subtitles to tell)\n"
                       "  anomalies: none\n");
}

TEST_CASE("the span runs from the earliest start to the latest end", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kOutOfOrder);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    // Not "first to last": on a file whose order is broken the two differ, and
    // only this one says the truth about what the file covers.
    CHECK_THAT(out.str(), ContainsSubstring("  span: 00:00:01.000 -> 00:00:07.000\n"));
}

TEST_CASE("the report names the subtitle that breaks the order", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kOutOfOrder);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    // By subtitle number, not by line — ADR 0018. The overlap comes with it:
    // a subtitle that starts before the previous one started also starts before
    // it ended, and the two are fixed differently.
    CHECK_THAT(out.str(),
               ContainsSubstring("  anomalies: subtitle 2 starts before the previous one ends, "
                                 "subtitle 2 starts before the previous one starts\n"));
}

TEST_CASE("a byte order mark and Windows endings are seen", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.srt",
                 "\xEF\xBB\xBF"
                 "1\r\n00:00:01,000 --> 00:00:03,000\r\nOnly one.\r\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  byte order mark: present\n"));
    CHECK_THAT(out.str(), ContainsSubstring("  line endings: CRLF\n"));
}

TEST_CASE("mixed line endings are signalled with their line", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt",
                                              "1\r\n00:00:01,000 --> 00:00:03,000\r\nWindows.\r\n"
                                              "\r\n2\n00:00:04,000 --> 00:00:06,000\nUnix.\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("mixed from line 5"));
}

TEST_CASE("a file that is not there is named, and nothing is reported", "[cli][inspection]") {
    const InMemoryFileSystem files;
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "absent.srt", out, Reporter{errors, 0}));

    CHECK(out.str().empty());
    CHECK(errors.str() == "absent.srt: does not exist\n");
}

TEST_CASE("bytes that are not UTF-8 are refused rather than mangled", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", "1\n00:00:01,000 --> 00:00:03,000\n\xFF\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(errors.str(), ContainsSubstring("is not valid UTF-8"));
}

TEST_CASE("a file in no known format is refused", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", "nothing that any reader claims\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(errors.str(), ContainsSubstring("is in no format this tool knows"));
}

TEST_CASE("the narration deepens with the level", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoSubtitles);

    const auto narrate = [&files](int level) {
        std::ostringstream out;
        std::ostringstream errors;
        static_cast<void>(inspectFile(files, "a.srt", out, Reporter{errors, level}));
        return errors.str();
    };

    CHECK(narrate(1) == "a.srt: 2 subtitles\n");
    CHECK_THAT(narrate(2), ContainsSubstring("a.srt: SubRip, UTF-8, no BOM, LF line endings\n"));
    CHECK_THAT(narrate(3), ContainsSubstring("bytes read"));
    CHECK_THAT(narrate(3), ContainsSubstring("diagnostics while reading"));
}

TEST_CASE("a batch keeps going after a failure", "[cli][inspection]") {
    InMemoryFileSystem files;
    files.addFile("good.srt", kTwoSubtitles);
    std::ostringstream out;
    std::ostringstream errors;

    const ExitCode code = inspectAll(files, {"absent.srt", "good.srt"}, out, Reporter{errors, 1});

    CHECK(code == ExitCode::SomeFailed);
    CHECK_THAT(out.str(), ContainsSubstring("good.srt\n"));
    CHECK_THAT(errors.str(), ContainsSubstring("1 of 2 files inspected, 1 failed\n"));
}

TEST_CASE("a batch where nothing survives says so", "[cli][inspection]") {
    const InMemoryFileSystem files;
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectAll(files, {"a.srt", "b.srt"}, out, Reporter{errors, 1}) == ExitCode::AllFailed);
}

TEST_CASE("a file that cannot be opened is told from one that is absent", "[cli][inspection]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwoSubtitles);
    files.failNextRead(FileErrorKind::PermissionDenied);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    // Not "does not exist": the file is there, and telling the two apart is
    // what lets a caller know whether to look for a typo or for a chmod.
    CHECK(errors.str() == "a.srt: cannot be opened: permission denied\n");
}

TEST_CASE("a device that refuses for another reason says so", "[cli][inspection]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwoSubtitles);
    files.failNextRead(FileErrorKind::Io);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "a.srt", out, Reporter{errors, 0}));
    CHECK(errors.str() == "a.srt: cannot be read\n");
}

TEST_CASE("a recognised format holding no subtitle is refused", "[cli][inspection]") {
    // WebVTT claims the file on its signature, then finds nothing in it. That
    // is a different failure from "no format claimed it", and the message says
    // which one it was.
    const InMemoryFileSystem files = withFile("a.vtt", "WEBVTT\n\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK_FALSE(inspectFile(files, "a.vtt", out, Reporter{errors, 0}));
    CHECK_THAT(errors.str(), ContainsSubstring("holds nothing recognisable as a subtitle"));
}

TEST_CASE("WebVTT is named as such", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.vtt", "WEBVTT\n\n00:00:01.000 --> 00:00:03.000\nOnly one.\n");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.vtt", out, Reporter{errors, 0}));
    CHECK_THAT(out.str(), ContainsSubstring("  format: WebVTT\n"));
}

TEST_CASE("classic Mac line endings are named", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.srt", "1\r00:00:01,000 --> 00:00:03,000\rOnly one.\r");
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));
    CHECK_THAT(out.str(), ContainsSubstring("  line endings: CR\n"));
}

TEST_CASE("the report names every subtitle out of place", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoLateLines);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    // Counted from one, as the report shows them. **Only the third is named as
    // breaking the order** — the fourth follows the third, so there is nothing
    // to do about it. That is the reading kept, and `--order-report` offered
    // the other one until the corpus failed to settle the question.
    CHECK_THAT(out.str(), ContainsSubstring("subtitle 3 starts before the previous one starts"));
    CHECK_THAT(out.str(), !ContainsSubstring("subtitle 4 starts before the previous one starts"));
}

TEST_CASE("the report names the grid a file was written on", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", subedit::test::gridBytes("grille-24.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  frame rate grid: 24 fps, clean (99.9%)\n"));
}

TEST_CASE("the report names the harmonic it set aside", "[cli][inspection]") {
    // A grid at 25 is included in a grid at 50: the file fits both, the lower
    // rate is the parsimonious reading, and the other is said rather than hidden.
    const InMemoryFileSystem files = withFile("a.srt", subedit::test::gridBytes("grille-25.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  frame rate grid: 25 fps, clean (100.0%)\n"));
    CHECK_THAT(out.str(),
               ContainsSubstring("  also fits: 50 fps, of which this rate is a whole divisor\n"));
}

TEST_CASE("the report gives the offset of a shifted grid", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.srt", subedit::test::gridBytes("grille-24-decalee.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  grid offset: 0.041 s\n"));
}

TEST_CASE("the report says when the span is too short to choose", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.srt", subedit::test::gridBytes("grille-24-courte.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  too short a span to separate: 24000/1001 fps\n"));
}

TEST_CASE("the report counts the starts that left the grid", "[cli][inspection]") {
    // Two thirds on a grid, the last third retimed: a few long runs. A file
    // corrected by hand would show as many runs as strays, and the count is
    // what tells the two apart.
    const InMemoryFileSystem files =
        withFile("a.srt", subedit::test::gridBytes("melange-groupe.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  frame rate grid: 30000/1001 fps, partial ("));
    CHECK_THAT(out.str(), ContainsSubstring("  off the grid: 53 of 168 starts, in 4 runs\n"));
}

TEST_CASE("a file on no known grid names no rate at all", "[cli][inspection]") {
    const InMemoryFileSystem files =
        withFile("a.srt", subedit::test::gridBytes("grille-absurde.srt"));
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  frame rate grid: none (best candidate at 15.3%)\n"));
}
