#include <subedit/core/format/sub_station_alpha_syntax.hpp>
#include <subedit/core/format/sub_station_alpha_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

using namespace event_field;

constexpr std::size_t kMarginDigits = 4;

/// What a file that never had one gets, so that it can be read back.
///
/// Gaupol's templates, cut to what matters: the detection reads `ScriptType`,
/// and a file without a style declares nothing for its events to point at.
constexpr std::string_view kSsaHeader = "[Script Info]\n"
                                        "ScriptType: v4.00\n"
                                        "Collisions: Normal\n"
                                        "Timer: 100.0000\n"
                                        "\n"
                                        "[V4 Styles]\n"
                                        "Format: Name, Fontname, Fontsize, PrimaryColour, "
                                        "SecondaryColour, TertiaryColour, BackColour, Bold, "
                                        "Italic, BorderStyle, Outline, Shadow, Alignment, "
                                        "MarginL, MarginR, MarginV, AlphaLevel, Encoding\n"
                                        "Style: Default,Sans,18,&HFFFFFF,&HFFFF00,&H000000,"
                                        "&H000000,0,0,1,2,2,2,30,30,30,0,0";

constexpr std::string_view kAssHeader = "[Script Info]\n"
                                        "ScriptType: v4.00+\n"
                                        "Collisions: Normal\n"
                                        "Timer: 100.0000\n"
                                        "\n"
                                        "[V4+ Styles]\n"
                                        "Format: Name, Fontname, Fontsize, PrimaryColour, "
                                        "SecondaryColour, OutlineColour, BackColour, Bold, "
                                        "Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, "
                                        "Angle, BorderStyle, Outline, Shadow, Alignment, "
                                        "MarginL, MarginR, MarginV, Encoding\n"
                                        "Style: Default,Sans,18,&H00FFFFFF,&H00FFFF00,&H00000000,"
                                        "&H00000000,0,0,0,0,100,100,0,0.00,1,2,2,2,30,30,30,0";

/// Appends `value` on exactly `width` digits, as the margins are written.
void appendPadded(std::string& text, int value, std::size_t width) {
    const std::string digits = std::to_string(value);
    if (digits.size() < width)
        text.append(width - digits.size(), '0');
    text += digits;
}

/// The columns the file declared, or the ones its format uses by default.
[[nodiscard]] std::vector<std::string> fieldsOf(const WriteRequest& request,
                                                SubtitleFormat format) {
    const SubStationAlphaFile* file = std::get_if<SubStationAlphaFile>(&request.extras);
    if (file != nullptr && !file->eventFields.empty())
        return file->eventFields;
    return defaultEventFields(format);
}

/// What the subtitle carries of this format, or what a subtitle from elsewhere
/// is written as.
[[nodiscard]] SubStationAlphaExtras extrasOf(const Subtitle& subtitle) {
    const SubStationAlphaExtras* extras = std::get_if<SubStationAlphaExtras>(&subtitle.extras);
    return extras != nullptr ? *extras : SubStationAlphaExtras{};
}

void appendField(std::string& out,
                 std::string_view field,
                 const Subtitle& subtitle,
                 const SubStationAlphaExtras& extras,
                 Document document) {
    if (field == kMarked) {
        out += "Marked=";
        out += std::to_string(extras.marked);
    } else if (field == kLayer) {
        out += std::to_string(extras.layer);
    } else if (field == kStart || field == kEnd) {
        out += subtitle.position(field == kStart ? Boundary::Start : Boundary::End)
                   .format(DecimalMark::Period, HourField::Unpadded, Decimals::Centiseconds);
    } else if (field == kStyle) {
        out += extras.style;
    } else if (field == kName) {
        out += extras.name;
    } else if (field == kMarginLeft) {
        appendPadded(out, extras.marginLeft, kMarginDigits);
    } else if (field == kMarginRight) {
        appendPadded(out, extras.marginRight, kMarginDigits);
    } else if (field == kMarginVertical) {
        appendPadded(out, extras.marginVertical, kMarginDigits);
    } else if (field == kEffect) {
        out += extras.effect;
    } else {
        out += textToEvent(subtitle.text(document));
    }
}

} // namespace

std::string SubStationAlphaWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);
    const std::vector<std::string> fields = fieldsOf(request, m_format);

    std::string out;
    const std::string_view templateHeader =
        m_format == SubtitleFormat::AdvancedSubStationAlpha ? kAssHeader : kSsaHeader;
    appendWithEnding(out, request.header.empty() ? templateHeader : request.header, ending);
    out += ending;
    out += ending;

    out += "[Events]";
    out += ending;
    out += "Format: ";
    for (std::size_t index = 0; index < fields.size(); ++index) {
        if (index > 0)
            out += ", ";
        out += fields[index];
    }
    out += ending;

    for (const Subtitle& subtitle : request.subtitles) {
        const SubStationAlphaExtras extras = extrasOf(subtitle);
        out += "Dialogue: ";
        for (std::size_t index = 0; index < fields.size(); ++index) {
            if (index > 0)
                out += ',';
            appendField(out, fields[index], subtitle, extras, request.document);
        }
        out += ending;
    }
    return out;
}

} // namespace subedit::core
