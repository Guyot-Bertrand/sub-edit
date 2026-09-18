#include <subedit/core/text/markup_parser.hpp>
#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/uchar.h>
#include <unicode/umachine.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Tells whether `point` belongs to a word: a letter, a digit, or a mark.
///
/// **A combining mark is part of the letter it follows.** A decomposed `é` is
/// an `e` and an accent, and a tag between the two cuts the word as surely as
/// one between two letters; so does one inside a Devanagari syllable.
[[nodiscard]] bool isWordCharacter(UChar32 point) {
    if (u_isalnum(point) != 0)
        return true;
    const auto type = static_cast<UCharCategory>(u_charType(point));
    return type == U_NON_SPACING_MARK || type == U_COMBINING_SPACING_MARK ||
           type == U_ENCLOSING_MARK;
}

/// Tells whether the code point ending at `at` and the one starting there both
/// belong to a word — that is, whether a tag at `at` cuts a word in two.
///
/// **Code points, and letters, digits or marks only** — issue #402. Counting every
/// byte above the ASCII range as a letter kept accented letters whole, and also
/// made letters of a no-break space, of `«`, `»` and `’`: an italic next to
/// French punctuation swallowed it. `u_isalnum` asks the question that was
/// meant.
[[nodiscard]] bool cutsWord(std::string_view text, std::size_t at) {
    if (at == 0 || at >= text.size())
        return false;

    return isWordCharacter(static_cast<UChar32>(codePointAt(text, previousCodePoint(text, at)))) &&
           isWordCharacter(static_cast<UChar32>(codePointAt(text, at)));
}

/// What one tag does: opens a style, closes one, or neither.
enum class TagKind { Opens, Closes, Opaque };

/// One tag as it was found: its bytes, what it does, and to what.
struct Tag {
    std::string text;
    TagKind kind = TagKind::Opaque;
    /// The styles it opens or closes, named so an opener finds its closer.
    std::vector<std::string> styles;
    /// Where it sat, as an offset into the visible text.
    std::size_t at = 0;
};

// ---------------------------------------------------------------------------
// Reading the tags out of a text
// ---------------------------------------------------------------------------

/// Reads one HTML tag, brackets included.
[[nodiscard]] Tag htmlTag(std::string_view text) {
    Tag tag;
    HtmlTag read = htmlTagOf(text);
    if (read.name.empty())
        return tag;

    tag.kind = read.closing ? TagKind::Closes : TagKind::Opens;
    tag.styles.push_back(std::move(read.name));
    return tag;
}

/// Reads one Sub Station Alpha block, braces included.
///
/// **A block says as many things as it likes**, and `{\b1\i1}` says two. Only
/// the three flags pair up; a colour, a font, a position or a comment has no
/// closer, so a block carrying any of them is carried whole and never moved.
[[nodiscard]] Tag subStationAlphaTag(std::string_view text) {
    Tag tag;
    const std::optional<std::vector<std::string_view>> overrides = overridesOf(text);
    if (!overrides.has_value())
        return tag;

    std::vector<std::string> opened;
    std::vector<std::string> closed;
    for (const std::string_view override : *overrides) {
        const std::optional<FlagOverride> flag = flagOverrideOf(override);
        if (!flag.has_value())
            return tag;
        (flag->on ? opened : closed).push_back(std::string{flag->letter});
    }

    if (opened.empty() == closed.empty())
        return tag; // neither, or both — carried whole
    tag.kind = opened.empty() ? TagKind::Closes : TagKind::Opens;
    tag.styles = opened.empty() ? std::move(closed) : std::move(opened);
    return tag;
}

/// Everything a braced vocabulary writes is carried whole.
///
/// MicroDVD and MPL2 open a style and never close it — `{Y:i}` reaches the end
/// of the subtitle, `/` the end of its line. A tag with no closer has no span
/// to widen, so it is exactly what an opaque tag is: something that sits where
/// it sits.
[[nodiscard]] Tag opaqueTag() {
    return Tag{};
}

/// Reads one bracketed or braced tag, in the vocabulary that wrote it.
[[nodiscard]] Tag tagOf(std::string_view text, MarkupVocabulary vocabulary) {
    switch (vocabulary) {
    case MarkupVocabulary::Html:
        return htmlTag(text);
    case MarkupVocabulary::SubStationAlpha:
        return subStationAlphaTag(text);
    case MarkupVocabulary::None:
    case MarkupVocabulary::MicroDvd:
    case MarkupVocabulary::Mpl2:
        return opaqueTag();
    }
    // The five are handled and the compiler checks it; only a cast from an
    // out-of-range integer could land here.
    std::unreachable();
}

/// The tags of `text` in `vocabulary`, and the visible text left over.
struct Reading {
    std::string visible;
    std::vector<Tag> tags;
};

/// Reads `text` through the one reader of tags — issue #403.
///
/// Where a tag starts and ends, and whether an MPL2 marker is one, is the
/// reader's to say; what a tag opens or closes is this parser's.
[[nodiscard]] Reading read(std::string_view text, MarkupVocabulary vocabulary) {
    Reading found;
    found.visible.reserve(text.size());

    for (const MarkupPiece& piece : piecesOf(text, vocabulary)) {
        if (piece.kind == MarkupPiece::Kind::Text) {
            found.visible += piece.text;
            continue;
        }
        // A marker opens a style it never closes, as a MicroDVD tag does.
        Tag tag =
            piece.kind == MarkupPiece::Kind::Marker ? opaqueTag() : tagOf(piece.text, vocabulary);
        tag.text = std::string{piece.text};
        tag.at = found.visible.size();
        found.tags.push_back(std::move(tag));
    }
    return found;
}

// ---------------------------------------------------------------------------
// What a text holds once read
// ---------------------------------------------------------------------------

/// One stretch of text a tag pair covers.
struct Span {
    std::string opening;
    /// Absent for a tag nobody closed, which stays unclosed.
    std::optional<std::string> closing{};
    std::string style;
    std::size_t first = 0;
    std::size_t last = 0;
    /// The number of the opening tag that wrote it, so that a block opening two
    /// styles is written once rather than twice.
    std::size_t writer = 0;
    /// The number of the closing tag, for the same reason: `{\b0\i0}` shuts
    /// two spans and is one tag.
    std::size_t closer = 0;
    /// Whether the replacement under way reached it. **Only a span a
    /// replacement reaches may move or merge**: the rest of the subtitle keeps
    /// its tags exactly where the file had them.
    bool reached = false;
};

/// A tag that opens nothing and closes nothing, and never moves on its own.
struct Loose {
    std::string text;
    std::size_t at = 0;
};

} // namespace

struct MarkupParser::Held {
    std::string original;
    std::string visible;
    std::vector<Span> spans;
    std::vector<Loose> loose;
    MarkupVocabulary vocabulary = MarkupVocabulary::None;
    bool touched = false;
    /// How many tags have been read, the text's and every replacement's.
    ///
    /// **What numbers a tag.** Two spans written by one tag must say so, and a
    /// number given by position in one reading would collide with the same
    /// position in the next.
    std::size_t tagsRead = 0;

    /// Takes in the tags a replacement carried, inside the stretch it fills.
    ///
    /// They land after the inherited ones, which is what makes a `<b>` that was
    /// already there the outer of the two.
    void adopt(const Reading& given, std::size_t at, std::size_t grown) {
        const std::size_t base = tagsRead;
        tagsRead += given.tags.size();

        for (std::size_t index = 0; index < given.tags.size(); ++index) {
            const Tag& tag = given.tags[index];
            if (tag.kind == TagKind::Opaque) {
                loose.push_back(Loose{.text = tag.text, .at = at + tag.at});
                continue;
            }
            if (tag.kind == TagKind::Closes)
                continue;
            for (const std::string& style : tag.styles) {
                spans.push_back(Span{.opening = tag.text,
                                     .closing = std::nullopt,
                                     .style = style,
                                     .first = at + tag.at,
                                     .last = at + grown,
                                     .writer = base + index,
                                     .closer = 0,
                                     .reached = true});
            }
        }

        for (std::size_t index = 0; index < given.tags.size(); ++index) {
            const Tag& tag = given.tags[index];
            if (tag.kind != TagKind::Closes)
                continue;
            for (const std::string& style : tag.styles)
                shutInside(style, tag, base + index, at, at + grown);
        }
    }

    /// Shuts the span of `style` the replacement's own opener made, between
    /// `first` and `last`.
    void shutInside(const std::string& style,
                    const Tag& closing,
                    std::size_t number,
                    std::size_t first,
                    std::size_t last) {
        for (std::size_t which = spans.size(); which-- > 0;) {
            Span& span = spans[which];
            if (span.style == style && span.first >= first && span.last == last) {
                span.closing = closing.text;
                span.closer = number;
                span.last = first + closing.at;
                return;
            }
        }
    }

    /// Turns a reading into spans and loose tags.
    void gather(const Reading& found) {
        const std::size_t base = tagsRead;
        tagsRead += found.tags.size();
        std::vector<std::size_t> open; // indices into spans, innermost last

        for (std::size_t index = 0; index < found.tags.size(); ++index) {
            const Tag& tag = found.tags[index];
            if (tag.kind == TagKind::Opaque) {
                loose.push_back(Loose{.text = tag.text, .at = tag.at});
                continue;
            }
            if (tag.kind == TagKind::Opens) {
                for (const std::string& style : tag.styles) {
                    spans.push_back(Span{.opening = tag.text,
                                         .closing = std::nullopt,
                                         .style = style,
                                         .first = tag.at,
                                         .last = found.visible.size(),
                                         .writer = base + index,
                                         .closer = 0,
                                         .reached = false});
                    open.push_back(spans.size() - 1);
                }
                continue;
            }

            // A closing tag shuts the innermost span of each style it names;
            // one that shuts nothing is text the document carries, and it
            // stays where it is.
            bool shut = false;
            for (const std::string& style : tag.styles) {
                for (std::size_t which = open.size(); which-- > 0;) {
                    Span& span = spans[open[which]];
                    if (span.style != style)
                        continue;
                    span.closing = tag.text;
                    span.closer = base + index;
                    span.last = tag.at;
                    open.erase(open.begin() + static_cast<std::ptrdiff_t>(which));
                    shut = true;
                    break;
                }
            }
            if (!shut)
                loose.push_back(Loose{.text = tag.text, .at = tag.at});
        }
    }
};

namespace {

/// `span` with a boundary that cuts a word pushed out to the word's edge.
[[nodiscard]] Span widenedOverWords(std::string_view visible, Span span) {
    while (cutsWord(visible, span.first))
        span.first = previousCodePoint(visible, span.first);
    while (cutsWord(visible, span.last))
        span.last = nextCodePoint(visible, span.last);
    return span;
}

/// Tells whether two spans a replacement may have run together meet.
///
/// **Touching is enough when the replacement reached both**: they cover what it
/// wrote, and two `<i>` side by side over one replacement are one italic. When
/// it reached only one, they must overlap — an identical tag that only touches
/// the match is a tag the file wrote apart, and it stays apart.
[[nodiscard]] bool meet(const Span& left, const Span& right) {
    if (left.reached && right.reached)
        return right.first <= left.last && left.first <= right.last;
    return right.first < left.last && left.first < right.last;
}

/// Merges two spans a replacement has run together, when they are the same tag.
///
/// **The same tag, and not the same style** — issue #402. Two `<font>` of two
/// colours are one style name and two things; merging on the name alone kept
/// the first colour and lost the second. And at least one of the two must be a
/// span the replacement reached: two identical tags side by side elsewhere in
/// the subtitle are what the file holds, and they stay two.
void mergeTwins(std::vector<Span>& spans) {
    for (std::size_t left = 0; left < spans.size(); ++left) {
        for (std::size_t right = spans.size(); right-- > left + 1;) {
            if (spans[left].style != spans[right].style ||
                spans[left].opening != spans[right].opening)
                continue;
            if (!spans[left].reached && !spans[right].reached)
                continue;
            if (!meet(spans[left], spans[right]))
                continue;
            spans[left].first = std::min(spans[left].first, spans[right].first);
            if (spans[right].last >= spans[left].last) {
                spans[left].last = spans[right].last;
                spans[left].closing = spans[right].closing;
                spans[left].closer = spans[right].closer;
            }
            spans[left].reached = true;
            spans.erase(spans.begin() + static_cast<std::ptrdiff_t>(right));
        }
    }
}

/// The tags already written at one offset, so that a tag two spans share is
/// written once.
struct Written {
    std::size_t opener = std::string::npos;
    std::vector<std::size_t> closers;
};

/// Writes the closing tag of `span`, unless the same tag went out here already.
void writeCloser(const Span& span, Written& written, std::string& out) {
    if (!span.closing.has_value() ||
        std::ranges::find(written.closers, span.closer) != written.closers.end())
        return;
    out += *span.closing;
    written.closers.push_back(span.closer);
}

/// Writes an empty pair, which shuts where it opens: it wraps nothing, so
/// nothing may land inside it.
void writeEmpty(const Span& span, Written& written, std::string& out) {
    if (span.writer != written.opener) {
        out += span.opening;
        written.opener = span.writer;
    }
    writeCloser(span, written, out);
}

/// The spans that open and shut at `position`.
[[nodiscard]] std::vector<std::size_t> emptiesAt(const std::vector<Span>& spans,
                                                 std::size_t position) {
    std::vector<std::size_t> empties;
    for (std::size_t which = 0; which < spans.size(); ++which) {
        if (spans[which].first == position && spans[which].last == position)
            empties.push_back(which);
    }
    return empties;
}

/// Writes the closers of the spans that end at `position`, innermost first:
/// what opened last shuts first.
///
/// **An empty pair the file wrote before a closer goes before it.** `<b>x<i></i></b>`
/// shuts the bold at the offset the pair sits at, and writing every closer
/// first took the pair out of the bold that held it.
void writeClosers(const std::vector<Span>& spans,
                  std::vector<std::size_t>& open,
                  std::vector<std::size_t>& empties,
                  std::size_t position,
                  Written& written,
                  std::string& out) {
    for (std::size_t which = open.size(); which-- > 0;) {
        const Span& span = spans[open[which]];
        if (span.last != position)
            continue;
        for (std::size_t pending = 0; pending < empties.size();) {
            if (spans[empties[pending]].writer > span.closer) {
                ++pending;
                continue;
            }
            writeEmpty(spans[empties[pending]], written, out);
            empties.erase(empties.begin() + static_cast<std::ptrdiff_t>(pending));
        }
        writeCloser(span, written, out);
        open.erase(open.begin() + static_cast<std::ptrdiff_t>(which));
    }
}

/// Writes the openers of the spans that start at `position`, in the order they
/// were made, and the empty pairs no closer came after.
///
/// A tag that opened two styles at once is written once.
void writeOpeners(const std::vector<Span>& spans,
                  std::vector<std::size_t>& open,
                  const std::vector<std::size_t>& empties,
                  std::size_t position,
                  Written& written,
                  std::string& out) {
    for (std::size_t which = 0; which < spans.size(); ++which) {
        const Span& span = spans[which];
        if (span.first != position)
            continue;
        if (span.last == position) {
            if (std::ranges::find(empties, which) != empties.end())
                writeEmpty(span, written, out);
            continue;
        }
        if (span.writer != written.opener) {
            out += span.opening;
            written.opener = span.writer;
        }
        open.push_back(which);
    }
}

} // namespace

MarkupParser::MarkupParser(std::string_view text, SubtitleFormat format)
    : m_held(std::make_unique<Held>()) {
    m_held->original = std::string{text};
    m_held->vocabulary = vocabularyOf(format);

    // The tags stay where the file put them. Pushing a boundary to the edge of
    // a word is what a replacement does to the tags it reaches, and nothing
    // else does it — issue #402.
    const Reading found = read(text, m_held->vocabulary);
    m_held->visible = found.visible;
    m_held->gather(found);
}

MarkupParser::~MarkupParser() = default;
MarkupParser::MarkupParser(MarkupParser&&) noexcept = default;
MarkupParser& MarkupParser::operator=(MarkupParser&&) noexcept = default;

std::string_view MarkupParser::visible() const {
    return m_held->visible;
}

void MarkupParser::replace(std::size_t at, std::size_t count, std::string_view replacement) {
    m_held->touched = true;
    const std::size_t end = at + count;

    const Reading given = read(replacement, m_held->vocabulary);
    const std::size_t grown = given.visible.size();
    const auto shift = static_cast<std::ptrdiff_t>(grown) - static_cast<std::ptrdiff_t>(count);

    const auto moved = [&](std::size_t position) -> std::size_t {
        if (position <= at)
            return position;
        if (position >= end)
            return static_cast<std::size_t>(static_cast<std::ptrdiff_t>(position) + shift);
        return at; // inside what went away
    };

    // **A boundary that cuts a word is pushed to its edge, and every style that
    // then touches the match covers the whole of it** — the two halves of the
    // rule, in that order. A span the match does not reach, even widened, is
    // left exactly where it was: it only shifts with the text before it.
    for (Span& span : m_held->spans) {
        const Span widened = widenedOverWords(m_held->visible, span);
        span.reached = widened.first < end && widened.last > at;
        if (span.reached) {
            span.first = std::min(widened.first, at);
            span.last = std::max(widened.last, end);
        }
        span.first = moved(span.first);
        span.last = moved(span.last);
    }
    for (Loose& one : m_held->loose)
        one.at = moved(one.at);

    m_held->adopt(given, at, grown);

    m_held->visible = m_held->visible.substr(0, at) + given.visible + m_held->visible.substr(end);

    // A style whose whole text went away goes with it; an empty pair the file
    // held elsewhere is the file's business.
    std::erase_if(m_held->spans,
                  [](const Span& span) { return span.reached && span.first >= span.last; });
    mergeTwins(m_held->spans);
}

void MarkupParser::transform(std::size_t at, std::size_t count, std::string_view replacement) {
    const std::size_t end = at + count;
    const std::size_t grown = replacement.size();
    const auto shift = static_cast<std::ptrdiff_t>(grown) - static_cast<std::ptrdiff_t>(count);

    // A tag inside what was rewritten keeps the place it had, as far as the new
    // text reaches. Nothing widens, and nothing is adopted.
    const auto moved = [&](std::size_t position) -> std::size_t {
        if (position <= at)
            return position;
        if (position >= end)
            return static_cast<std::size_t>(static_cast<std::ptrdiff_t>(position) + shift);
        return std::min(position, at + grown);
    };

    rewrite(m_held->visible.substr(0, at) + std::string{replacement} + m_held->visible.substr(end),
            moved);
}

void MarkupParser::rewrite(std::string_view rewritten,
                           const std::function<std::size_t(std::size_t)>& placed) {
    m_held->touched = true;

    // A span the rewrite emptied goes; one the file already held empty stays.
    // Nothing merges: two tags side by side before a transformation are two
    // tags side by side after it.
    std::vector<bool> emptied(m_held->spans.size(), false);
    for (std::size_t which = 0; which < m_held->spans.size(); ++which) {
        Span& span = m_held->spans[which];
        const bool wasEmpty = span.first >= span.last;
        span.first = placed(span.first);
        span.last = placed(span.last);
        emptied[which] = !wasEmpty && span.first >= span.last;
    }
    for (Loose& one : m_held->loose)
        one.at = placed(one.at);

    m_held->visible = std::string{rewritten};

    for (std::size_t which = emptied.size(); which-- > 0;) {
        if (emptied[which])
            m_held->spans.erase(m_held->spans.begin() + static_cast<std::ptrdiff_t>(which));
    }
}

std::string MarkupParser::text() const {
    if (!m_held->touched)
        return m_held->original;

    std::string out;
    std::vector<std::size_t> open;

    const auto writeAt = [&](std::size_t position) {
        Written written;
        std::vector<std::size_t> empties = emptiesAt(m_held->spans, position);
        writeClosers(m_held->spans, open, empties, position, written, out);
        for (const Loose& one : m_held->loose) {
            if (one.at == position)
                out += one.text;
        }
        writeOpeners(m_held->spans, open, empties, position, written, out);
    };

    for (std::size_t at = 0; at < m_held->visible.size(); ++at) {
        writeAt(at);
        out += m_held->visible[at];
    }
    writeAt(m_held->visible.size());
    return out;
}

} // namespace subedit::core
