#include <subedit/core/text/markup_parser.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The three characters MPL2 writes at the head of a line.
constexpr std::string_view kMpl2Markers = "\\/_";

[[nodiscard]] char lowered(char letter) {
    return letter >= 'A' && letter <= 'Z' ? static_cast<char>(letter - 'A' + 'a') : letter;
}

/// Tells whether `letter` belongs to a word.
///
/// **Bytes, deliberately.** Every byte of a UTF-8 sequence above the ASCII
/// range is counted as part of a word, so an accented letter never looks like a
/// boundary. Nothing finer is needed: the question is « does this tag fall
/// inside a word », and a word never ends in the middle of a character.
[[nodiscard]] bool inWord(char letter) {
    /// The first byte above the ASCII range: everything at or above it belongs
    /// to a multi-byte sequence, and no word ends inside a character.
    constexpr unsigned char kBeyondAscii = 0x80;

    const auto byte = static_cast<unsigned char>(letter);
    return byte >= kBeyondAscii || (letter >= '0' && letter <= '9') ||
           (lowered(letter) >= 'a' && lowered(letter) <= 'z');
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

/// Reads one HTML tag, brackets off.
[[nodiscard]] Tag htmlTag(std::string_view inside) {
    Tag tag;
    const bool closing = inside.starts_with('/');
    const std::string_view body = closing ? inside.substr(1) : inside;

    std::string name;
    for (const char letter : body) {
        if (letter == ' ' || letter == '/')
            break;
        name += lowered(letter);
    }
    if (name.empty())
        return tag;

    tag.kind = closing ? TagKind::Closes : TagKind::Opens;
    tag.styles.push_back(std::move(name));
    return tag;
}

/// Reads one Sub Station Alpha block, braces off.
///
/// **A block says as many things as it likes**, and `{\b1\i1}` says two. Only
/// the three flags pair up; a colour, a font, a position or a comment has no
/// closer, so a block carrying any of them is carried whole and never moved.
[[nodiscard]] Tag subStationAlphaTag(std::string_view block) {
    Tag tag;
    std::vector<std::string> opened;
    std::vector<std::string> closed;

    while (!block.empty()) {
        if (block.front() != '\\')
            return tag;
        block.remove_prefix(1);
        const std::size_t next = block.find('\\');
        const std::string_view override = block.substr(0, next);
        block = next == std::string_view::npos ? std::string_view{} : block.substr(next);

        if (override.size() < 2)
            return tag;
        const char letter = override.front();
        if (letter != 'b' && letter != 'i' && letter != 'u')
            return tag;

        bool on = false;
        for (const char digit : override.substr(1)) {
            if (digit < '0' || digit > '9')
                return tag;
            on = on || digit != '0';
        }
        (on ? opened : closed).push_back(std::string{letter});
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
[[nodiscard]] Tag tagOf(std::string_view inside, MarkupVocabulary vocabulary) {
    switch (vocabulary) {
    case MarkupVocabulary::Html:
        return htmlTag(inside);
    case MarkupVocabulary::SubStationAlpha:
        return subStationAlphaTag(inside);
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

[[nodiscard]] Reading read(std::string_view text, MarkupVocabulary vocabulary) {
    Reading found;
    found.visible.reserve(text.size());

    const bool angles = vocabulary == MarkupVocabulary::Html;
    const bool braces = vocabulary == MarkupVocabulary::SubStationAlpha ||
                        vocabulary == MarkupVocabulary::MicroDvd ||
                        vocabulary == MarkupVocabulary::Mpl2;
    const bool markers = vocabulary == MarkupVocabulary::Mpl2;

    bool atLineHead = true;
    for (std::size_t at = 0; at < text.size();) {
        if (markers && atLineHead && kMpl2Markers.contains(text[at])) {
            Tag tag = opaqueTag();
            tag.text = text.substr(at, 1);
            tag.at = found.visible.size();
            found.tags.push_back(std::move(tag));
            ++at;
            continue;
        }

        const char opening = angles ? '<' : '{';
        const char closing = angles ? '>' : '}';
        if ((angles || braces) && text[at] == opening) {
            const std::size_t ending = text.find(closing, at);
            if (ending != std::string_view::npos) {
                const std::string_view inside = text.substr(at + 1, ending - at - 1);
                Tag tag = tagOf(inside, vocabulary);
                tag.text = text.substr(at, ending - at + 1);
                tag.at = found.visible.size();
                found.tags.push_back(std::move(tag));
                at = ending + 1;
                continue;
            }
        }

        atLineHead = text[at] == '\n';
        found.visible += text[at];
        ++at;
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
    /// Which opening tag wrote it, so that a block opening two styles is
    /// written once rather than twice.
    std::size_t writer = 0;
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

    /// Takes in the tags a replacement carried, inside the stretch it fills.
    ///
    /// They land after the inherited ones, which is what makes a `<b>` that was
    /// already there the outer of the two.
    void adopt(const Reading& given, std::size_t at, std::size_t grown) {
        for (const Tag& tag : given.tags) {
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
                                     .writer = spans.size()});
            }
        }

        for (const Tag& tag : given.tags) {
            if (tag.kind != TagKind::Closes)
                continue;
            for (const std::string& style : tag.styles)
                shutInside(style, tag.text, at, at + grown, at + tag.at);
        }
    }

    /// Shuts the span of `style` the replacement's own opener made.
    void shutInside(const std::string& style,
                    const std::string& closing,
                    std::size_t first,
                    std::size_t last,
                    std::size_t at) {
        for (std::size_t which = spans.size(); which-- > 0;) {
            Span& span = spans[which];
            if (span.style == style && span.first >= first && span.last == last) {
                span.closing = closing;
                span.last = at;
                return;
            }
        }
    }

    /// Turns a reading into spans and loose tags.
    void gather(const Reading& found) {
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
                                         .writer = index});
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

/// Pushes a boundary that cuts a word out to the word's edge.
void widenOverWords(const std::string& visible, std::vector<Span>& spans) {
    const auto cuts = [&visible](std::size_t at) {
        return at > 0 && at < visible.size() && inWord(visible[at - 1]) && inWord(visible[at]);
    };
    for (Span& span : spans) {
        while (cuts(span.first))
            --span.first;
        while (cuts(span.last))
            ++span.last;
    }
}

/// Merges the spans of one style that now cover the same stretch, or touch.
void mergeTwins(std::vector<Span>& spans) {
    for (std::size_t left = 0; left < spans.size(); ++left) {
        for (std::size_t right = spans.size(); right-- > left + 1;) {
            if (spans[left].style != spans[right].style)
                continue;
            if (spans[right].first > spans[left].last || spans[left].first > spans[right].last)
                continue;
            spans[left].first = std::min(spans[left].first, spans[right].first);
            if (spans[right].last >= spans[left].last) {
                spans[left].last = spans[right].last;
                spans[left].closing = spans[right].closing;
            }
            spans.erase(spans.begin() + static_cast<std::ptrdiff_t>(right));
        }
    }
}

} // namespace

MarkupParser::MarkupParser(std::string_view text, SubtitleFormat format)
    : m_held(std::make_unique<Held>()) {
    m_held->original = std::string{text};
    m_held->vocabulary = vocabularyOf(format);

    const Reading found = read(text, m_held->vocabulary);
    m_held->visible = found.visible;
    m_held->gather(found);
    widenOverWords(m_held->visible, m_held->spans);
    mergeTwins(m_held->spans);
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

    // **Every style that touches the match covers the whole of it**, which is
    // the second half of the rule and the reason a span is widened before it is
    // moved: a `<i>` over half the match comes out over all of the replacement.
    for (Span& span : m_held->spans) {
        if (span.first < end && span.last > at) {
            span.first = std::min(span.first, at);
            span.last = std::max(span.last, end);
        }
        span.first = moved(span.first);
        span.last = moved(span.last);
    }
    for (Loose& one : m_held->loose)
        one.at = moved(one.at);

    m_held->adopt(given, at, grown);

    m_held->visible = m_held->visible.substr(0, at) + given.visible + m_held->visible.substr(end);

    std::erase_if(m_held->spans, [](const Span& span) { return span.first >= span.last; });
    mergeTwins(m_held->spans);
}

std::string MarkupParser::text() const {
    if (!m_held->touched)
        return m_held->original;

    std::string out;
    std::vector<std::size_t> open;

    const auto writeAt = [&](std::size_t position) {
        // Closers first, innermost first: what opened last shuts first.
        for (std::size_t which = open.size(); which-- > 0;) {
            const Span& span = m_held->spans[open[which]];
            if (span.last != position)
                continue;
            if (span.closing.has_value())
                out += *span.closing;
            open.erase(open.begin() + static_cast<std::ptrdiff_t>(which));
        }
        for (const Loose& one : m_held->loose) {
            if (one.at == position)
                out += one.text;
        }
        // Openers in the order they were made, and a tag that opened two styles
        // at once is written once.
        std::size_t written = std::string::npos;
        for (std::size_t which = 0; which < m_held->spans.size(); ++which) {
            const Span& span = m_held->spans[which];
            if (span.first != position)
                continue;
            if (span.writer != written) {
                out += span.opening;
                written = span.writer;
            }
            open.push_back(which);
        }
    };

    for (std::size_t at = 0; at < m_held->visible.size(); ++at) {
        writeAt(at);
        out += m_held->visible[at];
    }
    writeAt(m_held->visible.size());
    return out;
}

std::string replacingAll(std::string_view text,
                         std::string_view pattern,
                         std::string_view replacement,
                         SubtitleFormat format) {
    if (pattern.empty())
        return std::string{text};

    MarkupParser parser{text, format};
    bool found = false;
    std::size_t at = 0;
    while (true) {
        const std::size_t which = parser.visible().find(pattern, at);
        if (which == std::string_view::npos)
            break;
        parser.replace(which, pattern.size(), replacement);
        at = which + MarkupParser{replacement, format}.visible().size();
        found = true;
    }
    return found ? parser.text() : std::string{text};
}

} // namespace subedit::core
