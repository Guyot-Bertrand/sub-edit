#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/web_vtt_writer.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace subedit::core {

namespace {

constexpr std::string_view kSignature = "WEBVTT";
constexpr std::string_view kArrow = " --> ";
constexpr std::int64_t kOneHourInMilliseconds = 3600000;

/// Tells whether a position needs its hours field written.
[[nodiscard]] bool reachesAnHour(Timestamp position) {
    const std::int64_t milliseconds = position.milliseconds();
    const std::int64_t magnitude = milliseconds < 0 ? -milliseconds : milliseconds;
    return magnitude >= kOneHourInMilliseconds;
}

/// Decides once, for the whole file, how the timestamps are shaped.
[[nodiscard]] HourField hourFieldFor(std::span<const Subtitle> subtitles) {
    const bool anyReachesAnHour = std::ranges::any_of(subtitles, [](const Subtitle& subtitle) {
        return reachesAnHour(subtitle.start) || reachesAnHour(subtitle.end);
    });
    return anyReachesAnHour ? HourField::Always : HourField::Omitted;
}

/// Returns the extras of `subtitle`, or empty ones if it carries another
/// format's — a project opened as SubRip and saved as WebVTT.
[[nodiscard]] WebVttExtras extrasOf(const Subtitle& subtitle) {
    const auto* extras = std::get_if<WebVttExtras>(&subtitle.extras);
    return extras == nullptr ? WebVttExtras{} : *extras;
}

/// Appends a block preceded by a blank line, as the format separates them.
void appendBlock(std::string& out, std::string_view block, std::string_view ending) {
    if (block.empty())
        return;
    out += ending;
    appendWithEnding(out, block, ending);
    out += ending;
}

} // namespace

std::string WebVttWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);
    const HourField hourField = hourFieldFor(request.subtitles);

    std::string out;

    // A file without cues is still a WebVTT file; writing nothing at all would
    // produce something no reader accepts.
    const std::string_view header = trimmedBlanks(request.header);
    appendWithEnding(out, header.empty() ? kSignature : header, ending);
    out += ending;

    for (const Subtitle& subtitle : request.subtitles) {
        const WebVttExtras extras = extrasOf(subtitle);

        appendBlock(out, extras.style, ending);
        appendBlock(out, extras.comment, ending);

        out += ending;

        if (!extras.id.empty()) {
            out += extras.id;
            out += ending;
        }

        out += subtitle.start.format(DecimalMark::Period, hourField);
        out += kArrow;
        out += subtitle.end.format(DecimalMark::Period, hourField);
        if (!extras.settings.empty()) {
            out += ' ';
            out += extras.settings;
        }
        out += ending;

        appendWithEnding(out, subtitle.text(request.document), ending);
        out += ending;
    }

    return out;
}

} // namespace subedit::core
