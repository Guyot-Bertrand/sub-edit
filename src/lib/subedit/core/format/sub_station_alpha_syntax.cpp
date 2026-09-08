#include <subedit/core/format/sub_station_alpha_syntax.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/lines.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

using namespace event_field;

constexpr std::array<std::string_view, 11> kKnownFields = {
    kMarked,
    kLayer,
    kStart,
    kEnd,
    kStyle,
    kName,
    kMarginLeft,
    kMarginRight,
    kMarginVertical,
    kEffect,
    kText,
};

/// The nine columns the two formats share, after the first.
constexpr std::array<std::string_view, 9> kSharedFields = {
    kStart,
    kEnd,
    kStyle,
    kName,
    kMarginLeft,
    kMarginRight,
    kMarginVertical,
    kEffect,
    kText,
};

constexpr std::string_view kHardBreak = "\\N";
constexpr std::string_view kSoftBreak = "\\n";

} // namespace

bool isKnownEventField(std::string_view name) {
    return std::ranges::contains(kKnownFields, name);
}

std::vector<std::string> defaultEventFields(SubtitleFormat format) {
    std::vector<std::string> fields;
    fields.reserve(kSharedFields.size() + 1);
    fields.emplace_back(format == SubtitleFormat::AdvancedSubStationAlpha ? kLayer : kMarked);
    for (const std::string_view field : kSharedFields)
        fields.emplace_back(field);
    return fields;
}

std::vector<std::string_view> splitEventLine(std::string_view values, std::size_t columns) {
    std::vector<std::string_view> found;
    if (columns == 0)
        return found;
    found.reserve(columns);

    std::string_view rest = values;
    while (found.size() + 1 < columns) {
        const std::size_t comma = rest.find(',');
        if (comma == std::string_view::npos)
            break;
        found.push_back(trimmedBlanks(rest.substr(0, comma)));
        rest = rest.substr(comma + 1);
    }
    // Whatever is left is the last column, commas and blanks included: it is
    // the text, and trimming it would eat a space the author wrote.
    found.push_back(rest);
    return found;
}

std::string textFromEvent(std::string_view value) {
    std::string text;
    for (std::size_t index = 0; index < value.size();) {
        const std::string_view rest = value.substr(index);
        if (rest.starts_with(kHardBreak) || rest.starts_with(kSoftBreak)) {
            text += '\n';
            index += kHardBreak.size();
            continue;
        }
        text += value[index];
        ++index;
    }
    return text;
}

std::string textToEvent(std::string_view text) {
    std::string value;
    for (const char character : text) {
        if (character == '\n')
            value += kHardBreak;
        else
            value += character;
    }
    return value;
}

} // namespace subedit::core
