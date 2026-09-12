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
    /// `replacement` is read in the same vocabulary, so it may carry tags of
    /// its own: they are the only way a user typing in a dialog box can ask for
    /// a style. What they say is added to what the replaced text carried.
    ///
    /// **Offsets are into `visible()`, and they move**: replacing shifts what
    /// follows, so a caller walking several occurrences either goes backwards
    /// or asks `visible()` again.
    void replace(std::size_t at, std::size_t count, std::string_view replacement);

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
