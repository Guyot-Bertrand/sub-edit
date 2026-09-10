#pragma once

#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <cstddef>
#include <string>

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

/// Drops from every run what `abilities` cannot write, and counts it.
///
/// **Done before the tags are composed, not while they are.** Two runs that
/// differ only by a colour the target cannot write become the same run once it
/// is gone, and a text encoded run by run would then open and close the same
/// italic twice in a row for no reason.
[[nodiscard]] WritableMarkup keepWritable(const StyledText& runs, const StyleAbilities& abilities);

} // namespace subedit::core
