// What `inspect` says as a record: the same facts as its report, as keys.
//
// There is no JSON reader in the C++, and none is wanted: these cases look for
// the fragment of the record that carries each fact. The whole line, byte for
// byte, is held by the expected files of the end-to-end tests.

#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <grid_fixtures.hpp>
#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::inspectFile;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;

namespace {

/// The record `inspect` writes for `content`, and nothing else was written.
std::string recordOf(const std::string& content,
                     const subedit::core::ReadingChoices& reading = {}) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream text;
    std::ostringstream errors;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("inspect");

    REQUIRE(inspectFile(files, "a.srt", reading, text, reporter));
    // The text report and the record never both go out.
    CHECK(text.str().empty());
    return records.str();
}

const std::string kTwoSubtitles = "1\n00:00:01,000 --> 00:00:03,500\nFirst.\n\n"
                                  "2\n00:00:04,000 --> 00:00:06,200\nSecond.\n";

} // namespace

TEST_CASE("a record opens with the envelope, and is one line", "[cli][inspection][json]") {
    const std::string record = recordOf(kTwoSubtitles);

    CHECK(record.starts_with(
        R"({"schema":1,"command":"inspect","file":"a.srt","ok":true,"format":"srt",)"));
    CHECK(record.ends_with("}\n"));
    CHECK(std::count(record.begin(), record.end(), '\n') == 1);
    CHECK_THAT(record, ContainsSubstring(R"("subtitles":2,"span_ms":{"start":1000,"end":6200})"));
    CHECK_THAT(record, ContainsSubstring(R"("anomalies":[],"warnings":[]})"));
}

TEST_CASE("the encoding says where it came from", "[cli][inspection][json]") {
    CHECK_THAT(
        recordOf(kTwoSubtitles),
        ContainsSubstring(
            R"("encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false})"));
    CHECK_THAT(recordOf("\xEF\xBB\xBF"
                        "1\r\n00:00:01,000 --> 00:00:03,000\r\nOnly one.\r\n"),
               ContainsSubstring(R"("origin":"byte-order-mark","byte_order_mark":true)"));
    CHECK_THAT(recordOf(kTwoSubtitles,
                        subedit::core::ReadingChoices{.encoding = subedit::core::Encoding::utf8(
                                                          subedit::core::ByteOrderMark::Absent)}),
               ContainsSubstring(R"("origin":"asked")"));
}

TEST_CASE("the line endings are named, and the first mixed line is given",
          "[cli][inspection][json]") {
    CHECK_THAT(recordOf("1\r\n00:00:01,000 --> 00:00:03,000\r\nOnly one.\r\n"),
               ContainsSubstring(R"("line_endings":{"kind":"crlf","mixed_from_line":null})"));
    CHECK_THAT(recordOf("1\r\n00:00:01,000 --> 00:00:03,000\r\nWindows.\r\n"
                        "\r\n2\n00:00:04,000 --> 00:00:06,000\nUnix.\n"),
               ContainsSubstring(R"("line_endings":{"kind":"crlf","mixed_from_line":5})"));
    CHECK_THAT(recordOf("1\r00:00:01,000 --> 00:00:03,000\rOnly one.\r"),
               ContainsSubstring(R"("kind":"cr")"));
}

TEST_CASE("an anomaly is named by its subtitle", "[cli][inspection][json]") {
    // The second starts before the first.
    CHECK_THAT(recordOf("1\n00:00:05,000 --> 00:00:07,000\nLater.\n\n"
                        "2\n00:00:01,000 --> 00:00:03,000\nEarlier.\n"),
               ContainsSubstring(R"({"subtitle":2,"kind":"out-of-order"})"));
    // A subtitle that ends before it starts.
    CHECK_THAT(recordOf("1\n00:00:05,000 --> 00:00:04,000\nBackwards.\n"),
               ContainsSubstring(R"({"subtitle":1,"kind":"end-before-start"})"));
    // The second starts while the first is still on screen.
    CHECK_THAT(recordOf("1\n00:00:01,000 --> 00:00:05,000\nLong.\n\n"
                        "2\n00:00:02,000 --> 00:00:03,000\nInside.\n"),
               ContainsSubstring(R"({"subtitle":2,"kind":"overlapping-subtitles"})"));
}

TEST_CASE("a grid is described by its rate as a string and its concentration in thousandths",
          "[cli][inspection][json]") {
    const std::string clean = recordOf(subedit::test::gridBytes("grille-24.srt"));
    CHECK_THAT(
        clean,
        ContainsSubstring(R"("grid":{"verdict":"clean","enough_starts":true,"rate":"24",)"
                          R"("concentration_permille":999,"offset_ms":0,"also_fits":null,)"));
    CHECK_THAT(clean, ContainsSubstring(R"("frame_rate":null)"));

    // A grid at 25 is included in a grid at 50: the lower is retained, the other is said.
    CHECK_THAT(
        recordOf(subedit::test::gridBytes("grille-25.srt")),
        ContainsSubstring(R"("concentration_permille":1000,"offset_ms":0,"also_fits":"50")"));

    CHECK_THAT(recordOf(subedit::test::gridBytes("grille-24-decalee.srt")),
               ContainsSubstring(R"("offset_ms":41)"));
    CHECK_THAT(recordOf(subedit::test::gridBytes("grille-24-courte.srt")),
               ContainsSubstring(R"("not_separated":["24000/1001"])"));
}

TEST_CASE("starts off the grid are counted", "[cli][inspection][json]") {
    const std::string record = recordOf(subedit::test::gridBytes("melange-groupe.srt"));

    CHECK_THAT(
        record,
        ContainsSubstring(R"("verdict":"partial","enough_starts":true,"rate":"30000/1001")"));
    CHECK_THAT(record, ContainsSubstring(R"("strays":53,"starts":168})"));
}

TEST_CASE("a file on no known grid names no rate, and says how close the best was",
          "[cli][inspection][json]") {
    CHECK_THAT(recordOf(subedit::test::gridBytes("grille-absurde.srt")),
               ContainsSubstring(R"("verdict":"silent","enough_starts":true,"rate":null,)"
                                 R"("concentration_permille":153,"offset_ms":null,)"));
}

TEST_CASE("too few starts say so, and nothing else about the grid", "[cli][inspection][json]") {
    CHECK_THAT(
        recordOf(kTwoSubtitles),
        ContainsSubstring(R"("grid":{"verdict":"silent","enough_starts":false,"rate":null,)"
                          R"("concentration_permille":null,"offset_ms":null,)"
                          R"("also_fits":null,"not_separated":[],"strays":null,"starts":2})"));
}

TEST_CASE("a file counted in frames has a rate and no grid", "[cli][inspection][json]") {
    const std::string frames =
        "{25}{75}First.\n{100}{150}Second.\n{200}{260}Third.\n{300}{360}Fourth.\n";

    const std::string assumed = recordOf(frames);
    CHECK_THAT(
        assumed,
        ContainsSubstring(R"("frame_rate":{"rate":"24000/1001","origin":"assumed"},"grid":null)"));
    CHECK_THAT(assumed, ContainsSubstring(R"("kind":"assumed-frame-rate")"));

    const std::string asked =
        recordOf(frames,
                 subedit::core::ReadingChoices{.frameRate = subedit::core::FrameRate{
                                                   subedit::core::StandardFrameRate::Fps25}});
    CHECK_THAT(asked, ContainsSubstring(R"("frame_rate":{"rate":"25","origin":"asked"})"));
    CHECK_THAT(asked, ContainsSubstring(R"("span_ms":{"start":1000,"end":14400})"));
}

TEST_CASE("what the reading decided is a warning, at level zero too", "[cli][inspection][json]") {
    // Two blocks without a number: the reader numbered them, and says so.
    const std::string record = recordOf("00:00:01,000 --> 00:00:01,500\nUn\n\n"
                                        "00:00:02,000 --> 00:00:02,500\nDeux\n\n");

    CHECK_THAT(
        record,
        ContainsSubstring(R"("warnings":[{"kind":"missing-numbering","line":1,"settled":true},)"
                          R"({"kind":"missing-numbering","line":4,"settled":true}])"));
}

TEST_CASE("a warning about the whole file has no line, and a detail is carried",
          "[cli][inspection][json]") {
    // Latin-1 bytes, no mark: the encoding was weighed, which speaks of the whole file.
    const std::string record = recordOf(
        "1\n00:00:01,000 --> 00:00:03,000\nCaf\xE9 \xE0 c\xF4t\xE9, d\xE9j\xE0 vu.\n\n"
        "2\n00:00:04,000 --> 00:00:06,000\nL'\xE9t\xE9 d\xE9j\xE0 fini, \xE0 bient\xF4t.\n");

    CHECK_THAT(record, ContainsSubstring(R"({"kind":"guessed-encoding")"));
    CHECK_THAT(record, !ContainsSubstring(R"("kind":"guessed-encoding","line")"));
}

TEST_CASE("a file that cannot be read is a record of failure, with a stable kind",
          "[cli][inspection][json]") {
    const auto failureOf = [](const std::string& content) {
        InMemoryFileSystem files;
        if (!content.empty())
            files.addFile("a.srt", content);
        std::ostringstream text;
        std::ostringstream errors;
        std::ostringstream records;
        const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("inspect");

        CHECK_FALSE(inspectFile(files, "a.srt", {}, text, reporter));
        // The line on standard error is the text's, and it is said at level zero.
        CHECK_THAT(errors.str(), ContainsSubstring("a.srt: "));
        return records.str();
    };

    CHECK(failureOf("") ==
          "{\"schema\":1,\"command\":\"inspect\",\"file\":\"a.srt\",\"ok\":false,"
          "\"error\":{\"kind\":\"not-found\",\"message\":\"a.srt: does not exist\"}}\n");
    CHECK_THAT(failureOf("nothing any reader claims\n"),
               ContainsSubstring(R"("kind":"unknown-format")"));
    CHECK_THAT(failureOf("\xFF\xFE"), ContainsSubstring(R"("ok":false)"));
}

TEST_CASE("a path that is not UTF-8 is cleaned and the record says so", "[cli][inspection][json]") {
    const InMemoryFileSystem files;
    std::ostringstream text;
    std::ostringstream errors;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("inspect");

    CHECK_FALSE(inspectFile(files, "caf\xE9.srt", {}, text, reporter));

    CHECK_THAT(records.str(), ContainsSubstring("\"file\":\"caf\xEF\xBF\xBD.srt\""));
    CHECK_THAT(records.str(), ContainsSubstring(R"("warnings":[{"kind":"path-not-utf8"}])"));
}
