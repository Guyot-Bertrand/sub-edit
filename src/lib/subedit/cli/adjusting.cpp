#include <subedit/cli/adjusting.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/i18n/messages.hpp>
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
#include <tuple>
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
    return std::unexpected{core::substitute(
        // TRANSLATORS: %1 is what the user typed. « off » is a value of the option, as typed.
        core::translate("--speed: \"%1\" is not a reading speed: expected a number of characters "
                        "per second above zero, like 15 or 12.5, or off"),
        {text})};
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
        // TRANSLATORS: %1 is an option, such as --minimum; %2 why its value does not read.
        return std::unexpected{core::substitute(core::translate("%1: %2"), {option, time.error()})};
    }
    const std::int64_t milliseconds = time->milliseconds();
    if (milliseconds < 0 || (milliseconds == 0 && !allowZero)) {
        return std::unexpected{core::substitute(
            // TRANSLATORS: %1 is an option, such as --minimum; %2 what the user typed.
            allowZero ? core::translate("%1: \"%2\" is not a duration of zero or more")
                      : core::translate("%1: \"%2\" is not a duration above zero"),
            {option, text})};
    }
    return std::optional{*time};
}

/// The reading speed the options leave: `current` — the file's — unless an
/// option names one, its two directions being options of their own.
[[nodiscard]] std::expected<std::optional<core::ReadingSpeed>, std::string>
speedFrom(const AdjustOptions& options,
          const core::DurationAdjustmentSettings& base,
          const std::optional<core::ReadingSpeed>& current) {
    for (const auto& [one, other, flag, opposite] :
         {std::tuple{"--lengthen", "--no-lengthen", options.lengthen, options.noLengthen},
          std::tuple{"--shorten", "--no-shorten", options.shorten, options.noShorten}}) {
        if (flag && opposite) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 and %2 are two options, such as --shorten and --no-shorten.
                core::translate("%1 and %2 say opposite things; give one of them"),
                {one, other})};
        }
    }
    const bool directed =
        options.lengthen || options.noLengthen || options.shorten || options.noShorten;
    if (options.speed == kOff) {
        if (directed) {
            return std::unexpected{std::string{core::translate(
                "--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading "
                "speed may move an end, and --speed off switches the reading speed off")}};
        }
        return std::nullopt;
    }
    const bool active = base.lengthen || base.shorten;
    if (!active && options.speed.empty() && !directed) {
        return current;
    }
    double speed = base.charactersPerSecond;
    if (!options.speed.empty()) {
        const std::expected<double, std::string> read = parseSpeed(options.speed);
        if (!read) {
            return std::unexpected{read.error()};
        }
        speed = *read;
    }
    // A flag says its direction, whatever the file said (#705).
    const bool lengthen =
        options.lengthen || (!options.noLengthen && (active ? base.lengthen : true));
    const bool shorten = options.shorten || (!options.noShorten && active && base.shorten);
    // A speed of zero edited into the file is no speed: switched off, not
    // refused (ADR 0022).
    if (speed > 0.0) {
        return core::ReadingSpeed::create(speed, lengthen, shorten);
    }
    return std::nullopt;
}

/// A duration limit the options leave: `off` switches it off, a time replaces
/// `current` — the file's —, and an option not given keeps it.
[[nodiscard]] std::expected<std::optional<core::Duration>, std::string>
limitFrom(std::string_view option,
          std::string_view text,
          bool allowZero,
          const std::optional<core::Duration>& current) {
    if (text == kOff) {
        return std::nullopt;
    }
    if (text.empty()) {
        return current;
    }
    return parseLimit(option, text, allowZero);
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
    return constraintsOf(options, core::DurationAdjustmentSettings{});
}

std::expected<core::DurationConstraints, std::string>
constraintsOf(const AdjustOptions& options, const core::DurationAdjustmentSettings& base) {
    // The file's constraints, which each option given then replaces.
    core::DurationConstraints constraints = core::constraintsOf(base);

    const auto speed = speedFrom(options, base, constraints.speed);
    if (!speed) {
        return std::unexpected{speed.error()};
    }
    constraints.speed = *speed;

    for (const auto& [option, text, allowZero, target] :
         {std::tuple{"--minimum", &options.minimum, true, &constraints.minimum},
          std::tuple{"--maximum", &options.maximum, false, &constraints.maximum},
          std::tuple{"--gap", &options.gap, true, &constraints.gap}}) {
        const auto limit = limitFrom(option, *text, allowZero, *target);
        if (!limit) {
            return std::unexpected{limit.error()};
        }
        *target = *limit;
    }

    if (!constraints.isAny()) {
        return std::unexpected{
            std::string{core::translate("adjust has nothing to adjust to: every constraint is off, "
                                        "or the reading speed is told to move no end")}};
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
                   const Json& settings,
                   const std::optional<Range>& range,
                   const Destination& destination,
                   const Reporter& reporter) {
    const ChangingOperation adjust = [constraints,
                                      settings](core::Session& session,
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
            .fields = {{"constraints", constraintsRecord(constraints)}, {"settings", settings}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::Adjusted, adjust, range);
}

} // namespace subedit::cli
