#include <subedit/cli/adjusting.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/editing.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>

namespace subedit::cli {

namespace {

constexpr std::string_view kOff = "off";

/// A decimal number as typed: digits, then optionally a point and digits.
///
/// Narrower than what `from_chars` reads, on purpose: it takes `1e3`, `inf` and
/// `nan`, none of which is a reading speed anyone meant.
[[nodiscard]] bool isDecimal(std::string_view text) {
    const std::size_t point = text.find('.');
    const std::string_view whole = text.substr(0, point);
    const std::string_view fraction =
        point == std::string_view::npos ? std::string_view{"0"} : text.substr(point + 1);
    const auto digits = [](std::string_view part) {
        return !part.empty() &&
               std::ranges::all_of(part, [](char c) { return c >= '0' && c <= '9'; });
    };
    return digits(whole) && digits(fraction);
}

[[nodiscard]] std::expected<double, std::string> parseSpeed(std::string_view text) {
    double speed = 0.0;
    if (isDecimal(text)) {
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), speed);
        if (error == std::errc{} && end == text.data() + text.size() && speed > 0.0) {
            return speed;
        }
    }
    return std::unexpected{"--speed: \"" + std::string{text} +
                           "\" is not a reading speed: expected a number of characters per second "
                           "above zero, like 15 or 12.5, or off"};
}

/// A duration option: `off`, a time, or the default when not given.
///
/// `allowZero` is the difference between a gap, a minimum and a maximum: the
/// first two may be zero, and a maximum of zero would end every subtitle where
/// it starts.
[[nodiscard]] std::expected<std::optional<core::Duration>, std::string>
parseLimit(std::string_view option, std::string_view text, bool allowZero) {
    const std::expected<core::Duration, std::string> time = parseTime(text);
    if (!time) {
        return std::unexpected{std::string{option} + ": " + time.error()};
    }
    const std::int64_t milliseconds = time->milliseconds();
    if (milliseconds < 0 || (milliseconds == 0 && !allowZero)) {
        return std::unexpected{
            std::string{option} + ": \"" + std::string{text} + "\" is not " +
            (allowZero ? "a duration of zero or more" : "a duration above zero")};
    }
    return std::optional{*time};
}

[[nodiscard]] Json millisecondsOf(const std::optional<core::Duration>& duration) {
    return duration ? Json{duration->milliseconds()} : Json{};
}

[[nodiscard]] std::string decimalOf(double value) {
    // Room for the shortest round-trip form of any double.
    constexpr std::size_t kLongestDouble = 32;
    std::array<char, kLongestDouble> buffer{};
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    return std::string{buffer.data(), result.ptr};
}

} // namespace

std::expected<core::DurationConstraints, std::string> constraintsOf(const AdjustOptions& options) {
    core::DurationConstraints constraints;

    // The reading speed, whose two directions are options of their own.
    if (options.speed == kOff) {
        if (options.shorten || options.noLengthen) {
            return std::unexpected{
                "--shorten and --no-lengthen say which way the reading speed may move an end, "
                "and --speed off switches the reading speed off"};
        }
        constraints.speed.reset();
    } else {
        double speed = core::kDefaultReadingSpeed;
        if (!options.speed.empty()) {
            const std::expected<double, std::string> read = parseSpeed(options.speed);
            if (!read) {
                return std::unexpected{read.error()};
            }
            speed = *read;
        }
        constraints.speed = core::ReadingSpeed::create(speed, !options.noLengthen, options.shorten);
    }

    if (options.minimum == kOff) {
        constraints.minimum.reset();
    } else if (!options.minimum.empty()) {
        const auto read = parseLimit("--minimum", options.minimum, true);
        if (!read) {
            return std::unexpected{read.error()};
        }
        constraints.minimum = *read;
    }

    if (!options.maximum.empty()) {
        const auto read = parseLimit("--maximum", options.maximum, false);
        if (!read) {
            return std::unexpected{read.error()};
        }
        constraints.maximum = *read;
    }

    if (options.gap == kOff) {
        constraints.gap.reset();
    } else if (!options.gap.empty()) {
        const auto read = parseLimit("--gap", options.gap, true);
        if (!read) {
            return std::unexpected{read.error()};
        }
        constraints.gap = *read;
    }

    if (!constraints.isAny()) {
        return std::unexpected{
            "adjust has nothing to adjust to: every constraint is off, or the reading speed "
            "is told to move no end"};
    }
    return constraints;
}

Json constraintsRecord(const core::DurationConstraints& constraints) {
    Json speed;
    if (constraints.speed) {
        speed = Json::object()
                    .set("cps", decimalOf(constraints.speed->charactersPerSecond()))
                    .set("lengthen", constraints.speed->lengthen())
                    .set("shorten", constraints.speed->shorten());
    }
    return Json::object()
        .set("speed", std::move(speed))
        .set("minimum_ms", millisecondsOf(constraints.minimum))
        .set("maximum_ms", millisecondsOf(constraints.maximum))
        .set("gap_ms", millisecondsOf(constraints.gap));
}

ExitCode adjustAll(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   const core::DurationConstraints& constraints,
                   const std::optional<Range>& range,
                   const Destination& destination,
                   const Reporter& reporter) {
    const ChangingOperation adjust = [constraints](core::Session& session,
                                                   const Request& request) -> OperationOutcome {
        core::DurationAdjustment adjustment =
            core::adjustDurations(session.project(), request.selection, constraints);
        const std::size_t target = request.selection.count();

        // Read before the command is moved: what was sacrificed is said even
        // when no end moved.
        const core::SacrificedConstraints sacrificed = adjustment.sacrificed;
        const std::size_t adjusted = adjustment.adjusted;
        if (adjustment.command != nullptr) {
            session.apply(std::move(adjustment.command));
        }

        const auto count = [](std::size_t number) { return static_cast<std::int64_t>(number); };
        return OperationResult{
            .sentence = core::noticeOfAdjustment(adjusted, sacrificed),
            .counts = {{"subtitles", count(target)},
                       {"adjusted", count(adjusted)},
                       {"sacrificed",
                        std::vector<Count>{{"speed", count(sacrificed.speed)},
                                           {"minimum", count(sacrificed.minimum)},
                                           {"gap", count(sacrificed.gap)}}}},
            .fields = {{"constraints", constraintsRecord(constraints)}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "adjusted", adjust, range);
}

} // namespace subedit::cli
