// The pattern of `replace` and the loop that applies it, on an in-memory file system.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/replacing.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::compilePattern;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Range;
using subedit::cli::replaceIn;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;
using subedit::core::SearchOptions;

namespace {

const std::string kFile = "1\n00:00:01,000 --> 00:00:02,000\nHello hello.\n\n"
                          "2\n00:00:03,000 --> 00:00:04,000\nHello again\n\n";

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
    std::string records;
};

Run run(const std::string& pattern,
        const std::string& replacement,
        SearchOptions options = {},
        const std::optional<Range>& range = std::nullopt,
        bool dryRun = false) {
    InMemoryFileSystem files;
    files.addFile("a.srt", kFile);
    std::ostringstream errors;
    std::ostringstream records;
    const auto compiled = compilePattern(pattern, options);
    REQUIRE(compiled.has_value());

    const ExitCode code =
        replaceIn(files,
                  {"a.srt"},
                  std::nullopt,
                  *compiled,
                  pattern,
                  replacement,
                  range,
                  Destination::from("", dryRun ? "" : "out", false, 1, dryRun).value(),
                  Reporter{errors, 1}.withRecords(records).forCommand("replace"));
    return {.code = code,
            .written = files.contentOf("out/a.srt").value_or(""),
            .errors = errors.str(),
            .records = records.str()};
}

} // namespace

TEST_CASE("a pattern that is empty is refused with the reason of the window",
          "[cli][replacing][CLI-REPLACE-02]") {
    const auto compiled = compilePattern("", {});

    REQUIRE_FALSE(compiled.has_value());
    CHECK(compiled.error() == "pattern: nothing to look for");
}

TEST_CASE("an expression ICU cannot read is refused, naming what went wrong",
          "[cli][replacing][CLI-REPLACE-02]") {
    const auto compiled = compilePattern("(", SearchOptions{.regex = true, .ignoreCase = true});

    REQUIRE_FALSE(compiled.has_value());
    CHECK_THAT(compiled.error(), ContainsSubstring("pattern: not a regular expression ("));
    // The same text is plain text without --regex, and readable.
    CHECK(compilePattern("(", {}).has_value());
}

TEST_CASE("every match is replaced, the case being ignored by default",
          "[cli][replacing][CLI-REPLACE-03]") {
    const Run done = run("hello", "bye");

    CHECK(done.code == ExitCode::Success);
    CHECK_THAT(done.written, ContainsSubstring("bye bye."));
    CHECK_THAT(done.written, ContainsSubstring("bye again"));
    CHECK_THAT(done.errors, ContainsSubstring("replaced 3 matches"));
    CHECK_THAT(done.records, ContainsSubstring("\"counts\":{\"replaced\":3,\"matched\":3}"));
}

TEST_CASE("case sensitivity tells the capital", "[cli][replacing][CLI-REPLACE-03]") {
    const Run done = run("hello", "bye", SearchOptions{.regex = false, .ignoreCase = false});

    CHECK_THAT(done.written, ContainsSubstring("Hello bye."));
    CHECK_THAT(done.errors, ContainsSubstring("replaced 1 match"));
}

TEST_CASE("a range limits where it looks", "[cli][replacing][CLI-REPLACE-01]") {
    const Run done = run("hello", "bye", {}, Range{.first = 2, .last = 2});

    CHECK_THAT(done.written, ContainsSubstring("Hello hello."));
    CHECK_THAT(done.written, ContainsSubstring("bye again"));
    CHECK_THAT(done.records, ContainsSubstring("\"replaced\":1,\"matched\":1"));
}

TEST_CASE("nothing found is not nothing to change", "[cli][replacing][CLI-REPLACE-04]") {
    const Run missing = run("zzz", "x");
    CHECK_THAT(missing.errors, ContainsSubstring("\"zzz\" not found"));
    CHECK(missing.written == kFile);
    CHECK_THAT(missing.records, ContainsSubstring("\"replaced\":0,\"matched\":0"));

    const Run itself = run("again", "again");
    CHECK_THAT(itself.errors, ContainsSubstring("nothing to change"));
    CHECK(itself.written == kFile);
    CHECK_THAT(itself.records, ContainsSubstring("\"replaced\":0,\"matched\":1"));
}

TEST_CASE("a dry run lists the changes and writes nothing", "[cli][replacing][CLI-REPLACE-04]") {
    const Run done = run("again", "once", {}, std::nullopt, true);

    CHECK(done.code == ExitCode::Success);
    CHECK(done.written.empty());
    CHECK_THAT(done.errors, ContainsSubstring("(dry run, nothing written)"));
    CHECK_THAT(done.records,
               ContainsSubstring("\"changes\":[{\"subtitle\":2,\"document\":\"main\","
                                 "\"before\":\"Hello again\",\"after\":\"Hello once\"}]"));
}
