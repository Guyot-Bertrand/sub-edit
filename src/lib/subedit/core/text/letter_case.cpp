#include <subedit/core/text/letter_case.hpp>
#include <subedit/core/text/markup_parser.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/bytestream.h>
#include <unicode/casemap.h>
#include <unicode/edits.h>
#include <unicode/stringpiece.h>
#include <unicode/uchar.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

/// The root locale, which ICU names with an empty string.
///
/// **Never the locale of the process** — issue #401. Handed `nullptr`, ICU
/// cases the way the environment says, and under `LANG=tr_TR` `istanbul` in
/// capitals came out `İSTANBUL`: one document, one menu entry, two answers on
/// two machines. Casing a subtitle by its own language is a question for the
/// internationalisation phase, not an accident of whoever runs the program.
constexpr const char* kRootLocale = "";

/// A text ICU has mapped, and the record of which bytes became which.
struct Mapped {
    std::string text;
    icu::Edits edits;
};

/// One ICU mapping over UTF-8, its edits kept.
///
/// **A case mapping changes the length of a letter** — `ß` grows into `SS`,
/// `ﬁ` into `FI`, `ſ` shrinks into `S` — so an offset into the source is not an
/// offset into the result. The edits are the only record of where each one
/// went, and the tags need exactly that.
///
/// No failure to handle: the bytes come from a decoder that produced UTF-8, and
/// the sink grows as it is written.
template<typename Map>
[[nodiscard]] Mapped mappedBy(std::string_view text, Map map) {
    Mapped out;
    icu::StringByteSink<std::string> sink{&out.text};
    UErrorCode status = U_ZERO_ERROR;
    map(icu::StringPiece{text.data(), static_cast<std::int32_t>(text.size())},
        sink,
        &out.edits,
        status);
    return out;
}

[[nodiscard]] Mapped uppered(std::string_view text) {
    return mappedBy(
        text, [](icu::StringPiece in, icu::ByteSink& sink, icu::Edits* edits, UErrorCode& status) {
            icu::CaseMap::utf8ToUpper(kRootLocale, 0, in, sink, edits, status);
        });
}

[[nodiscard]] Mapped lowered(std::string_view text) {
    return mappedBy(
        text, [](icu::StringPiece in, icu::ByteSink& sink, icu::Edits* edits, UErrorCode& status) {
            icu::CaseMap::utf8ToLower(kRootLocale, 0, in, sink, edits, status);
        });
}

/// Every word takes a capital, ICU deciding where a word begins.
///
/// **Where Python's `title()` would capitalise after an apostrophe**, turning
/// the French for « the summer » into a word with a capital in its middle. ICU
/// asks a word breaker instead, and a breaker knows better.
[[nodiscard]] Mapped titled(std::string_view text) {
    return mappedBy(
        text, [](icu::StringPiece in, icu::ByteSink& sink, icu::Edits* edits, UErrorCode& status) {
            icu::CaseMap::utf8ToTitle(kRootLocale, 0, nullptr, in, sink, edits, status);
        });
}

/// The whole text in small letters, its first letter capitalised.
///
/// Its caller has already found a letter, so there is always a first one.
[[nodiscard]] Mapped sentenced(std::string_view text) {
    Mapped small = lowered(text);
    const std::size_t head = nextCodePoint(small.text, 0);
    const Mapped capital = uppered(std::string_view{small.text}.substr(0, head));

    // What the second step did, written as edits so the two compose: the head
    // replaced by its capital, or left, and the rest untouched.
    icu::Edits second;
    const auto rest = static_cast<std::int32_t>(small.text.size() - head);
    // **A letter that grows on being capitalised keeps its small form**, and
    // the German `ß` is the one that does: its capital is `SS`, two letters
    // where there was one. Writing them would be defensible and surprising.
    // Gaupol writes `Ss` — Python's `capitalize` titlecases the first letter —
    // and the inventory of Gaupol records the difference.
    if (nextCodePoint(capital.text, 0) == capital.text.size()) {
        second.addReplace(static_cast<std::int32_t>(head),
                          static_cast<std::int32_t>(capital.text.size()));
        small.text.replace(0, head, capital.text);
    } else {
        second.addUnchanged(static_cast<std::int32_t>(head));
    }
    second.addUnchanged(rest);

    Mapped whole{.text = std::move(small.text), .edits = {}};
    UErrorCode status = U_ZERO_ERROR;
    whole.edits.mergeAndAppend(small.edits, second, status);
    return whole;
}

/// Where the part of a text that the case applies to begins.
///
/// It walks over punctuation and spaces, one code point at a time, and stops
/// at the first letter or digit — `u_isalnum` over the whole of Unicode, so the
/// first letter of a Greek or Cyrillic subtitle is a letter.
[[nodiscard]] std::size_t firstLetterOrDigit(std::string_view text) {
    std::size_t at = 0;
    while (at < text.size() && u_isalnum(static_cast<UChar32>(codePointAt(text, at))) == 0)
        at = nextCodePoint(text, at);
    return at;
}

/// A text re-cased, and where each offset of the text it came from went.
struct Recased {
    std::string text;
    std::function<std::size_t(std::size_t)> placed;
};

/// The four mappings, on a text with no tag left in it.
[[nodiscard]] Recased inCase(std::string_view text, LetterCase wanted) {
    // What precedes the first letter or digit is not the text's business.
    const std::size_t opening = firstLetterOrDigit(text);
    if (opening >= text.size())
        return Recased{.text = std::string{text},
                       .placed = [](std::size_t offset) { return offset; }};

    const std::string_view body = text.substr(opening);
    Mapped mapped;
    switch (wanted) {
    case LetterCase::Title:
        mapped = titled(body);
        break;
    case LetterCase::Sentence:
        mapped = sentenced(body);
        break;
    case LetterCase::Upper:
        mapped = uppered(body);
        break;
    case LetterCase::Lower:
        mapped = lowered(body);
        break;
    }

    std::string whole = std::string{text.substr(0, opening)} + mapped.text;
    return Recased{
        .text = std::move(whole),
        .placed = [edits = std::move(mapped.edits), opening](std::size_t offset) -> std::size_t {
            if (offset <= opening)
                return offset;
            // A tag sits at the start of a code point, so its offset is the
            // start or the end of an edit, never inside one.
            icu::Edits::Iterator iterator = edits.getFineIterator();
            UErrorCode status = U_ZERO_ERROR;
            const std::int32_t moved = iterator.destinationIndexFromSourceIndex(
                static_cast<std::int32_t>(offset - opening), status);
            return opening + static_cast<std::size_t>(moved);
        }};
}

} // namespace

std::string recased(std::string_view text, LetterCase wanted, SubtitleFormat format) {
    MarkupParser parser{text, format};
    const Recased changed = inCase(parser.visible(), wanted);
    if (changed.text == parser.visible())
        return std::string{text};

    // Rewritten whole and not at one offset: a letter that changes length
    // would otherwise leave every tag after it at its old byte offset — inside
    // a letter, or a letter away from its word.
    parser.rewrite(changed.text, changed.placed);
    return parser.text();
}

} // namespace subedit::core
