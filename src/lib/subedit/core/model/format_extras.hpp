#pragma once

#include <optional>
#include <string>
#include <variant>

namespace subedit::core {

/// The four Extended SubRip coordinates, in pixels.
///
/// They place the subtitle on screen. Gaupol writes them only when they are
/// not all zero, which is why `SubRipExtras` holds them as an option rather
/// than as four zeroed fields: absent and « all four at zero » are different
/// states, and both have to survive a read followed by a write.
struct Rectangle {
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;

    friend bool operator==(const Rectangle&, const Rectangle&) = default;
};

/// What a SubRip subtitle carries beyond positions and text.
struct SubRipExtras {
    std::optional<Rectangle> coordinates{};

    friend bool operator==(const SubRipExtras&, const SubRipExtras&) = default;
};

/// What a WebVTT cue carries beyond positions and text.
///
/// Losing `settings` loses the position of the cue on screen; losing `style`
/// or `comment` loses blocks the author wrote deliberately. All of it has to
/// come back out unchanged.
struct WebVttExtras {
    std::string id{};
    std::string settings{};
    std::string style{};
    std::string comment{};

    friend bool operator==(const WebVttExtras&, const WebVttExtras&) = default;
};

/// The format-specific data of a subtitle, if its format has any.
///
/// A variant rather than Gaupol's bag of dynamic attributes: the compiler
/// knows which fields exist, asking a SubRip subtitle for its cue settings
/// does not compile, and the exhaustiveness of a visit is checked instead of
/// hoped for. `std::monostate` covers the formats with no data of their own.
using FormatExtras = std::variant<std::monostate, SubRipExtras, WebVttExtras>;

} // namespace subedit::core
