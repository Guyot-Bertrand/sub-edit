#pragma once

// The tag-aware parser ADR 0009 has been promising since phase 1.
//
// It holds a text's tags aside, lets the visible text be transformed, and puts
// them back. Without it, applying an expression to the source text breaks the
// tags — the ADR gives the example: `\[.*?\]` swallowing an opening tag and
// leaving its closing one orphaned.
//
// **It is not the pivot of ADR 0031, and the ADR says so itself**: the pivot
// translates one vocabulary into another and drops what it cannot carry; this
// one understands no tag at all. It carries every tag as the literal string it
// found, which is what lets a `{\pos(x,y)}` cross untouched and a `<i>` nobody
// closed stay unclosed.
//
// **Its rule is written case by case**, in `src/test/data/textes/recherche.cas`
// and its brace sibling. Where the two disagree, the corpus is right.

#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace subedit::core {

/// A text cut into what is seen and what marks it up.
class MarkupParser {

public:
    /// Reads `text` in the vocabulary `format` writes.
    MarkupParser(std::string_view text, SubtitleFormat format);

    ~MarkupParser();

    MarkupParser(const MarkupParser&) = delete;
    MarkupParser& operator=(const MarkupParser&) = delete;
    MarkupParser(MarkupParser&&) noexcept;
    MarkupParser& operator=(MarkupParser&&) noexcept;

    /// The text without its tags — what a search looks in.
    [[nodiscard]] std::string_view visible() const;

    /// Replaces `count` visible characters at `at` by `replacement`.
    ///
    /// **The only operation that moves a tag, and only the tags it reaches** —
    /// issue #402. A boundary that cuts a word is pushed to the word's edge,
    /// then every style touching the match covers the whole replacement. A tag
    /// the match does not reach, even pushed, stays where the file had it.
    ///
    /// `replacement` is read in the same vocabulary, so it may carry tags of
    /// its own: they are the only way a user typing in a dialog box can ask for
    /// a style. What they say is added to what the replaced text carried.
    ///
    /// **Offsets are into `visible()`, and they move**: replacing shifts what
    /// follows, so a caller walking several occurrences either goes backwards
    /// or asks `visible()` again.
    void replace(std::size_t at, std::size_t count, std::string_view replacement);

    /// Rewrites `count` visible characters at `at`, leaving every tag where it
    /// stands.
    ///
    /// **The other half of the pair, and what separates them is what a style
    /// does.** A *replacement* — a phrase typed in place of another — takes the
    /// style of everything it touched, because nothing relates the new text to
    /// the old. A *transformation* — the same text in another case, a dash put
    /// at the head of a line — takes none: letting the italic over the first
    /// word of a subtitle spread to its last because the case changed would be
    /// absurd. **Nor does it push a boundary to a word's edge**: a tag cutting a
    /// word before cuts it after, and two tags side by side stay two.
    ///
    /// `replacement` is visible text and only that: nothing in it is read as a
    /// tag, a transformation having no business inventing one.
    void transform(std::size_t at, std::size_t count, std::string_view replacement);

    /// Rewrites the whole visible text, each tag carried to the offset
    /// `placed` gives for the one it had.
    ///
    /// **For a transformation whose length changes inside the text** — a case
    /// mapping, where `ﬁ` becomes `FI` and `İ` becomes two code points. A
    /// rewrite at one offset cannot follow that: a tag keeps its byte offset,
    /// and lands inside a letter or outside its word. Only the mapping knows
    /// where each offset went, so it says so. Issue #401.
    ///
    /// `placed` receives every offset a tag holds, at the start of a code point,
    /// and answers one into `rewritten`; it must never go backwards.
    void rewrite(std::string_view rewritten, const std::function<std::size_t(std::size_t)>& placed);

    /// The text, tags put back.
    [[nodiscard]] std::string text() const;

private:
    struct Held;
    std::unique_ptr<Held> m_held;
};

/// Returns `text` with every occurrence of `pattern` replaced.
///
/// Nothing is rebuilt when nothing matches: the text comes back as the very
/// bytes it arrived as, which is what keeps a search that finds nothing from
/// tidying a file behind the user's back.
[[nodiscard]] std::string replacingAll(std::string_view text,
                                       std::string_view pattern,
                                       std::string_view replacement,
                                       SubtitleFormat format);

} // namespace subedit::core
