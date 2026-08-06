#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace {

using subedit::core::DecimalMark;
using subedit::core::Timestamp;

std::optional<std::int64_t> parsed(std::string_view text) {
    const std::optional<Timestamp> position = Timestamp::parse(text);
    if (!position.has_value())
        return std::nullopt;
    return position->milliseconds();
}

} // namespace

TEST_CASE("a well-formed timestamp is read back", "[time][parse]") {
    CHECK(parsed("00:00:00,000") == 0);
    CHECK(parsed("01:01:01,500") == 3661500);
    CHECK(parsed("99:59:59,999") == 359999999);
}

TEST_CASE("either decimal mark is accepted", "[time][parse]") {
    // A file is read as it comes: a period in a SubRip file is a mistake we
    // recover from, not a reason to lose the subtitle.
    CHECK(parsed("01:01:01.500") == 3661500);
    CHECK(parsed("01:01:01,500") == 3661500);
}

TEST_CASE("fields may be written with a single digit", "[time][parse]") {
    CHECK(parsed("1:2:3,400") == 3723400);
    CHECK(parsed("0:0:0,000") == 0);
}

TEST_CASE("the decimals are read as a fraction of a second", "[time][parse]") {
    CHECK(parsed("00:00:00,4") == 400);
    CHECK(parsed("00:00:00,45") == 450);
    CHECK(parsed("00:00:00,456") == 456);
}

TEST_CASE("the decimals may be missing altogether", "[time][parse]") {
    CHECK(parsed("00:00:01") == 1000);
    CHECK(parsed("1:2:3") == 3723000);
}

TEST_CASE("the hours may be missing, as WebVTT allows", "[time][parse]") {
    CHECK(parsed("01:01.500") == 61500);
    CHECK(parsed("59:59,999") == 3599999);
    CHECK(parsed("0:0") == 0);
}

TEST_CASE("a leading sign reads as a position before the origin", "[time][parse]") {
    CHECK(parsed("-00:00:01,500") == -1500);
    CHECK(parsed("-01:01.500") == -61500);
}

TEST_CASE("surrounding blanks are ignored", "[time][parse]") {
    CHECK(parsed("  01:01:01,500") == 3661500);
    CHECK(parsed("01:01:01,500\t ") == 3661500);
}

TEST_CASE("text that is not a timestamp is refused", "[time][parse]") {
    // Refusing is what lets the reader report a diagnostic on the line and
    // move on, instead of inventing a position.
    CHECK(parsed("") == std::nullopt);
    CHECK(parsed("-") == std::nullopt);
    CHECK(parsed("hello") == std::nullopt);
    CHECK(parsed("00:00:01,500 --> 00:00:02,000") == std::nullopt);
    CHECK(parsed("+00:00:01,500") == std::nullopt);
    CHECK(parsed("00:00:01,500x") == std::nullopt);
    CHECK(parsed("00 00 01 500") == std::nullopt);
    CHECK(parsed("0a:00:00,000") == std::nullopt);
    CHECK(parsed("00:00:00:00,000") == std::nullopt);
}

TEST_CASE("a field out of its range is refused", "[time][parse]") {
    CHECK(parsed("00:60:00,000") == std::nullopt);
    CHECK(parsed("00:00:60,000") == std::nullopt);
    CHECK(parsed("100:00:00,000") == std::nullopt);
    CHECK(parsed("00:00:00,0000") == std::nullopt);
    CHECK(parsed("00:00:00,") == std::nullopt);
    CHECK(parsed("00:") == std::nullopt);
}

TEST_CASE("a written position reads back identical", "[time][parse]") {
    const std::array<std::int64_t, 7> samples = {0, 1, 999, 1000, 61500, 3661500, 359999999};

    for (const std::int64_t milliseconds : samples) {
        for (const std::int64_t signedMilliseconds : {milliseconds, -milliseconds}) {
            const Timestamp original = Timestamp::fromMilliseconds(signedMilliseconds);

            CHECK(Timestamp::parse(original.format(DecimalMark::Comma)) == original);
            CHECK(Timestamp::parse(original.format(DecimalMark::Period)) == original);
        }
    }
}
