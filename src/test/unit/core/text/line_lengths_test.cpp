// The length of each line of a subtitle — issue #526.

#include <subedit/core/text/line_lengths.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

using subedit::core::CharacterLineMeasure;
using subedit::core::lineLengths;
using subedit::core::MarkupVocabulary;

namespace {

/// A measure that reads every character as one and a half, so the rounding
/// down has something to round.
class HalfwayMeasure final : public subedit::core::LineMeasure {

public:
    [[nodiscard]] double lengthOf(std::string_view text) const override {
        return 1.5 * static_cast<double>(text.size());
    }
};

} // namespace

TEST_CASE("each line of a text has its own length", "[text][line-lengths]") {
    const CharacterLineMeasure measure;
    CHECK(lineLengths(measure, "Hello", MarkupVocabulary::Html) == std::vector<int>{5});
    CHECK(lineLengths(measure, "Hi\nthere you", MarkupVocabulary::Html) == std::vector<int>{2, 9});
}

TEST_CASE("an empty text is one empty line, and an empty line counts zero",
          "[text][line-lengths]") {
    const CharacterLineMeasure measure;
    CHECK(lineLengths(measure, "", MarkupVocabulary::Html) == std::vector<int>{0});
    CHECK(lineLengths(measure, "a\n\nb", MarkupVocabulary::Html) == std::vector<int>{1, 0, 1});
    CHECK(lineLengths(measure, "a\n", MarkupVocabulary::Html) == std::vector<int>{1, 0});
}

TEST_CASE("lengths count code points, not bytes", "[text][line-lengths]") {
    const CharacterLineMeasure measure;
    CHECK(lineLengths(measure, "café\nété", MarkupVocabulary::Html) == std::vector<int>{4, 3});
}

TEST_CASE("tags are not counted, in whichever vocabulary wrote them", "[text][line-lengths]") {
    const CharacterLineMeasure measure;
    CHECK(lineLengths(measure, "<i>Hello</i>\n<b>it</b>", MarkupVocabulary::Html) ==
          std::vector<int>{5, 2});
    CHECK(lineLengths(measure, R"({\i1}Hello{\i0})", MarkupVocabulary::SubStationAlpha) ==
          std::vector<int>{5});
    // No vocabulary: a bracket is a character.
    CHECK(lineLengths(measure, "<i>a", MarkupVocabulary::None) == std::vector<int>{4});
}

TEST_CASE("a length is rounded down", "[text][line-lengths]") {
    const HalfwayMeasure measure;
    // 1.5, 3.0, 4.5
    CHECK(lineLengths(measure, "a\naa\naaa", MarkupVocabulary::None) == std::vector<int>{1, 3, 4});
}
