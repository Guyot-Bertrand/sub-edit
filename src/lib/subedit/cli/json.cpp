#include <subedit/cli/json.hpp>

#include <cstddef>
#include <cstdint>

namespace subedit::cli {

namespace {

constexpr std::string_view kReplacement = "\xEF\xBF\xBD"; // U+FFFD

// The shape of UTF-8, named once: a lead byte announces how many bytes the
// character takes, and each continuation byte carries six more bits.
constexpr unsigned char kAsciiEnd = 0x80;
constexpr unsigned char kContinuationMask = 0xC0;
constexpr unsigned char kContinuationTag = 0x80;
constexpr unsigned kContinuationBits = 6;
constexpr unsigned kContinuationPayload = 0x3F;
constexpr std::uint32_t kLastPoint = 0x10FFFF;
constexpr std::uint32_t kFirstSurrogate = 0xD800;
constexpr std::uint32_t kLastSurrogate = 0xDFFF;

/// One length of sequence: the lead bytes that announce it, and what it must hold.
struct Lead {
    unsigned char first;
    unsigned char last;
    std::size_t length;
    unsigned char payload; ///< the bits of the lead byte that belong to the character
    std::uint32_t least;   ///< below this, a shorter sequence should have been used
};

constexpr Lead kTwoBytes{.first = 0xC2, .last = 0xDF, .length = 2, .payload = 0x1F, .least = 0x80};
constexpr Lead kThreeBytes{
    .first = 0xE0, .last = 0xEF, .length = 3, .payload = 0x0F, .least = 0x800};
constexpr Lead kFourBytes{
    .first = 0xF0, .last = 0xF4, .length = 4, .payload = 0x07, .least = 0x10000};

[[nodiscard]] const Lead* leadFor(unsigned char byte) {
    for (const Lead* lead : {&kTwoBytes, &kThreeBytes, &kFourBytes}) {
        if (byte >= lead->first && byte <= lead->last) {
            return lead;
        }
    }
    return nullptr;
}

/// How many bytes the sequence starting at `at` takes if it is valid UTF-8, or
/// zero. Overlong forms, surrogates and what lies past U+10FFFF are invalid.
[[nodiscard]] std::size_t validSequenceAt(std::string_view text, std::size_t at) {
    const auto byte = [&](std::size_t index) { return static_cast<unsigned char>(text[index]); };
    const unsigned char first = byte(at);
    if (first < kAsciiEnd) {
        return 1;
    }

    const Lead* lead = leadFor(first);
    if (lead == nullptr || at + lead->length > text.size()) {
        return 0;
    }

    std::uint32_t point = first & lead->payload;
    for (std::size_t i = 1; i < lead->length; ++i) {
        const unsigned char next = byte(at + i);
        if ((next & kContinuationMask) != kContinuationTag) {
            return 0;
        }
        point = (point << kContinuationBits) | (next & kContinuationPayload);
    }
    const bool surrogate = point >= kFirstSurrogate && point <= kLastSurrogate;
    if (point < lead->least || point > kLastPoint || surrogate) {
        return 0;
    }
    return lead->length;
}

constexpr unsigned char kFirstPrintable = 0x20;
constexpr unsigned kNibbleBits = 4;
constexpr unsigned kNibbleMask = 0x0F;

void appendEscaped(std::string& out, std::string_view text) {
    constexpr std::string_view kHex = "0123456789abcdef";
    out += '"';
    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (byte < kFirstPrintable) {
                out += "\\u00";
                out += kHex[byte >> kNibbleBits];
                out += kHex[byte & kNibbleMask];
            } else {
                out += c;
            }
        }
    }
    out += '"';
}

} // namespace

CleanedText cleanedUtf8(std::string_view text) {
    CleanedText cleaned;
    cleaned.text.reserve(text.size());
    for (std::size_t at = 0; at < text.size();) {
        if (const std::size_t length = validSequenceAt(text, at); length != 0) {
            cleaned.text.append(text.substr(at, length));
            at += length;
        } else {
            cleaned.text.append(kReplacement);
            cleaned.replaced = true;
            ++at;
        }
    }
    return cleaned;
}

Json& Json::push(Json value) {
    std::get<Array>(m_value).push_back(std::move(value));
    return *this;
}

Json& Json::set(std::string key, Json value) {
    std::get<Object>(m_value).emplace_back(std::move(key), std::move(value));
    return *this;
}

std::string Json::dump() const {
    std::string out;
    if (std::holds_alternative<std::nullptr_t>(m_value)) {
        out = "null";
    } else if (const auto* flag = std::get_if<bool>(&m_value)) {
        out = *flag ? "true" : "false";
    } else if (const auto* number = std::get_if<std::int64_t>(&m_value)) {
        out = std::to_string(*number);
    } else if (const auto* text = std::get_if<std::string>(&m_value)) {
        appendEscaped(out, *text);
    } else if (const auto* array = std::get_if<Array>(&m_value)) {
        out = "[";
        for (std::size_t i = 0; i < array->size(); ++i) {
            if (i != 0) {
                out += ',';
            }
            out += (*array)[i].dump();
        }
        out += ']';
    } else {
        const auto& object = std::get<Object>(m_value);
        out = "{";
        for (std::size_t i = 0; i < object.size(); ++i) {
            if (i != 0) {
                out += ',';
            }
            appendEscaped(out, object[i].first);
            out += ':';
            out += object[i].second.dump();
        }
        out += '}';
    }
    return out;
}

} // namespace subedit::cli
