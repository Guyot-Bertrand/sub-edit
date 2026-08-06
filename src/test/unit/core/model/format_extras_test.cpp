#include <subedit/core/model/format_extras.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <variant>

namespace {

using subedit::core::FormatExtras;
using subedit::core::Rectangle;
using subedit::core::SubRipExtras;
using subedit::core::WebVttExtras;

} // namespace

TEST_CASE("extras hold nothing for a format that carries no data of its own", "[model][extras]") {
    const FormatExtras extras;

    CHECK(std::holds_alternative<std::monostate>(extras));
}

TEST_CASE("SubRip extras carry the extended coordinates", "[model][extras]") {
    const SubRipExtras subRip{
        .coordinates = Rectangle{.x1 = 40, .x2 = 600, .y1 = 20, .y2 = 460},
    };

    REQUIRE(subRip.coordinates.has_value());
    const Rectangle box = *subRip.coordinates;

    CHECK(box.x1 == 40);
    CHECK(box.x2 == 600);
    CHECK(box.y1 == 20);
    CHECK(box.y2 == 460);
}

TEST_CASE("SubRip coordinates are absent by default", "[model][extras]") {
    // Absent and « all four at zero » are different things: Gaupol writes the
    // coordinates only when they are not all zero, so the distinction has to
    // survive a read followed by a write.
    const SubRipExtras subRip;

    CHECK_FALSE(subRip.coordinates.has_value());
}

TEST_CASE("WebVTT extras carry what would otherwise be lost", "[model][extras]") {
    // Dropping the settings of a cue loses its position on screen.
    const WebVttExtras webVtt{
        .id = "chapitre-1",
        .settings = "align:start position:10%",
        .style = "STYLE\n::cue { color: yellow }",
        .comment = "NOTE traduction à revoir",
    };

    CHECK(webVtt.id == "chapitre-1");
    CHECK(webVtt.settings == "align:start position:10%");
    CHECK(webVtt.style == "STYLE\n::cue { color: yellow }");
    CHECK(webVtt.comment == "NOTE traduction à revoir");
}

TEST_CASE("extras keep their alternative and their contents through a copy", "[model][extras]") {
    FormatExtras original{WebVttExtras{.id = "cue-7", .settings = "line:0"}};

    const FormatExtras copy = original;
    // The original moves on to another format; the copy must not follow it.
    original = SubRipExtras{};

    REQUIRE(std::holds_alternative<WebVttExtras>(copy));
    CHECK(std::get<WebVttExtras>(copy).id == "cue-7");
    CHECK(std::get<WebVttExtras>(copy).settings == "line:0");
}

TEST_CASE("the compiler knows which fields a format has", "[model][extras]") {
    // The point of a variant over Gaupol's bag of dynamic attributes: asking a
    // SubRip subtitle for its cue settings does not compile, and an exhaustive
    // visit is checked rather than hoped for.
    const FormatExtras extras{SubRipExtras{}};

    const std::string kind = std::visit(
        []<typename Alternative>(const Alternative&) -> std::string {
            if constexpr (std::is_same_v<Alternative, SubRipExtras>)
                return "subrip";
            else if constexpr (std::is_same_v<Alternative, WebVttExtras>)
                return "webvtt";
            else
                return "aucun";
        },
        extras);

    CHECK(kind == "subrip");
}
