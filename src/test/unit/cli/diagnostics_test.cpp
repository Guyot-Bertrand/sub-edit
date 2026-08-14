#include <subedit/cli/diagnostics.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/diagnostic.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Reporter;
using subedit::cli::reportOf;
using subedit::cli::sayDiagnostics;
using subedit::core::Diagnostic;
using subedit::core::DiagnosticKind;
using subedit::core::Severity;

namespace {

Diagnostic at(int line, DiagnosticKind kind, Severity severity, std::string detail = {}) {
    return Diagnostic{
        .severity = severity, .line = line, .kind = kind, .detail = std::move(detail)};
}

std::string said(const std::vector<Diagnostic>& diagnostics, int level) {
    std::ostringstream errors;
    sayDiagnostics(Reporter{errors, level}, "a.srt", diagnostics);
    return errors.str();
}

} // namespace

TEST_CASE("a diagnostic names its line, what happened and what was done", "[cli][diagnostics]") {
    CHECK(reportOf(at(7, DiagnosticKind::EndBeforeStart, Severity::Warning)) ==
          "line 7: a subtitle that ends before it starts, left as it stands");
}

TEST_CASE("a diagnostic the reader settled says so", "[cli][diagnostics]") {
    CHECK(reportOf(at(3, DiagnosticKind::MissingNumbering, Severity::Recovered)) ==
          "line 3: a SubRip block without its number, settled by the reader");
}

TEST_CASE("what the category cannot carry is quoted after it", "[cli][diagnostics]") {
    // The detail holds the offending text of the file itself, so it is quoted:
    // unquoted, a line ending in a comma would read as part of the sentence.
    CHECK(reportOf(at(5, DiagnosticKind::MalformedTimestamp, Severity::Warning, "00:00:01 -> 3")) ==
          "line 5: a timing line that could not be read (\"00:00:01 -> 3\"), left as it stands");
}

TEST_CASE("an empty detail adds nothing", "[cli][diagnostics]") {
    // No empty parentheses: most diagnostics carry none.
    CHECK_THAT(reportOf(at(2, DiagnosticKind::MixedNewlines, Severity::Recovered)),
               !ContainsSubstring("("));
}

TEST_CASE("the detail is truncated rather than allowed to flood the line", "[cli][diagnostics]") {
    // It comes from the file, so its length is not ours to trust.
    const std::string flood(400, 'x');
    const std::string reported =
        reportOf(at(1, DiagnosticKind::IgnoredLine, Severity::Recovered, flood));

    CHECK(reported.size() < 200);
    CHECK_THAT(reported, ContainsSubstring("…"));
}

TEST_CASE("diagnostics are said one by one, at the level that details", "[cli][diagnostics]") {
    const std::vector<Diagnostic> two = {
        at(3, DiagnosticKind::MissingNumbering, Severity::Recovered),
        at(7, DiagnosticKind::EndBeforeStart, Severity::Warning),
    };

    const std::string third = said(two, 3);
    CHECK_THAT(third,
               ContainsSubstring("a.srt: line 3: a SubRip block without its number, "
                                 "settled by the reader\n"));
    CHECK_THAT(third,
               ContainsSubstring("a.srt: line 7: a subtitle that ends before it starts, "
                                 "left as it stands\n"));
}

TEST_CASE("nothing is said below the level that details", "[cli][diagnostics]") {
    const std::vector<Diagnostic> one = {
        at(3, DiagnosticKind::MissingNumbering, Severity::Recovered)};

    CHECK(said(one, 2).empty());
    CHECK(said(one, 1).empty());
    CHECK(said(one, 0).empty());
}

TEST_CASE("a file the reader had nothing to say about says nothing", "[cli][diagnostics]") {
    CHECK(said({}, 3).empty());
}
