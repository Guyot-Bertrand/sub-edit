#include <subedit/core/text/letter_case.hpp>
#include <subedit/core/text/markup_parser.hpp>

#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <unicode/utf16.h>
#include <unicode/utypes.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Tells whether `unit` opens the part of a text the case applies to.
///
/// `u_isalnum` over the whole plane rather than an ASCII test: the first letter
/// of a Greek or Cyrillic subtitle is a letter.
[[nodiscard]] bool isLetterOrDigit(UChar32 unit) {
    return u_isalnum(unit) != 0;
}

/// Where the part of a text that the case applies to begins.
///
/// **It walks over punctuation and spaces, and stops at the first of anything
/// else** — including the first half of a letter outside the basic plane. Such
/// a letter is two units and `u_isalnum` says nothing useful of either half;
/// stopping there is what keeps the body whole, where skipping the pair would
/// leave the letter out of the case.
[[nodiscard]] std::int32_t firstLetterOrDigit(const std::vector<UChar>& wide, std::int32_t length) {
    std::int32_t at = 0;
    while (at < length && !U16_IS_SURROGATE(wide[static_cast<std::size_t>(at)]) &&
           !isLetterOrDigit(wide[static_cast<std::size_t>(at)]))
        ++at;
    return at;
}

/// `text` as UTF-16, which is what ICU works in.
///
/// **No failure to handle, and the two reasons are worth writing.** A preflight
/// says exactly how much room is needed, so the second call cannot overflow;
/// and the bytes come from a decoder that produced UTF-8, so there is nothing
/// here for ICU to refuse.
[[nodiscard]] std::vector<UChar> widened(std::string_view text) {
    UErrorCode status = U_ZERO_ERROR;
    std::int32_t wanted = 0;
    u_strFromUTF8(
        nullptr, 0, &wanted, text.data(), static_cast<std::int32_t>(text.size()), &status);

    std::vector<UChar> wide(static_cast<std::size_t>(wanted) + 1);
    status = U_ZERO_ERROR;
    u_strFromUTF8(wide.data(),
                  wanted + 1,
                  nullptr,
                  text.data(),
                  static_cast<std::int32_t>(text.size()),
                  &status);
    wide.resize(static_cast<std::size_t>(wanted));
    return wide;
}

/// UTF-16 back to the UTF-8 the model holds.
[[nodiscard]] std::string narrowed(const std::vector<UChar>& wide) {
    UErrorCode status = U_ZERO_ERROR;
    std::int32_t wanted = 0;
    u_strToUTF8(nullptr, 0, &wanted, wide.data(), static_cast<std::int32_t>(wide.size()), &status);

    std::string out(static_cast<std::size_t>(wanted), '\0');
    status = U_ZERO_ERROR;
    u_strToUTF8(
        out.data(), wanted, nullptr, wide.data(), static_cast<std::int32_t>(wide.size()), &status);
    return out;
}

/// One ICU mapping, asked how much room it wants before it is given any.
///
/// **A case mapping can grow** — `ß` becomes `SS` — so the size is never
/// guessed. The first call writes nothing and answers the length; the second
/// cannot overflow, which is why there is no failure to handle.
template<typename Map>
[[nodiscard]] std::vector<UChar> mapped(const std::vector<UChar>& wide, Map map) {
    const auto given = static_cast<std::int32_t>(wide.size());

    UErrorCode status = U_ZERO_ERROR;
    const std::int32_t wanted = map(nullptr, 0, wide.data(), given, &status);

    std::vector<UChar> out(static_cast<std::size_t>(wanted) + 1);
    status = U_ZERO_ERROR;
    map(out.data(), wanted + 1, wide.data(), given, &status);

    out.resize(static_cast<std::size_t>(wanted));
    return out;
}

[[nodiscard]] std::vector<UChar> uppered(const std::vector<UChar>& wide) {
    return mapped(
        wide,
        [](UChar* out, std::int32_t room, const UChar* in, std::int32_t given, UErrorCode* status) {
            return u_strToUpper(out, room, in, given, nullptr, status);
        });
}

[[nodiscard]] std::vector<UChar> lowered(const std::vector<UChar>& wide) {
    return mapped(
        wide,
        [](UChar* out, std::int32_t room, const UChar* in, std::int32_t given, UErrorCode* status) {
            return u_strToLower(out, room, in, given, nullptr, status);
        });
}

/// Every word takes a capital, ICU deciding where a word begins.
///
/// **Where Python's `title()` would capitalise after an apostrophe**, turning
/// the French for « the summer » into a word with a capital in its middle. ICU
/// asks a word breaker instead, and a breaker knows better.
[[nodiscard]] std::vector<UChar> titled(const std::vector<UChar>& wide) {
    return mapped(
        wide,
        [](UChar* out, std::int32_t room, const UChar* in, std::int32_t given, UErrorCode* status) {
            return u_strToTitle(out, room, in, given, nullptr, nullptr, status);
        });
}

/// The whole text in small letters, its first letter capitalised.
///
/// Its caller has already found a letter, so there is always a first one.
[[nodiscard]] std::vector<UChar> sentenced(const std::vector<UChar>& wide) {
    std::vector<UChar> small = lowered(wide);
    const std::vector<UChar> head{small.front()};
    std::vector<UChar> capital = uppered(head);
    // **A letter that grows on being capitalised keeps its small form**, and
    // the German `ß` is the one that does: its capital is `SS`, two letters
    // where there was one. Writing them would be a defensible answer and a
    // surprising one; leaving the letter alone is what Gaupol does too.
    if (capital.size() != 1)
        return small;
    small.front() = capital.front();
    return small;
}

/// The four mappings, on a text with no tag left in it.
[[nodiscard]] std::string inCase(std::string_view text, LetterCase wanted) {
    const std::vector<UChar> wide = widened(text);

    // What precedes the first letter or digit is not the text's business.
    const auto length = static_cast<std::int32_t>(wide.size());
    const std::int32_t opening = firstLetterOrDigit(wide, length);
    if (opening >= length)
        return std::string{text};

    const std::vector<UChar> prefix(wide.begin(), wide.begin() + opening);
    const std::vector<UChar> body(wide.begin() + opening, wide.end());

    std::vector<UChar> mappedBody;
    switch (wanted) {
    case LetterCase::Title:
        mappedBody = titled(body);
        break;
    case LetterCase::Sentence:
        mappedBody = sentenced(body);
        break;
    case LetterCase::Upper:
        mappedBody = uppered(body);
        break;
    case LetterCase::Lower:
        mappedBody = lowered(body);
        break;
    }

    std::vector<UChar> whole = prefix;
    whole.insert(whole.end(), mappedBody.begin(), mappedBody.end());
    return narrowed(whole);
}

} // namespace

std::string recased(std::string_view text, LetterCase wanted, SubtitleFormat format) {
    MarkupParser parser{text, format};
    const std::string changed = inCase(parser.visible(), wanted);
    if (changed == parser.visible())
        return std::string{text};

    parser.transform(0, parser.visible().size(), changed);
    return parser.text();
}

} // namespace subedit::core
