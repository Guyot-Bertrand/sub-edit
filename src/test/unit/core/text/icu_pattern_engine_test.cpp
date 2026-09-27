// The ICU engine at the interface level — issue #499, ADR 0036: the parts of
// `PatternMatcher::find` that a correction never reaches, because it never
// hands the engine what these cases hand it.

#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::IcuPatternEngine;
using subedit::core::Match;
using subedit::core::PatternFlags;
using subedit::core::PatternMatcher;

std::unique_ptr<PatternMatcher> compiled(std::string_view expression) {
    const IcuPatternEngine engine;
    auto matcher = engine.compile(expression, PatternFlags{});
    REQUIRE(matcher.has_value());
    return std::move(*matcher);
}

/// The one match `matcher` finds in `text` from `from` — fails the test, by
/// name, if there is none.
///
/// **The access sits inside the `if` that guards it**, the shape
/// `common_errors.cpp`'s own `replaceAll` uses for the same nested type: a
/// `REQUIRE` followed by a later, separate dereference is two statements the
/// static analyser does not connect through a `std::expected<std::optional<…>>`,
/// however the two are written.
Match matchOf(PatternMatcher& matcher, std::string_view text, std::size_t from) {
    const auto found = matcher.find(text, from);
    if (found.has_value() && found->has_value())
        return **found;
    FAIL("no match found");
    return {};
}

} // namespace

TEST_CASE("a search starting past the end of the text finds nothing", "[text][pattern][engine]") {
    const std::unique_ptr<PatternMatcher> matcher = compiled("a");

    const auto found = matcher->find("a", 2);
    REQUIRE(found.has_value());
    CHECK_FALSE(found->has_value());
}

TEST_CASE("a text that is not valid UTF-8 is still searched leniently", "[text][pattern][engine]") {
    // ICU opens malformed UTF-8 rather than refusing it — measured, not
    // documented behaviour — so the pattern still finds the 'a' that follows
    // the bad bytes. What this proves is the absence of a crash, not a
    // particular reading of the two stray bytes.
    const std::unique_ptr<PatternMatcher> matcher = compiled("a");
    const std::string invalid{"\xFF"
                              "\xFE"
                              "a"};

    const auto found = matcher->find(invalid, 0);
    REQUIRE(found.has_value());
    CHECK(found->has_value());
}

TEST_CASE("a group number is read by its name and nothing by an unknown one",
          "[text][pattern][engine]") {
    const std::unique_ptr<PatternMatcher> matcher = compiled("(?<word>a)(b)");

    CHECK(matcher->groupNumber("word") == std::optional<int>{1});
    CHECK(matcher->groupNumber("nobody") == std::nullopt);
    CHECK(matcher->groupCount() == 2);
}

TEST_CASE("every group named or not is found at the offsets ICU gives", "[text][pattern][engine]") {
    const std::unique_ptr<PatternMatcher> matcher = compiled("(a)(x)?");

    const Match match = matchOf(*matcher, "a", 0);
    REQUIRE(match.groups.size() == 3);
    CHECK(match.whole().start == 0);
    CHECK(match.whole().end == 1);
    CHECK(match.groups[1].has_value());
    // The optional group did not take part: nothing, not a zero-length span.
    CHECK_FALSE(match.groups[2].has_value());
}
