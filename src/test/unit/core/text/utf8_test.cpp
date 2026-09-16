// Walking UTF-8 one code point at a time — what the parser and the case share.

#include <subedit/core/text/utf8.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string_view>

namespace {

using subedit::core::codePointAt;
using subedit::core::nextCodePoint;
using subedit::core::previousCodePoint;

/// One code point of each length: one, two, three and four bytes.
constexpr std::string_view kEveryLength = "aéﬁ𝒜";

} // namespace

TEST_CASE("a code point is read whatever its length", "[text][utf8]") {
    CHECK(codePointAt(kEveryLength, 0) == U'a');
    CHECK(codePointAt(kEveryLength, 1) == U'é');
    CHECK(codePointAt(kEveryLength, 3) == U'ﬁ');
    CHECK(codePointAt(kEveryLength, 6) == U'𝒜');
}

TEST_CASE("the walk steps over whole code points, forwards", "[text][utf8]") {
    CHECK(nextCodePoint(kEveryLength, 0) == 1);
    CHECK(nextCodePoint(kEveryLength, 1) == 3);
    CHECK(nextCodePoint(kEveryLength, 3) == 6);
    CHECK(nextCodePoint(kEveryLength, 6) == kEveryLength.size());
}

TEST_CASE("the walk steps over whole code points, backwards", "[text][utf8]") {
    CHECK(previousCodePoint(kEveryLength, kEveryLength.size()) == 6);
    CHECK(previousCodePoint(kEveryLength, 6) == 3);
    CHECK(previousCodePoint(kEveryLength, 3) == 1);
    CHECK(previousCodePoint(kEveryLength, 1) == 0);
}
