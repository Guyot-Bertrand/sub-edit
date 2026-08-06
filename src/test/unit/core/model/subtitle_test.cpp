#include <subedit/core/model/document.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <variant>

namespace {

using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::SubRipExtras;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::WebVttExtras;

Subtitle sample() {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(1000),
        .end = Timestamp::fromMilliseconds(3500),
        .mainText = "Bonjour.",
        .translationText = "Hello.",
    };
}

} // namespace

TEST_CASE("a subtitle carries two positions and two texts", "[model][subtitle]") {
    const Subtitle subtitle = sample();

    CHECK(subtitle.start == Timestamp::fromMilliseconds(1000));
    CHECK(subtitle.end == Timestamp::fromMilliseconds(3500));
    CHECK(subtitle.mainText == "Bonjour.");
    CHECK(subtitle.translationText == "Hello.");
}

TEST_CASE("the duration is derived from the two positions, never stored", "[model][subtitle]") {
    Subtitle subtitle = sample();

    CHECK(subtitle.duration() == Duration::fromMilliseconds(2500));

    subtitle.end = Timestamp::fromMilliseconds(5000);

    CHECK(subtitle.duration() == Duration::fromMilliseconds(4000));
}

TEST_CASE("a subtitle whose end precedes its start is representable", "[model][subtitle]") {
    // Assumed exception to the principle of unrepresentable invalid states,
    // justified by ADR 0008: a real file may contain the anomaly, and the user
    // has to see it to fix it. Refusing to hold it would mean refusing to open
    // the file.
    const Subtitle subtitle{
        .start = Timestamp::fromMilliseconds(4000),
        .end = Timestamp::fromMilliseconds(1000),
    };

    CHECK(subtitle.duration() == Duration::fromMilliseconds(-3000));
    CHECK(subtitle.duration() < Duration::zero());
}

TEST_CASE("a default subtitle sits at the origin, empty", "[model][subtitle]") {
    const Subtitle subtitle;

    CHECK(subtitle.start == Timestamp::origin());
    CHECK(subtitle.end == Timestamp::origin());
    CHECK(subtitle.duration() == Duration::zero());
    CHECK(subtitle.mainText.empty());
    CHECK(subtitle.translationText.empty());
    CHECK(std::holds_alternative<std::monostate>(subtitle.extras));
}

TEST_CASE("a document selects one of the two texts", "[model][subtitle]") {
    const Subtitle subtitle = sample();

    CHECK(subtitle.text(Document::Main) == "Bonjour.");
    CHECK(subtitle.text(Document::Translation) == "Hello.");
}

TEST_CASE("a document selects the text to write into", "[model][subtitle]") {
    Subtitle subtitle = sample();

    subtitle.text(Document::Translation) = "Good morning.";

    CHECK(subtitle.translationText == "Good morning.");
    CHECK(subtitle.mainText == "Bonjour.");
}

TEST_CASE("copying a subtitle keeps its WebVTT extras", "[model][subtitle]") {
    Subtitle subtitle = sample();
    subtitle.extras = WebVttExtras{.id = "cue-3", .settings = "align:end"};

    const Subtitle copy = subtitle;

    REQUIRE(std::holds_alternative<WebVttExtras>(copy.extras));
    CHECK(std::get<WebVttExtras>(copy.extras).id == "cue-3");
    CHECK(std::get<WebVttExtras>(copy.extras).settings == "align:end");
}

TEST_CASE("copying a subtitle keeps its SubRip extras", "[model][subtitle]") {
    using subedit::core::Rectangle;

    Subtitle subtitle = sample();
    subtitle.extras = SubRipExtras{
        .coordinates = Rectangle{.x1 = 1, .x2 = 2, .y1 = 3, .y2 = 4},
    };

    const Subtitle copy = subtitle;

    REQUIRE(std::holds_alternative<SubRipExtras>(copy.extras));
    const SubRipExtras extras = std::get<SubRipExtras>(copy.extras);
    if (!extras.coordinates.has_value()) {
        FAIL("the copy lost the SubRip coordinates");
        return;
    }

    CHECK(*extras.coordinates == Rectangle{.x1 = 1, .x2 = 2, .y1 = 3, .y2 = 4});
}
