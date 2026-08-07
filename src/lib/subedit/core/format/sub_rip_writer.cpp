#include <subedit/core/format/sub_rip_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

namespace subedit::core {

namespace {

constexpr std::string_view kArrow = " --> ";
constexpr std::size_t kCoordinateDigits = 3;

/// Appends `value` on at least `width` digits, padded with zeroes.
void appendPadded(std::string& text, int value, std::size_t width) {
    const std::string digits = std::to_string(value);
    if (digits.size() < width)
        text.append(width - digits.size(), '0');
    text += digits;
}

/// Appends `  X1:040 X2:600 Y1:020 Y2:460`, if there is anything to append.
///
/// Coordinates all at zero are left out, as Gaupol does: a file that never
/// carried them must not gain them by being saved.
void appendCoordinates(std::string& text, const FormatExtras& extras) {
    const SubRipExtras* subRip = std::get_if<SubRipExtras>(&extras);
    if (subRip == nullptr || !subRip->coordinates.has_value())
        return;

    const Rectangle box = *subRip->coordinates;
    if (box == Rectangle{})
        return;

    text += "  X1:";
    appendPadded(text, box.x1, kCoordinateDigits);
    text += " X2:";
    appendPadded(text, box.x2, kCoordinateDigits);
    text += " Y1:";
    appendPadded(text, box.y1, kCoordinateDigits);
    text += " Y2:";
    appendPadded(text, box.y2, kCoordinateDigits);
}

} // namespace

std::string SubRipWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);

    std::string out;
    int numbering = 0;
    for (const Subtitle& subtitle : request.subtitles) {
        ++numbering;

        out += std::to_string(numbering);
        out += ending;

        out += subtitle.start.format(DecimalMark::Comma);
        out += kArrow;
        out += subtitle.end.format(DecimalMark::Comma);
        appendCoordinates(out, subtitle.extras);
        out += ending;

        appendWithEnding(out, subtitle.text(request.document), ending);
        out += ending;

        // The blank line closing every block, the last one included.
        out += ending;
    }
    return out;
}

} // namespace subedit::core
