#include <subedit/cli/complaint.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::cli {

namespace {

using Halves = std::pair<std::string_view, std::string_view>;

/// `text` cut at the first `separator`, the separator dropped.
std::optional<Halves> cut(std::string_view text, std::string_view separator) {
    const std::size_t at = text.find(separator);
    if (at == std::string_view::npos) {
        return std::nullopt;
    }
    return Halves{text.substr(0, at), text.substr(at + separator.size())};
}

/// `text` without `suffix` at its end, if it ends with it.
std::optional<std::string_view> before(std::string_view text, std::string_view suffix) {
    if (!text.ends_with(suffix)) {
        return std::nullopt;
    }
    return text.substr(0, text.size() - suffix.size());
}

/// `text` without `prefix` at its start, if it starts with it.
std::optional<std::string_view> after(std::string_view text, std::string_view prefix) {
    if (!text.starts_with(prefix)) {
        return std::nullopt;
    }
    return text.substr(prefix.size());
}

/// The largest `double`, as CLI11 prints the upper bound of `NonNegativeNumber`: a range with
/// that bound has no upper bound worth saying.
constexpr std::string_view kNoUpperBound = "1.79769e+308";

std::optional<std::string> unexpected(std::string_view said) {
    const std::optional<Halves> halves = cut(said, " expected: ");
    if (!halves) {
        return std::nullopt;
    }
    const std::string_view arguments = halves->second;
    // CLI11 joins them with a space; a count is only for the plural.
    const auto count = static_cast<std::uint64_t>(std::ranges::count(arguments, ' ') + 1);
    // TRANSLATORS: %1 is what was typed on the command line and nothing takes, as typed.
    const auto sentence = core::translatePlural("The following argument was not expected: %1",
                                                "The following arguments were not expected: %1",
                                                count);
    return core::substitute(sentence, {arguments});
}

std::optional<std::string> required(std::string_view said) {
    const std::optional<std::string_view> name = before(said, " is required");
    if (!name) {
        return std::nullopt;
    }
    if (*name == "A subcommand") {
        return std::string{core::translate("A subcommand is required")};
    }
    // TRANSLATORS: %1 is an option or an argument, as named in --help.
    const auto sentence = core::translate("%1 is required");
    return core::substitute(sentence, {*name});
}

std::optional<std::string> excludes(std::string_view said) {
    const std::optional<Halves> names = cut(said, " excludes ");
    if (!names) {
        return std::nullopt;
    }
    // TRANSLATORS: %1 and %2 are two options, as typed.
    const auto sentence = core::translate("%1 excludes %2");
    return core::substitute(sentence, {names->first, names->second});
}

std::optional<std::string> count(std::string_view said) {
    const std::optional<Halves> named = cut(said, ": ");
    if (!named) {
        return std::nullopt;
    }
    const auto [name, rest] = *named;
    if (const std::optional<std::string_view> missing = before(rest, " missing"); missing) {
        const std::optional<Halves> typed = cut(*missing, " required ");
        if (!typed) {
            return std::nullopt;
        }
        // TRANSLATORS: %1 is an option given with too few values after it, %2 how many it
        // needs, %3 their metavariable as --help shows it (TEXT, N).
        const auto sentence = core::translate("%1: %2 required %3 missing");
        return core::substitute(sentence, {name, typed->first, typed->second});
    }
    const std::optional<std::string_view> most = after(rest, "At Most ");
    const std::optional<std::string_view> least = after(rest, "At least ");
    const std::optional<std::string_view> bound = most ? most : least;
    const std::optional<Halves> numbers =
        bound ? cut(*bound, " required but received ") : std::nullopt;
    if (!numbers) {
        return std::nullopt;
    }
    if (most) {
        // TRANSLATORS: %1 is an option, %2 the most values it takes, %3 how many were given.
        const auto sentence = core::translate("%1: At Most %2 required but received %3");
        return core::substitute(sentence, {name, numbers->first, numbers->second});
    }
    // TRANSLATORS: %1 is an option, %2 the fewest values it takes, %3 how many were given.
    const auto sentence = core::translate("%1: At least %2 required but received %3");
    return core::substitute(sentence, {name, numbers->first, numbers->second});
}

std::optional<std::string> validation(std::string_view said) {
    const std::optional<Halves> named = cut(said, ": ");
    if (!named) {
        return std::nullopt;
    }
    const auto [name, rest] = *named;

    // IsMember: `<value> not in {a,b,c}`.
    if (const std::optional<Halves> member = cut(rest, " not in {"); member) {
        const std::optional<std::string_view> set = before(member->second, "}");
        if (!set) {
            return std::nullopt;
        }
        // TRANSLATORS: %1 is an option, %2 the value given, %3 the values it takes, between
        // commas.
        const auto sentence = core::translate("%1: %2 not in {%3}");
        return core::substitute(sentence, {name, member->first, *set});
    }

    // Range: `Value <value> not in range [<low> - <high>]`.
    const std::optional<std::string_view> ranged = after(rest, "Value ");
    if (!ranged) {
        return std::nullopt;
    }
    const std::optional<Halves> value = cut(*ranged, " not in range [");
    if (!value) {
        return std::nullopt;
    }
    const std::optional<std::string_view> bounds = before(value->second, "]");
    const std::optional<Halves> ends = bounds ? cut(*bounds, " - ") : std::nullopt;
    if (!ends) {
        return std::nullopt;
    }
    if (ends->first == "0" && ends->second == kNoUpperBound) {
        // TRANSLATORS: %1 is an option, %2 the value given.
        const auto sentence = core::translate("%1: Value %2 is not a number zero or greater");
        return core::substitute(sentence, {name, value->first});
    }
    // TRANSLATORS: %1 is an option, %2 the value given, %3 and %4 the bounds of what it takes.
    const auto sentence = core::translate("%1: Value %2 not in range [%3 - %4]");
    return core::substitute(sentence, {name, value->first, ends->first, ends->second});
}

std::optional<std::string> translated(ComplaintKind kind, std::string_view said) {
    switch (kind) {
    case ComplaintKind::Unexpected:
        return unexpected(said);
    case ComplaintKind::Required:
        return required(said);
    case ComplaintKind::Excludes:
        return excludes(said);
    case ComplaintKind::Count:
        return count(said);
    case ComplaintKind::Validation:
        return validation(said);
    case ComplaintKind::Other:
        break;
    }
    return std::nullopt;
}

} // namespace

std::string complaintIn(ComplaintKind kind, std::string_view said) {
    return translated(kind, said).value_or(std::string{said});
}

std::string helpHint() {
    return std::string{core::translate("Run with --help for more information.")};
}

} // namespace subedit::cli
