// `LineMeasure` and its two noyau pieces — decision D5 of the spec of
// phase 12, issue #502.

#include <subedit/core/text/line_measure.hpp>

#include <catch2/catch_test_macros.hpp>

using subedit::core::CachedLineMeasure;
using subedit::core::CharacterLineMeasure;

TEST_CASE("the character measure counts code points, not bytes", "[text][line-break]") {
    const CharacterLineMeasure measure;
    CHECK(measure.lengthOf("") == 0.0);
    CHECK(measure.lengthOf("Hello") == 5.0);
    // "é" is two bytes in UTF-8, one code point.
    CHECK(measure.lengthOf("café") == 4.0);
}

TEST_CASE("a newline counts like any other character", "[text][line-break]") {
    const CharacterLineMeasure measure;
    CHECK(measure.lengthOf("a\nb") == 3.0);
}

namespace {

/// A measure that counts how many times it was really asked, so a cache in
/// front of it can be told apart from one that only pretends to be there.
class CountingMeasure final : public subedit::core::LineMeasure {

public:
    mutable int calls = 0;

    [[nodiscard]] double lengthOf(std::string_view text) const override {
        ++calls;
        return static_cast<double>(text.size());
    }
};

} // namespace

TEST_CASE("the cache answers a repeated text without asking again", "[text][line-break]") {
    const CountingMeasure counted;
    const CachedLineMeasure cached{counted};

    CHECK(cached.lengthOf("hello") == 5.0);
    CHECK(cached.lengthOf("hello") == 5.0);
    CHECK(cached.lengthOf("world!") == 6.0);

    CHECK(counted.calls == 2);
}
