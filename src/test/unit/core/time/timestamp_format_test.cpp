#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

namespace {

using subedit::core::DecimalMark;
using subedit::core::HourField;
using subedit::core::Timestamp;

std::string formatted(std::int64_t milliseconds, DecimalMark mark = DecimalMark::Comma) {
    return Timestamp::fromMilliseconds(milliseconds).format(mark);
}

} // namespace

TEST_CASE("a position is written as HH:MM:SS with three decimals", "[time][format]") {
    CHECK(formatted(0) == "00:00:00,000");
    CHECK(formatted(1) == "00:00:00,001");
    CHECK(formatted(3661500) == "01:01:01,500");
    CHECK(formatted(359999999) == "99:59:59,999");
}

TEST_CASE("the decimal mark is chosen by the caller", "[time][format]") {
    // SubRip writes a comma, WebVTT a period. The difference belongs to the
    // format, not to the position.
    CHECK(formatted(3661500, DecimalMark::Comma) == "01:01:01,500");
    CHECK(formatted(3661500, DecimalMark::Period) == "01:01:01.500");
}

TEST_CASE("a position before the origin is written with a leading sign", "[time][format]") {
    CHECK(formatted(-1) == "-00:00:00,001");
    CHECK(formatted(-3661500) == "-01:01:01,500");
}

TEST_CASE("writing saturates at ninety-nine hours", "[time][format]") {
    // A constraint of the file formats, not of the representation: int64_t
    // holds far more, and the position itself is left untouched.
    CHECK(formatted(360000000) == "99:59:59,999");
    CHECK(formatted(999999999999) == "99:59:59,999");
    CHECK(formatted(-360000000) == "-99:59:59,999");
    CHECK(Timestamp::fromMilliseconds(360000000).milliseconds() == 360000000);
}

TEST_CASE("the hours field can be left out below one hour", "[time][format]") {
    // WebVTT allows MM:SS.mmm, and Gaupol drops the hours when no subtitle
    // reaches one hour.
    const Timestamp position = Timestamp::fromMilliseconds(61500);

    CHECK(position.format(DecimalMark::Period, HourField::Omitted) == "01:01.500");
    CHECK(Timestamp::fromMilliseconds(-61500).format(DecimalMark::Period, HourField::Omitted) ==
          "-01:01.500");
}

TEST_CASE("the hours field comes back when the position reaches one hour", "[time][format]") {
    // Minutes stay within two digits and below sixty; folding an hour into
    // them would produce «61:01.500», which no reader accepts.
    const Timestamp position = Timestamp::fromMilliseconds(3661500);

    CHECK(position.format(DecimalMark::Period, HourField::Omitted) == "01:01:01.500");
    CHECK(Timestamp::fromMilliseconds(-3600000).format(DecimalMark::Period, HourField::Omitted) ==
          "-01:00:00.000");
}
