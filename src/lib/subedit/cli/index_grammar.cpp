#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/time/duration.hpp>

#include <algorithm>
#include <expected>
#include <limits>

namespace subedit::cli {

namespace {

constexpr std::size_t kBase = 10;

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

    constexpr std::size_t kLargest = std::numeric_limits<std::size_t>::max();
    std::size_t number = 0;
    for (const char digit : text) {
        const auto added = static_cast<std::size_t>(digit - '0');
        // Checked before it happens rather than detected after: an overflowed
        // count would name some other subtitle, which is worse than a refusal.
        if (number > (kLargest - added) / kBase) {
            return std::unexpected{refusal(text, "no subtitle file holds that many subtitles")};
        }
        number = number * kBase + added;
    }

    if (number == 0) {
        return std::unexpected{
            refusal(text, "subtitles are counted from 1, as the file shows them")};
    }
    return number;
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
