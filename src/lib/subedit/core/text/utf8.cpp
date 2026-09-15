#include <subedit/core/text/utf8.hpp>

namespace subedit::core {

namespace {

/// Tells whether `byte` continues a sequence rather than starting one.
[[nodiscard]] bool continues(char byte) {
    constexpr unsigned kContinuationMask = 0xC0U;
    constexpr unsigned kContinuation = 0x80U;
    return (static_cast<unsigned char>(byte) & kContinuationMask) == kContinuation;
}

} // namespace

std::size_t previousCodePoint(std::string_view text, std::size_t at) {
    --at;
    while (at > 0 && continues(text[at]))
        --at;
    return at;
}

std::size_t nextCodePoint(std::string_view text, std::size_t at) {
    ++at;
    while (at < text.size() && continues(text[at]))
        ++at;
    return at;
}

char32_t codePointAt(std::string_view text, std::size_t at) {
    constexpr unsigned kPayload = 0x3FU;
    constexpr unsigned kAsciiMask = 0x7FU;
    constexpr unsigned kBitsPerContinuation = 6U;

    const std::size_t end = nextCodePoint(text, at);
    const std::size_t length = end - at;
    // A lead byte keeps `7 - length` bits of its own in a sequence of `length`
    // bytes, and all seven when it stands alone.
    const unsigned leadMask = length == 1 ? kAsciiMask : (kAsciiMask >> length);
    unsigned point = static_cast<unsigned char>(text[at]) & leadMask;
    for (std::size_t next = at + 1; next < end; ++next)
        point =
            (point << kBitsPerContinuation) | (static_cast<unsigned char>(text[next]) & kPayload);
    return static_cast<char32_t>(point);
}

} // namespace subedit::core
