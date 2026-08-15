#include <subedit/cli/digits.hpp>

#include <limits>

namespace subedit::cli {

std::optional<std::int64_t> appendedDigit(std::int64_t value, char digit) {
    constexpr std::int64_t kBase = 10;
    constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();

    const std::int64_t added = digit - '0';
    if (value > (kLargest - added) / kBase) {
        return std::nullopt;
    }
    return (value * kBase) + added;
}

} // namespace subedit::cli
