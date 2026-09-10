#include <subedit/core/format/degradation.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup_conversion.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::int64_t kMillisecond = 1;
constexpr std::int64_t kCentisecond = 10;
constexpr std::int64_t kTenth = 100;
constexpr std::int64_t kSecond = 1000;

/// The millisecond grain a time format writes on, or nothing for MicroDVD.
[[nodiscard]] std::optional<std::int64_t> grainOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
    case SubtitleFormat::WebVtt:
        return kMillisecond;
    case SubtitleFormat::SubViewer2:
    case SubtitleFormat::SubStationAlpha:
    case SubtitleFormat::AdvancedSubStationAlpha:
    case SubtitleFormat::Lrc:
        return kCentisecond;
    case SubtitleFormat::Mpl2:
        return kTenth;
    case SubtitleFormat::TMPlayer:
        return kSecond;
    case SubtitleFormat::MicroDvd:
        return std::nullopt;
    }
    std::unreachable();
}

/// How far `position` is from where `format` would put it.
[[nodiscard]] std::int64_t shiftOf(Timestamp position, SubtitleFormat format, FrameRate rate) {
    const std::int64_t moved = asWrittenBy(position, format, rate).milliseconds();
    const std::int64_t gap = moved - position.milliseconds();
    return gap < 0 ? -gap : gap;
}

} // namespace

bool carriesEnds(SubtitleFormat format) {
    return format != SubtitleFormat::TMPlayer && format != SubtitleFormat::Lrc;
}

bool holdsLineBreaks(SubtitleFormat format) {
    return format != SubtitleFormat::Lrc;
}

Timestamp asWrittenBy(Timestamp position, SubtitleFormat format, FrameRate rate) {
    const std::optional<std::int64_t> grain = grainOf(format);
    if (!grain.has_value())
        return Timestamp::fromFrame(position.toFrame(rate), rate);

    // Halves away from zero, the rule every writer of the project follows.
    const std::int64_t milliseconds = position.milliseconds();
    const std::int64_t half = *grain / 2;
    const std::int64_t rounded =
        milliseconds < 0 ? (milliseconds - half) / *grain : (milliseconds + half) / *grain;
    return Timestamp::fromMilliseconds(rounded * *grain);
}

bool ConversionLoss::isAny() const {
    return ends || joined > 0 || tags > 0 || header || fields > 0 || precision > 0;
}

ConversionLoss convertFor(std::vector<Subtitle>& subtitles,
                          const SourceFile& source,
                          SubtitleFormat target,
                          FrameRate rate) {
    ConversionLoss loss;
    if (source.format == target)
        return loss;

    loss.ends = carriesEnds(source.format) && !carriesEnds(target);
    // **A header is lost the moment the format changes**, and `headerFor` is
    // what enforces it: the writer of another format would not take it back.
    loss.header = !source.header.empty();

    for (Subtitle& subtitle : subtitles) {
        for (const Document document : {Document::Main, Document::Translation}) {
            std::string& text = subtitle.text(document);
            if (text.empty())
                continue;
            const ConvertedMarkup carried = convertMarkup(text, source.format, target);
            loss.tags += carried.dropped;
            text = carried.text;
        }

        if (!holdsLineBreaks(target) && subtitle.mainText.contains('\n'))
            ++loss.joined;

        if (carriesFields(subtitle.extras))
            ++loss.fields;

        // **Only the positions the file will hold are measured.** An end that
        // is not written cannot have moved, and counting its rounding would
        // report a shift nobody can see.
        loss.precision = std::max(loss.precision, shiftOf(subtitle.start, target, rate));
        if (carriesEnds(target))
            loss.precision = std::max(loss.precision, shiftOf(subtitle.end, target, rate));
    }

    return loss;
}

} // namespace subedit::core
