// The replacements of Gaupol, read as Python's `re.sub` reads them — issue #499,
// decision D3.

#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/replacement_template.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::core::Match;
using subedit::core::MatchSpan;
using subedit::core::ReplacementTemplate;

namespace {

std::optional<int> noNames(std::string_view /*name*/) {
    return std::nullopt;
}

std::optional<int> theWordIsGroupTwo(std::string_view name) {
    return name == "word" ? std::optional<int>{2} : std::nullopt;
}

/// The text a template makes of a match of `text` whose groups are the spans
/// given — group 0 first — with `groupCount` groups in the pattern.
std::string expanded(std::string_view replacement,
                     std::string_view text,
                     std::vector<std::optional<MatchSpan>> groups,
                     std::size_t groupCount) {
    const auto parsed = ReplacementTemplate::parse(replacement, groupCount, theWordIsGroupTwo);
    REQUIRE(parsed.has_value());
    return parsed->expandedFor(text, Match{.groups = std::move(groups)});
}

std::string reasonOf(std::string_view replacement, std::size_t groupCount = 0) {
    const auto parsed = ReplacementTemplate::parse(replacement, groupCount, noNames);
    REQUIRE_FALSE(parsed.has_value());
    return parsed.error().reason;
}

/// "ab cd": the whole match, then `ab` and `cd`.
const std::vector<std::optional<MatchSpan>> kTwoWords{MatchSpan{.start = 0, .end = 5},
                                                      MatchSpan{.start = 0, .end = 2},
                                                      MatchSpan{.start = 3, .end = 5}};

} // namespace

TEST_CASE("a replacement without a backslash is what it says", "[text][pattern][template]") {
    CHECK(expanded("l", "I", {MatchSpan{.start = 0, .end = 1}}, 0) == "l");
    CHECK(expanded("", "I", {MatchSpan{.start = 0, .end = 1}}, 0).empty());
    CHECK(expanded("O.K.", "ok", {MatchSpan{.start = 0, .end = 2}}, 0) == "O.K.");
}

TEST_CASE("a group is written \\N, or \\g<N>, or \\g<name>", "[text][pattern][template]") {
    CHECK(expanded(R"(\1 \2)", "ab cd", kTwoWords, 2) == "ab cd");
    CHECK(expanded(R"(\2\1)", "ab cd", kTwoWords, 2) == "cdab");
    CHECK(expanded(R"(\g<2>-\g<1>)", "ab cd", kTwoWords, 2) == "cd-ab");
    CHECK(expanded(R"(<\g<0>>)", "ab cd", kTwoWords, 2) == "<ab cd>");
    CHECK(expanded(R"(\g<word>)", "ab cd", kTwoWords, 2) == "cd");
}

TEST_CASE("a group that took no part stands for nothing", "[text][pattern][template]") {
    const std::vector<std::optional<MatchSpan>> second{
        MatchSpan{.start = 0, .end = 2}, std::nullopt, MatchSpan{.start = 0, .end = 2}};
    CHECK(expanded(R"([\1|\2])", "ab", second, 2) == "[|ab]");
}

TEST_CASE("a group the match does not carry stands for nothing, without reading past it",
          "[text][pattern][template]") {
    // The template was parsed for two groups; the match it is applied to here
    // has only the whole match — as could happen if a caller reused a template
    // against an engine that answered fewer groups than it promised at parsing.
    const auto parsed = ReplacementTemplate::parse(R"(\1[\2])", 2, noNames);
    REQUIRE(parsed.has_value());
    const std::vector<std::optional<MatchSpan>> onlyWhole{MatchSpan{.start = 0, .end = 1}};

    CHECK(parsed->expandedFor("a", Match{.groups = onlyWhole}) == "[]");
}

TEST_CASE("an octal escape is a character", "[text][pattern][template]") {
    // How the shipped files write a space and a zero without GKeyFile repairing
    // them — `-\040` and `\060\1`.
    CHECK(expanded(R"(-\040)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == "- ");
    CHECK(expanded(R"(\1\060)",
                   "1",
                   {MatchSpan{.start = 0, .end = 1}, MatchSpan{.start = 0, .end = 1}},
                   1) == "10");
    CHECK(expanded(R"(\101)", "x", {MatchSpan{.start = 0, .end = 1}}, 1) == "A");
    // Above 127, a character, written in UTF-8.
    CHECK(expanded(R"(\351)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == "\xC3\xA9");
    CHECK(expanded(R"(\0)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == std::string(1, '\0'));
}

TEST_CASE("one digit or two are a group, three octal digits a character",
          "[text][pattern][template]") {
    const std::vector<std::optional<MatchSpan>> groups(12, MatchSpan{.start = 0, .end = 1});
    CHECK(expanded(R"(\11)", "x", groups, 11) == "x");
    // `\12` is group twelve when there are twelve, and refused when there are not.
    CHECK_THAT(reasonOf(R"(\12)", 2), ContainsSubstring("invalid group reference 12"));
    // Three octal digits are a character even when the groups would fit.
    CHECK(expanded(R"(\123)", "x", groups, 11) == "S");
}

TEST_CASE("the escapes Python knows are what they say", "[text][pattern][template]") {
    CHECK(expanded(R"(a\nb\tc\\d)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == "a\nb\tc\\d");
    CHECK(expanded(R"(\a\b\f\r\v)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == "\a\b\f\r\v");
}

TEST_CASE("a backslash before something that is not a letter stays, both characters",
          "[text][pattern][template]") {
    CHECK(expanded(R"(\. \-)", "x", {MatchSpan{.start = 0, .end = 1}}, 0) == R"(\. \-)");
}

TEST_CASE("what Python refuses is refused", "[text][pattern][template]") {
    CHECK_THAT(reasonOf(R"(\q)"), ContainsSubstring("bad escape"));
    CHECK_THAT(reasonOf(R"(\)"), ContainsSubstring("backslash"));
    CHECK_THAT(reasonOf(R"(\1)"), ContainsSubstring("invalid group reference 1"));
    CHECK_THAT(reasonOf(R"(\g<3>)", 2), ContainsSubstring("invalid group reference 3"));
    CHECK_THAT(reasonOf(R"(\g<nobody>)", 2), ContainsSubstring("unknown group name"));
    CHECK_THAT(reasonOf(R"(\g1)"), ContainsSubstring("missing <"));
    CHECK_THAT(reasonOf(R"(\g<1)"), ContainsSubstring("missing >"));
    CHECK_THAT(reasonOf(R"(\g<>)"), ContainsSubstring("missing group name"));
    CHECK_THAT(reasonOf(R"(\477)"), ContainsSubstring("out of range"));
}
