#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A colour, as the formats write one.
///
/// **Two orders, and neither is more natural than the other.** HTML writes
/// `RRGGBB`, Sub Station Alpha and MicroDVD write `BBGGRR`. Keeping the three
/// channels rather than the six characters is what makes the reversal a
/// property of the writing rather than of the value.
class Colour {

public:
    [[nodiscard]] static constexpr Colour
    fromChannels(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
        return Colour{red, green, blue};
    }

    /// Reads `RRGGBB`, or nothing if that is not what it is.
    [[nodiscard]] static std::optional<Colour> parse(std::string_view text);

    /// Reads `BBGGRR`, the order the two brace vocabularies write.
    ///
    /// **Short values are padded on the left**, as Gaupol pads them: a
    /// `{\c&Hff00&}` is `00ff00`, not `ff0000`. Leading zeroes are what a real
    /// file omits.
    [[nodiscard]] static std::optional<Colour> parseReversed(std::string_view text);

    [[nodiscard]] std::string toString() const;

    [[nodiscard]] std::string toReversedString() const;

    [[nodiscard]] friend bool operator==(Colour, Colour) = default;

private:
    constexpr Colour(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
        : m_red(red), m_green(green), m_blue(blue) {}

    std::uint8_t m_red;
    std::uint8_t m_green;
    std::uint8_t m_blue;
};

/// One thing a style says, and the unit a conversion loses.
///
/// **Six, and they are Gaupol's** — ADR 0031 fixes the scope there and nowhere
/// else. A count of lost tags is a count of these: a run that goes from bold
/// and red to plain in a format that writes neither has lost two.
enum class StyleAttribute {
    Bold,
    Italic,
    Underline,
    Colour,
    Font,
    Size,
};

/// The six, in the order the enumeration declares them.
///
/// **For walking them, never for deciding anything**, as `kSubtitleFormats` is
/// for the formats. It is also the order tags are opened in, which is why it is
/// worth being an order rather than a set: a nesting that always opens in the
/// same order closes in the same order too.
inline constexpr std::array<StyleAttribute, 6> kStyleAttributes = {
    StyleAttribute::Bold,
    StyleAttribute::Italic,
    StyleAttribute::Underline,
    StyleAttribute::Colour,
    StyleAttribute::Font,
    StyleAttribute::Size,
};

/// How a run of text is drawn.
///
/// **A resolved state and not a stack of tags.** `<b><i>x</i></b>` and
/// `<i><b>x</b></i>` say the same thing about `x`, and a pivot that kept the
/// nesting would have to decide they are equal anyway. Keeping what applies,
/// rather than how it was written, is what lets one vocabulary be compared to
/// another at all.
struct Style {
    bool bold = false;
    bool italic = false;
    bool underline = false;
    std::optional<Colour> colour{};
    std::optional<std::string> font{};
    std::optional<int> size{};

    /// Tells whether this style says anything about `attribute`.
    [[nodiscard]] bool carries(StyleAttribute attribute) const;

    /// Drops what this style says about `attribute`.
    [[nodiscard]] Style without(StyleAttribute attribute) const;

    /// How many of the six this style says something about.
    [[nodiscard]] std::size_t count() const;

    /// Tells whether this style says nothing at all.
    [[nodiscard]] bool isPlain() const { return count() == 0; }

    friend bool operator==(const Style&, const Style&) = default;
};

/// One run of text, and everything that applies to it.
struct StyledRun {
    std::string text{};
    Style style{};

    friend bool operator==(const StyledRun&, const StyledRun&) = default;
};

/// The pivot: a text cut into runs, each carrying one style — ADR 0031.
///
/// **It lives the time of a conversion and no longer.** A text that was read
/// stays the raw string ADR 0009 keeps it as; nothing here is stored, and
/// opening a file and saving it in its own format never builds one.
using StyledText = std::vector<StyledRun>;

/// Appends `text` under `style`, joining it to the run before when it can.
///
/// **Joining matters to the encoders, not to the reader.** A vocabulary writes
/// one tag per run, so two adjacent runs that say the same thing would be
/// written as two identical tags in a row. Nothing downstream would be wrong;
/// the file would just be uglier than the one it came from.
void appendRun(StyledText& runs, std::string_view text, const Style& style);

/// The text alone, every tag gone.
[[nodiscard]] std::string plainTextOf(const StyledText& runs);

} // namespace subedit::core
