#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/markup_parser.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

std::optional<ReadingSpeed>
ReadingSpeed::create(double charactersPerSecond, bool lengthen, bool shorten) {
    // Written as a negation so that NaN, which compares false to everything,
    // is refused with the rest.
    if (!(charactersPerSecond > 0.0))
        return std::nullopt;

    return ReadingSpeed{charactersPerSecond, lengthen, shorten};
}

namespace {

/// How many characters `text` shows, tags left out.
///
/// Characters and not bytes: `len` counts code points, and a French subtitle
/// counted in bytes would be read slower than it is. A UTF-8 continuation byte
/// is the only kind that does not start a character.
[[nodiscard]] std::size_t visibleLength(std::string_view text, SubtitleFormat format) {
    const MarkupParser parser{text, format};
    constexpr unsigned kContinuationMask = 0xC0U;
    constexpr unsigned kContinuation = 0x80U;
    return static_cast<std::size_t>(std::ranges::count_if(parser.visible(), [](char byte) {
        return (static_cast<unsigned char>(byte) & kContinuationMask) != kContinuation;
    }));
}

/// How long `length` characters take to read at `speed`.
///
/// One rounding, to the millisecond and here: the speed is a decimal the user
/// typed, and a duration is a whole number of milliseconds.
[[nodiscard]] Duration readingTimeOf(std::size_t length, const ReadingSpeed& speed) {
    constexpr double kMillisecondsPerSecond = 1000.0;
    const double milliseconds =
        static_cast<double>(length) * kMillisecondsPerSecond / speed.charactersPerSecond();
    return Duration::fromMilliseconds(static_cast<std::int64_t>(std::llround(milliseconds)));
}

/// Tells whether a duration of `shown` breaks the reading speed, given which
/// way the speed was allowed to move the end.
[[nodiscard]] bool breaksSpeed(Duration shown, Duration needed, const ReadingSpeed& speed) {
    return (speed.lengthen() && shown < needed) || (speed.shorten() && shown > needed);
}

/// What one subtitle is measured against, read once from the project.
struct Bounds {
    /// The time its text needs, when a reading speed is asked.
    std::optional<Duration> needed{};

    /// Where the next subtitle of the project starts, when there is one.
    std::optional<Timestamp> next{};
};

[[nodiscard]] Bounds
boundsOf(const Project& project, SubtitleIndex index, const DurationConstraints& constraints) {
    Bounds bounds;
    if (const std::optional<ReadingSpeed>& speed = constraints.speed; speed.has_value()) {
        // The main text, in the main file's format, and by design: a reading
        // speed is what the viewer reads on the picture, and the picture shows
        // the main document. A translation in another format changes nothing.
        const std::string_view text = project.subtitleAt(index).mainText;
        bounds.needed =
            readingTimeOf(visibleLength(text, project.sourceFile(Document::Main).format), *speed);
    }
    if (index.value() + 1 < project.count())
        bounds.next = project.subtitleAt(SubtitleIndex::fromValue(index.value() + 1)).start;
    return bounds;
}

/// Returns the end the four constraints give, applied in Gaupol's order.
[[nodiscard]] Timestamp endFor(Timestamp start,
                               Timestamp end,
                               const Bounds& bounds,
                               const DurationConstraints& constraints) {
    // 1 — reading speed.
    const std::optional<ReadingSpeed>& speed = constraints.speed;
    if (speed.has_value() && bounds.needed.has_value() &&
        breaksSpeed(end - start, *bounds.needed, *speed))
        end = start + *bounds.needed;

    // 2 — minimum, 3 — maximum.
    if (constraints.minimum.has_value() && end - start < *constraints.minimum)
        end = start + *constraints.minimum;
    if (constraints.maximum.has_value() && end - start > *constraints.maximum)
        end = start + *constraints.maximum;

    // 4 — the gap, last, and it wins. Never before the start: an end moved
    // before its own start would trade one fault for a worse one.
    if (constraints.gap.has_value() && bounds.next.has_value() &&
        *bounds.next - end < *constraints.gap)
        end = std::max(start, *bounds.next - *constraints.gap);

    return end;
}

/// Counts what an end of `end` still breaks, which is what was given up.
void countSacrifices(Timestamp start,
                     Timestamp end,
                     const Bounds& bounds,
                     const DurationConstraints& constraints,
                     SacrificedConstraints& sacrificed) {
    const Duration shown = end - start;

    const std::optional<ReadingSpeed>& speed = constraints.speed;
    if (speed.has_value() && bounds.needed.has_value() &&
        breaksSpeed(shown, *bounds.needed, *speed))
        ++sacrificed.speed;
    if (constraints.minimum.has_value() && shown < *constraints.minimum)
        ++sacrificed.minimum;
    if (constraints.gap.has_value() && bounds.next.has_value() &&
        *bounds.next - end < *constraints.gap)
        ++sacrificed.gap;
}

} // namespace

DurationAdjustment adjustDurations(const Project& project,
                                   const Selection& selection,
                                   const DurationConstraints& constraints) {
    DurationAdjustment adjustment;
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : selection.indices()) {
        const Subtitle& subtitle = project.subtitleAt(index);
        const Bounds bounds = boundsOf(project, index, constraints);
        const Timestamp end = endFor(subtitle.start, subtitle.end, bounds, constraints);

        countSacrifices(subtitle.start, end, bounds, constraints, adjustment.sacrificed);

        if (end != subtitle.end)
            commands.push_back(
                std::make_unique<SetPositionCommand>(project, index, Boundary::End, end));
    }

    adjustment.adjusted = commands.size();
    if (!commands.empty())
        adjustment.command =
            std::make_unique<CompositeCommand>(CommandKind::AdjustDurations, std::move(commands));
    return adjustment;
}

} // namespace subedit::core
