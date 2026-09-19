#pragma once

// Reading the tags out of a marked-up text — one reader per vocabulary, and
// every piece of the core that looks at a tag goes through it. Issue #403.
//
// **Nine routines read tags before this one, and they disagreed.** The parser
// saw italics in `<i >` where the pivot saw nothing, and the italic toggle could
// never take it out; in MPL2 the parser read a `/` after a brace as a marker
// where the pivot read text. What a tag *means* stays each reader's business —
// the pivot resolves a style, the parser pairs an opener with its closer, the
// italic toggle rewrites one override — but where a tag starts, where it ends
// and what it names are decided here, once.

#include <subedit/core/text/markup_vocabulary.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One stretch of a marked-up text.
struct MarkupPiece {
    enum class Kind {
        Text,   ///< what a viewer sees
        Tag,    ///< a bracketed or braced tag, its brackets or braces included
        Marker, ///< one of the characters MPL2 writes at the head of a line
    };

    Kind kind = Kind::Text;
    /// The bytes of the piece, a view into the text that was read.
    std::string_view text{};
    /// Where they start in that text.
    std::size_t at = 0;
};

/// Cuts `text` into pieces, in the vocabulary that wrote it.
///
/// **A tag opens on `<` or `{` and closes on the first `>` or `}` of the same
/// line**; an opener nothing closes before the line ends is text, and so is one
/// followed by a second opener first — a lone `<` does not swallow the tag
/// after it. No format of
/// the nine writes a tag across a line, and reading one that did would let a
/// stray `<` swallow the lines after it.
///
/// **An MPL2 marker is a marker at the very head of a line, and nowhere
/// else** — not after a brace, where MPL2's own writer never puts one. Each
/// marker is a piece of its own.
///
/// Two pieces of text never follow each other: a stretch of text is one piece.
[[nodiscard]] std::vector<MarkupPiece> piecesOf(std::string_view text, MarkupVocabulary vocabulary);

/// Whether reading `text` could turn up a tag or a marker at all.
///
/// **A no is always right, a yes is only a maybe**: a tag needs its opener, so a
/// text without one is text alone, and reading it is a piece list allocated to
/// learn that. Most subtitles are such texts, and a caller that only wants the
/// tags can skip the read. MPL2 always says yes: a marker is found by its place
/// at the head of a line and not by an opener, and `piecesOf` is the one to say
/// where it sits.
[[nodiscard]] bool mayHoldMarkup(std::string_view text, MarkupVocabulary vocabulary);

/// What an HTML tag names: `<I >` is an opening `i`, `</font>` a closing `font`.
///
/// The name ends at a space or a tab. A self-closing tag — `<br/>`, `<i/>` —
/// names nothing.
struct HtmlTag {
    /// The name, lower-cased — empty for `<>`, which names nothing.
    std::string name{};
    bool closing = false;
    /// What follows the name, the spaces before it off: `color="#ff0000"`.
    std::string_view attributes{};
};

/// Reads `tag`, its angle brackets included.
[[nodiscard]] HtmlTag htmlTagOf(std::string_view tag);

/// The overrides of a Sub Station Alpha block, each without its backslash.
///
/// `{\b1\i1}` holds `b1` and `i1`, and `{}` holds none. Nothing when the block
/// is not a run of overrides at all — `{une note}` is a comment in the wild.
[[nodiscard]] std::optional<std::vector<std::string_view>> overridesOf(std::string_view tag);

/// A `b`, `i` or `u` override, and whether it turns its style on.
///
/// `i1` and `i700` turn italics on and `i0` off: Advanced SSA writes a weight
/// where Sub Station Alpha writes a flag, and any number above zero is on.
struct FlagOverride {
    char letter = 'i';
    bool on = false;
};

/// Reads `override`, backslash off, as one of the three flags — or nothing.
[[nodiscard]] std::optional<FlagOverride> flagOverrideOf(std::string_view override);

/// What a MicroDVD tag says: `{Y:i}` is `y`, over the whole subtitle, valued `i`.
struct ScopedTag {
    /// The letter, lower-cased: `c`, `f`, `s` or `y`.
    char letter = 'y';
    /// A capital reaches the end of the subtitle, a small letter its line.
    bool wholeSubtitle = false;
    std::string_view value{};
};

/// Reads `tag`, braces included, or nothing when it is not `{X:…}`.
[[nodiscard]] std::optional<ScopedTag> scopedTagOf(std::string_view tag);

} // namespace subedit::core
