#include <subedit/core/model/source_file.hpp>
#include <subedit/core/text/encoding.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::string_view kUtf8Bom = "\xEF\xBB\xBF";

/// A continuation byte is the only one shaped `10xxxxxx`.
constexpr std::uint8_t kContinuationMask = 0xC0;
constexpr std::uint8_t kContinuationValue = 0x80;
constexpr std::uint8_t kContinuationPayload = 0x3F;
constexpr unsigned kContinuationBits = 6;

/// Below this, a byte is a character of its own — plain ASCII.
constexpr std::uint8_t kAsciiLimit = 0x80;

/// The high bits of a lead byte say how many bytes the character takes:
/// `110xxxxx` two, `1110xxxx` three, `11110xxx` four.
constexpr std::uint8_t kTwoByteMask = 0xE0;
constexpr std::uint8_t kTwoByteLead = 0xC0;
constexpr std::uint8_t kThreeByteMask = 0xF0;
constexpr std::uint8_t kThreeByteLead = 0xE0;
constexpr std::uint8_t kFourByteMask = 0xF8;
constexpr std::uint8_t kFourByteLead = 0xF0;

/// All eight bits, shifted to keep only the payload of a lead byte.
constexpr std::uint32_t kAllBits = 0xFF;

/// The highest code point Unicode defines.
constexpr std::uint32_t kMaxCodePoint = 0x10FFFF;

/// The range reserved for UTF-16 surrogate halves, which UTF-8 never encodes.
constexpr std::uint32_t kFirstSurrogate = 0xD800;
constexpr std::uint32_t kLastSurrogate = 0xDFFF;

[[nodiscard]] constexpr std::uint8_t byteAt(std::string_view bytes, std::size_t index) {
    return static_cast<std::uint8_t>(bytes[index]);
}

[[nodiscard]] constexpr bool isContinuation(std::uint8_t byte) {
    return (byte & kContinuationMask) == kContinuationValue;
}

/// How many bytes the character starting with `lead` occupies, `0` if it
/// cannot start one.
[[nodiscard]] constexpr std::size_t lengthOf(std::uint8_t lead) {
    if (lead < kAsciiLimit)
        return 1;
    if ((lead & kTwoByteMask) == kTwoByteLead)
        return 2;
    if ((lead & kThreeByteMask) == kThreeByteLead)
        return 3;
    if ((lead & kFourByteMask) == kFourByteLead)
        return 4;
    return 0;
}

/// The smallest code point each length is allowed to encode.
///
/// Encoding a character on more bytes than it needs used to be tolerated, and
/// is how a check on a shorter form gets slipped past.
[[nodiscard]] constexpr std::uint32_t smallestFor(std::size_t length) {
    constexpr std::array<std::uint32_t, 5> kSmallest = {0, 0, 0x80, 0x800, 0x10000};
    return kSmallest.at(length);
}

} // namespace

bool isValidUtf8(std::string_view bytes) {
    std::size_t index = 0;
    while (index < bytes.size()) {
        const std::uint8_t lead = byteAt(bytes, index);
        const std::size_t length = lengthOf(lead);
        if (length == 0 || index + length > bytes.size())
            return false;

        std::uint32_t codePoint = lead & (kAllBits >> (length + 1));
        if (length == 1)
            codePoint = lead;

        for (std::size_t offset = 1; offset < length; ++offset) {
            const std::uint8_t next = byteAt(bytes, index + offset);
            if (!isContinuation(next))
                return false;
            codePoint = (codePoint << kContinuationBits) | (next & kContinuationPayload);
        }

        if (codePoint < smallestFor(length))
            return false;
        if (codePoint > kMaxCodePoint)
            return false;
        if (codePoint >= kFirstSurrogate && codePoint <= kLastSurrogate)
            return false;

        index += length;
    }
    return true;
}

bool hasUtf8Bom(std::string_view bytes) {
    return bytes.starts_with(kUtf8Bom);
}

std::string_view withoutUtf8Bom(std::string_view bytes) {
    if (!hasUtf8Bom(bytes))
        return bytes;
    return bytes.substr(kUtf8Bom.size());
}

NewlineScan scanNewlines(std::string_view content) {
    int carriageReturnLineFeed = 0;
    int lineFeed = 0;
    int carriageReturn = 0;

    NewlineScan scan;
    Newline first = Newline::Lf;
    bool seenAny = false;
    int line = 0;

    for (std::size_t index = 0; index < content.size(); ++index) {
        Newline kind = Newline::Lf;
        if (content[index] == '\r') {
            const bool followed = index + 1 < content.size() && content[index + 1] == '\n';
            kind = followed ? Newline::CrLf : Newline::Cr;
            if (followed)
                ++index;
        } else if (content[index] != '\n') {
            continue;
        }

        ++line;
        switch (kind) {
        case Newline::CrLf:
            ++carriageReturnLineFeed;
            break;
        case Newline::Lf:
            ++lineFeed;
            break;
        case Newline::Cr:
            ++carriageReturn;
            break;
        }

        if (!seenAny) {
            first = kind;
            seenAny = true;
        } else if (kind != first && !scan.mixed) {
            scan.mixed = true;
            scan.mixedAtLine = line;
        }
    }

    if (!seenAny)
        return scan;

    scan.newline = Newline::Lf;
    if (carriageReturnLineFeed >= lineFeed && carriageReturnLineFeed >= carriageReturn)
        scan.newline = Newline::CrLf;
    else if (carriageReturn > lineFeed)
        scan.newline = Newline::Cr;

    return scan;
}

} // namespace subedit::core
