// The translation of Python's expressions to ICU's — issue #499, ADR 0036.

#include <subedit/core/text/python_regex_syntax.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::core::icuExpressionOf;

namespace {

std::string translated(const std::string& python) {
    const auto icu = icuExpressionOf(python);
    REQUIRE(icu.has_value());
    return *icu;
}

} // namespace

TEST_CASE("what is written the same in both syntaxes is not touched", "[text][pattern][syntax]") {
    for (const char* same : {"a",
                             R"(\d+)",
                             R"(\s{2,})",
                             "(?<=[a-z])I",
                             R"(^([\-\–\—])(\S))",
                             R"(\A x \.)",
                             "(a|b)+?"}) {
        CHECK(translated(same) == same);
    }
}

TEST_CASE("the end of the text is \\z in ICU", "[text][pattern][syntax]") {
    CHECK(translated(R"(#+.+?(#+|\Z))") == R"(#+.+?(#+|\z))");
}

TEST_CASE("a word character is a letter, a number or an underscore, as in Python",
          "[text][pattern][syntax]") {
    CHECK(translated(R"(\w)") == R"([\p{L}\p{N}_])");
    CHECK(translated(R"(\W)") == R"([^\p{L}\p{N}_])");
    // Inside a class, `\w` is the contents of the set and `\W` a nested one.
    CHECK(translated(R"([\w#])") == R"([\p{L}\p{N}_#])");
    CHECK(translated(R"([^\W\d])") == R"([^[^\p{L}\p{N}_]\d])");
    // A boundary is where a word character meets what is not one, on either side.
    const std::string boundary = translated(R"(\b)");
    CHECK_THAT(boundary, ContainsSubstring(R"((?<=[\p{L}\p{N}_])(?![\p{L}\p{N}_]))"));
    CHECK_THAT(boundary, ContainsSubstring(R"((?<![\p{L}\p{N}_])(?=[\p{L}\p{N}_]))"));
    const std::string inside = translated(R"(\B)");
    CHECK_THAT(inside, ContainsSubstring(R"((?<=[\p{L}\p{N}_])(?=[\p{L}\p{N}_]))"));
}

TEST_CASE("an escaped backslash is not the start of an escape", "[text][pattern][syntax]") {
    CHECK(translated(R"(\\w)") == R"(\\w)");
    CHECK(translated(R"(\\\w)") == R"(\\[\p{L}\p{N}_])");
}

TEST_CASE("a bracket or an ampersand pair inside a class is a literal one",
          "[text][pattern][syntax]") {
    CHECK(translated("[a[b]") == R"([a\[b])");
    CHECK(translated("[a&&b]") == R"([a\&&b])");
    CHECK(translated("[a&b]") == "[a&b]");
    // A `]` first in a class, after an optional `^`, is a character.
    CHECK(translated("[]a]") == R"([\]a])");
    CHECK(translated("[^]a]") == R"([^\]a])");
}

TEST_CASE("Python's named groups are ICU's", "[text][pattern][syntax]") {
    CHECK(translated("(?P<word>a)(?P=word)") == R"((?<word>a)\k<word>)");
}

TEST_CASE("what cannot be said in ICU is refused", "[text][pattern][syntax]") {
    const auto underscore = icuExpressionOf("(?P<the_word>a)");
    REQUIRE_FALSE(underscore.has_value());
    CHECK_THAT(underscore.error().reason, ContainsSubstring("the_word"));

    CHECK_FALSE(icuExpressionOf("(?P<open").has_value());
    CHECK_FALSE(icuExpressionOf("(?P=open").has_value());
    CHECK_FALSE(icuExpressionOf("(?P<1>a)").has_value());
    CHECK_FALSE(icuExpressionOf("a\\").has_value());
}
