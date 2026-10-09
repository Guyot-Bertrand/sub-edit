// The `plural=` expression of a catalogue — issue #658.

#include <subedit/core/i18n/plural_expression.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

using subedit::core::PluralExpression;

namespace {

PluralExpression parsed(const std::string& text) {
    auto expression = PluralExpression::parse(text);
    REQUIRE(expression.has_value());
    return *expression;
}

constexpr const char* kRussian =
    "n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2";
constexpr const char* kPolish = "n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2";

} // namespace

TEST_CASE("English's rule is the default one", "[i18n][plural]") {
    const auto rule = PluralExpression::germanic();
    CHECK(rule.evaluate(0) == 1);
    CHECK(rule.evaluate(1) == 0);
    CHECK(rule.evaluate(2) == 1);
}

TEST_CASE("French counts zero as a singular", "[i18n][plural]") {
    const auto rule = parsed("(n > 1)");
    CHECK(rule.evaluate(0) == 0);
    CHECK(rule.evaluate(1) == 0);
    CHECK(rule.evaluate(2) == 1);
}

TEST_CASE("Russian has three forms", "[i18n][plural]") {
    const auto rule = parsed(kRussian);
    CHECK(rule.evaluate(0) == 2);
    CHECK(rule.evaluate(1) == 0);
    CHECK(rule.evaluate(2) == 1);
    CHECK(rule.evaluate(5) == 2);
    CHECK(rule.evaluate(11) == 2);
    CHECK(rule.evaluate(21) == 0);
    CHECK(rule.evaluate(22) == 1);
    CHECK(rule.evaluate(25) == 2);
    CHECK(rule.evaluate(101) == 0);
    CHECK(rule.evaluate(111) == 2);
    CHECK(rule.evaluate(112) == 2);
}

TEST_CASE("Polish has three forms, and one has no teens", "[i18n][plural]") {
    const auto rule = parsed(kPolish);
    CHECK(rule.evaluate(1) == 0);
    CHECK(rule.evaluate(2) == 1);
    CHECK(rule.evaluate(12) == 2);
    CHECK(rule.evaluate(22) == 1);
}

TEST_CASE("the operators bind as in C", "[i18n][plural]") {
    CHECK(parsed("1 + 2 * 3").evaluate(0) == 7);
    CHECK(parsed("(1 + 2) * 3").evaluate(0) == 9);
    CHECK(parsed("10 - 3 - 2").evaluate(0) == 5);
    CHECK(parsed("7 / 2").evaluate(0) == 3);
    CHECK(parsed("7 % 4").evaluate(0) == 3);
    CHECK(parsed("1 || 0 && 0").evaluate(0) == 1);
    CHECK(parsed("!0").evaluate(0) == 1);
    CHECK(parsed("!!5").evaluate(0) == 1);
    CHECK(parsed("1 < 2 == 1").evaluate(0) == 1);
    CHECK(parsed("2 <= 2").evaluate(0) == 1);
    CHECK(parsed("2 >= 3").evaluate(0) == 0);
    CHECK(parsed("n != 3").evaluate(3) == 0);
    CHECK(parsed("n ? 5 : 6").evaluate(0) == 6);
    CHECK(parsed("n ? 5 : n ? 6 : 7").evaluate(0) == 7);
    CHECK(parsed("  n  ==  1  ").evaluate(1) == 1);
}

TEST_CASE("an expression that does not read is refused with a reason", "[i18n][plural]") {
    for (const char* text : {"",
                             "n +",
                             "(n",
                             "n)",
                             "n ? 1",
                             "n @ 3",
                             "n n",
                             "1 2",
                             "n =",
                             "n >> 1",
                             "99999999999999999999999",
                             "?",
                             "n ? : 1",
                             "n ? 1 :"}) {
        INFO(text);
        const auto expression = PluralExpression::parse(text);
        REQUIRE_FALSE(expression.has_value());
        CHECK_FALSE(expression.error().empty());
    }
}

TEST_CASE("an expression too long, too large or too deep is refused", "[i18n][plural]") {
    CHECK_FALSE(PluralExpression::parse(std::string(2000, ' ') + "n").has_value());
    CHECK_FALSE(
        PluralExpression::parse(std::string(500, '(') + "n" + std::string(500, ')')).has_value());

    // Short enough for the length limit, and still more nodes than the cap.
    std::string sum = "n";
    for (int i = 0; i < 300; ++i) {
        sum += "+n";
    }
    CHECK_FALSE(PluralExpression::parse(sum).has_value());

    const std::string negations(200, '!');
    CHECK_FALSE(PluralExpression::parse(negations + "n").has_value());
}

TEST_CASE("evaluating never divides by zero nor overflows", "[i18n][plural]") {
    CHECK(parsed("5 / n").evaluate(0) == 0);
    CHECK(parsed("5 % n").evaluate(0) == 0);

    constexpr auto kLargest = UINT64_MAX;
    CHECK(parsed("n * n * n").evaluate(kLargest) == kLargest);
    CHECK(parsed("n + n").evaluate(kLargest) == kLargest - 1);
    CHECK(parsed("0 - n").evaluate(1) == kLargest);
}
