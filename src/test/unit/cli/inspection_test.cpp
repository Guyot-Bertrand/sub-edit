#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ExitCode;
using subedit::cli::inspectAll;
using subedit::cli::inspectFile;
using subedit::cli::Reporter;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::OrderReport;

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
                       "  order: in order\n");
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

TEST_CASE("the report names the line that breaks the order", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kOutOfOrder);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}));

    CHECK_THAT(out.str(), ContainsSubstring("  order: line 2 breaks the order\n"));
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

TEST_CASE("the report names lines that break the order", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoLateLines);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}, OrderReport::Breaks));

    // Counted from one, as the report shows them: the third subtitle of the
    // file is the one that starts before the one before it.
    CHECK_THAT(out.str(), ContainsSubstring("  order: line 3 breaks the order\n"));
}

TEST_CASE("the report names lines that start late", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoLateLines);
    std::ostringstream out;
    std::ostringstream errors;

    CHECK(inspectFile(files, "a.srt", out, Reporter{errors, 0}, OrderReport::Late));

    // The wording differs, and that is what tells the reader which reading they
    // are looking at: a bare list of indices would be ambiguous between the two.
    CHECK_THAT(out.str(), ContainsSubstring("  order: lines 3, 4 start late\n"));
}

TEST_CASE("naming what breaks the order is the default reading", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoLateLines);

    const auto report = [&files](auto&&... reading) {
        std::ostringstream out;
        std::ostringstream errors;
        static_cast<void>(inspectFile(files, "a.srt", out, Reporter{errors, 0}, reading...));
        return out.str();
    };

    CHECK(report() == report(OrderReport::Breaks));
}

TEST_CASE("both readings agree on a file that is in order", "[cli][inspection]") {
    const InMemoryFileSystem files = withFile("a.srt", kTwoSubtitles);

    const auto order = [&files](OrderReport reading) {
        std::ostringstream out;
        std::ostringstream errors;
        static_cast<void>(inspectFile(files, "a.srt", out, Reporter{errors, 0}, reading));
        return out.str();
    };

    CHECK_THAT(order(OrderReport::Breaks), ContainsSubstring("  order: in order\n"));
    CHECK_THAT(order(OrderReport::Late), ContainsSubstring("  order: in order\n"));
}
