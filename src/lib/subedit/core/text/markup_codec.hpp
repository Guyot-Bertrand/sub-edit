#pragma once

#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::core {

/// What reading a marked-up text gave, and what it could not make sense of.
struct DecodedMarkup {
    StyledText runs{};

    /// Tags whose shape the vocabulary knows and the pivot has no room for.
    ///
    /// `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>` — the layout of ADR 0031,
    /// which is deliberately outside the six attributes. They are dropped here
    /// and counted here, because a conversion that says nothing about them
    /// would be the silence the ADR set out to end.
    ///
    /// **Only an opening tag counts.** `<c.loud>…</c>` is one thing lost, not
    /// two, and a reader told « 2 tags dropped » would go looking for a second.
    std::size_t unknown = 0;
};

/// A styled text written back into a vocabulary, and what would not fit.
struct EncodedMarkup {
    std::string text{};

    /// How many style attributes the format could not carry.
    std::size_t dropped = 0;
};

/// A styled text reduced to what a format can write, and what that cost.
struct WritableMarkup {
    StyledText runs{};
    std::size_t dropped = 0;
};

/// Reads `text` in `vocabulary`, into the runs of style ADR 0031 pivots on.
///
/// **The one place that knows which reader a vocabulary calls for**, and the
/// one that knows which writer — the two `switch` below are the whole of it.
/// They lived in the conversion while it was the only thing that needed them;
/// a second reader arrived with the italic toggle, and a second `switch` over
/// the five vocabularies would be a second place to forget one.
[[nodiscard]] DecodedMarkup decodeAs(std::string_view text, MarkupVocabulary vocabulary);

/// Writes `runs` in `vocabulary`, keeping what `abilities` can carry.
///
/// Its only caller is the conversion, and it sits beside `decodeAs` rather than
/// inside it: half a codec in one file and half in another would cost more to
/// read than the symbol costs to expose.
[[nodiscard]] EncodedMarkup
encodeAs(const StyledText& runs, MarkupVocabulary vocabulary, const StyleAbilities& abilities);

/// Drops from every run what `abilities` cannot write, and counts it.
///
/// **Done before the tags are composed, not while they are.** Two runs that
/// differ only by a colour the target cannot write become the same run once it
/// is gone, and a text encoded run by run would then open and close the same
/// italic twice in a row for no reason.
[[nodiscard]] WritableMarkup keepWritable(const StyledText& runs, const StyleAbilities& abilities);

} // namespace subedit::core
