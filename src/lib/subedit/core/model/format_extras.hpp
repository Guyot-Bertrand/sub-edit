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

/// What a Sub Station Alpha subtitle carries beyond positions and text.
///
/// **One branch for the two formats**, which is what they are: `ass.py`
/// inherits from `ssa.py` in Gaupol, and the event fields differ by one. The
/// first is `Marked=0` in SSA and a layer number in Advanced SSA — the same
/// column, holding a different thing — so it is held here as the number both
/// are, and each writer says it its own way.
struct SubStationAlphaExtras {
    int layer = 0;
    std::string style{};
    std::string name{};
    int marginLeft = 0;
    int marginRight = 0;
    int marginVertical = 0;
    std::string effect{};

    friend bool operator==(const SubStationAlphaExtras&, const SubStationAlphaExtras&) = default;
};

/// The format-specific data of a subtitle, if its format has any.
///
/// A variant rather than Gaupol's bag of dynamic attributes: the compiler
/// knows which fields exist, asking a SubRip subtitle for its cue settings
/// does not compile, and the exhaustiveness of a visit is checked instead of
/// hoped for. `std::monostate` covers the formats with no data of their own.
/// **Four branches for nine formats, and counting says why.** Five of the nine
/// carry nothing of their own per subtitle — SubViewer 2, MicroDVD, MPL2,
/// TMPlayer and LRC — and Sub Station Alpha and Advanced SSA share one. The
/// question the scoping asked, whether nine formats would make this a variant
/// of nine branches, is answered by the formats themselves.
using FormatExtras =
    std::variant<std::monostate, SubRipExtras, WebVttExtras, SubStationAlphaExtras>;

} // namespace subedit::core
