#include <subedit/cli/digits.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/time/duration.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>

namespace subedit::cli {

namespace {

std::string refusal(std::string_view text, std::string_view why) {
    return "\"" + std::string{text} + "\" is not a subtitle number: " + std::string{why};
}

/// How a reference is written, said once and quoted wherever it is not.
constexpr std::string_view kShape = "write it <index>=<time>, as in 3=00:00:10.000";

} // namespace

std::expected<std::size_t, std::string> parseSubtitleNumber(std::string_view text) {
    const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
    if (text.empty() || !std::ranges::all_of(text, isDigit)) {
        return std::unexpected{
            refusal(text, "expected a whole number, counted from 1 as the file shows them")};
    }

    // Accumulated as a signed integer, and returned as a count: the guard is
    // shared with the two other grammars, and a subtitle number that does not
    // fit in a signed sixty-four bits is not one anyway.
    std::int64_t number = 0;
    for (const char digit : text) {
        const std::optional<std::int64_t> grown = appendedDigit(number, digit);
        if (!grown.has_value()) {
            return std::unexpected{refusal(text, "no subtitle file holds that many subtitles")};
        }
        number = *grown;
    }

    if (number == 0) {
        return std::unexpected{
            refusal(text, "subtitles are counted from 1, as the file shows them")};
    }
    return static_cast<std::size_t>(number);
}

std::expected<Reference, std::string> parseReference(std::string_view text) {
    const std::size_t equals = text.find('=');
    if (equals == std::string_view::npos) {
        return std::unexpected{"\"" + std::string{text} +
                               "\" is not a reference: " + std::string{kShape}};
    }

    const std::string_view number = text.substr(0, equals);
    const std::string_view time = text.substr(equals + 1);
    if (number.empty() || time.empty()) {
        return std::unexpected{"\"" + std::string{text} +
                               "\" is missing one of its two halves: " + std::string{kShape}};
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

} // namespace subedit::cli
