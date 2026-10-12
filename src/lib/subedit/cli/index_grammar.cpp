#include <subedit/cli/digits.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/time/duration.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>

namespace subedit::cli {

namespace {

std::string refusal(std::string_view text, std::string_view why) {
    // TRANSLATORS: %1 is what the user typed, %2 why it does not read.
    return core::substitute(core::translate("\"%1\" is not a subtitle number: %2"), {text, why});
}

/// How a reference is written, said once and quoted wherever it is not.
[[nodiscard]] std::string_view shapeOfReference() {
    // TRANSLATORS: <index>=<time> is the form typed, as written.
    return core::translate("write it <index>=<time>, as in 3=00:00:10.000");
}

} // namespace

std::expected<std::size_t, std::string> parseSubtitleNumber(std::string_view text) {
    const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
    if (text.empty() || !std::ranges::all_of(text, isDigit)) {
        return std::unexpected{refusal(
            text,
            core::translate("expected a whole number, counted from 1 as the file shows them"))};
    }

    // Accumulated as a signed integer, and returned as a count: the guard is
    // shared with the two other grammars, and a subtitle number that does not
    // fit in a signed sixty-four bits is not one anyway.
    std::int64_t number = 0;
    for (const char digit : text) {
        const std::optional<std::int64_t> grown = appendedDigit(number, digit);
        if (!grown.has_value()) {
            return std::unexpected{
                refusal(text, core::translate("no subtitle file holds that many subtitles"))};
        }
        number = *grown;
    }

    if (number == 0) {
        return std::unexpected{
            refusal(text, core::translate("subtitles are counted from 1, as the file shows them"))};
    }
    return static_cast<std::size_t>(number);
}

std::expected<Reference, std::string> parseReference(std::string_view text) {
    const std::size_t equals = text.find('=');
    if (equals == std::string_view::npos) {
        // TRANSLATORS: %1 is what the user typed, %2 how a reference is written.
        return std::unexpected{core::substitute(core::translate("\"%1\" is not a reference: %2"),
                                                {text, shapeOfReference()})};
    }

    const std::string_view number = text.substr(0, equals);
    const std::string_view time = text.substr(equals + 1);
    if (number.empty() || time.empty()) {
        return std::unexpected{
            // TRANSLATORS: %1 is what the user typed, %2 how a reference is written.
            core::substitute(core::translate("\"%1\" is missing one of its two halves: %2"),
                             {text, shapeOfReference()})};
    }

    const std::expected<std::size_t, std::string> read = parseSubtitleNumber(number);
    if (!read) {
        return std::unexpected{read.error()};
    }

    const std::expected<core::Duration, std::string> target = parseTime(time);
    if (!target) {
        return std::unexpected{target.error()};
    }

    return Reference{.number = *read,
                     .target = core::Timestamp::fromMilliseconds(target->milliseconds())};
}

std::expected<Range, std::string> parseRange(std::string_view text) {
    const auto shape = [text](std::string_view why) {
        return std::unexpected{
            // TRANSLATORS: %1 is what the user typed, %2 why it does not read.
            core::substitute(core::translate("\"%1\" is not a range: %2"), {text, why})};
    };
    const std::string_view howToWrite =
        // TRANSLATORS: N-M and N- are the forms typed, as written.
        core::translate("write it N-M, or N- to go to the end, counted from 1 and inclusive");

    const std::size_t dash = text.find('-');
    if (dash == std::string_view::npos) {
        return shape(howToWrite);
    }

    // The two halves are subtitle numbers, with the refusals of that grammar: a
    // sign, a zero or a decimal point is said there, in its own words.
    const std::expected<std::size_t, std::string> first = parseSubtitleNumber(text.substr(0, dash));
    if (!first) {
        return std::unexpected{first.error()};
    }

    const std::string_view rest = text.substr(dash + 1);
    if (rest.empty()) {
        return Range{.first = *first, .last = std::nullopt};
    }

    const std::expected<std::size_t, std::string> last = parseSubtitleNumber(rest);
    if (!last) {
        return std::unexpected{last.error()};
    }
    if (*last < *first) {
        return shape(core::translate("it ends before it starts"));
    }
    return Range{.first = *first, .last = *last};
}

std::expected<core::Selection, std::string> selectionOf(const Range& range, std::size_t count) {
    const std::string written =
        std::to_string(range.first) + "-" + (range.last ? std::to_string(*range.last) : "");
    const std::string held = std::to_string(count);

    if (range.first > count) {
        return std::unexpected{core::substitute(
            // TRANSLATORS: %1 is the range typed, %2 how many subtitles the file has. English
            // writes the number alone; the form is chosen by %2.
            core::translatePlural("range %1 starts after the last subtitle: the file has %2",
                                  "range %1 starts after the last subtitle: the file has %2",
                                  count),
            {written, held})};
    }
    if (range.last && *range.last > count) {
        return std::unexpected{core::substitute(
            // TRANSLATORS: %1 is the range typed, %2 how many subtitles the file has.
            core::translatePlural("range %1 ends after the last subtitle: the file has %2",
                                  "range %1 ends after the last subtitle: the file has %2",
                                  count),
            {written, held})};
    }

    return core::Selection::range(core::SubtitleIndex::fromNumber(range.first),
                                  core::SubtitleIndex::fromNumber(range.last.value_or(count)));
}

} // namespace subedit::cli
