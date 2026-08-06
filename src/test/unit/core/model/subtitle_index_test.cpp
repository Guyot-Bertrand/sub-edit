#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <type_traits>

namespace {

using subedit::core::SubtitleIndex;

} // namespace

TEST_CASE("an index carries the position it was built from", "[model][index]") {
    CHECK(SubtitleIndex::fromValue(0).value() == 0);
    CHECK(SubtitleIndex::fromValue(41).value() == 41);
}

TEST_CASE("an index knows the number the user reads", "[model][index]") {
    // Zero-based inside, one-based on screen. Confusing the two is the classic
    // off-by-one of a subtitle editor, so the conversion is written once here
    // rather than at every display site.
    CHECK(SubtitleIndex::fromValue(0).number() == 1);
    CHECK(SubtitleIndex::fromValue(41).number() == 42);
}

TEST_CASE("an index is built from the number the user reads", "[model][index]") {
    CHECK(SubtitleIndex::fromNumber(1) == SubtitleIndex::fromValue(0));
    CHECK(SubtitleIndex::fromNumber(42) == SubtitleIndex::fromValue(41));
}

TEST_CASE("indices compare and order by position", "[model][index]") {
    CHECK(SubtitleIndex::fromValue(1) < SubtitleIndex::fromValue(2));
    CHECK(SubtitleIndex::fromValue(2) > SubtitleIndex::fromValue(1));
    CHECK(SubtitleIndex::fromValue(7) == SubtitleIndex::fromValue(7));
    CHECK(SubtitleIndex::fromValue(7) != SubtitleIndex::fromValue(8));
}

TEST_CASE("an index is not a count in disguise", "[model][index]") {
    static_assert(!std::is_convertible_v<SubtitleIndex, std::size_t>);
    static_assert(!std::is_convertible_v<std::size_t, SubtitleIndex>);
    static_assert(!std::is_constructible_v<SubtitleIndex, std::size_t>);
    static_assert(std::is_trivially_copyable_v<SubtitleIndex>);
}
