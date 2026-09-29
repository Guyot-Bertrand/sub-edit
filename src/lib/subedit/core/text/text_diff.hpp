#pragma once

// A codepoint-level diff of two texts, for the correction assistant's
// confirmation page — decision D8, issue #505. It says what changed, never
// why: the correction itself is `proposeCorrections`' business.

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One run of a text: either matches the other side, or was changed.
struct DiffSpan {
    std::string text;
    bool changed = false;

    friend bool operator==(const DiffSpan&, const DiffSpan&) = default;
};

/// `original` and `proposed`, each as spans marking what differs between the
/// two — two sequences, since a changed run need not be the same length on
/// both sides.
struct TextDiff {
    std::vector<DiffSpan> original;
    std::vector<DiffSpan> proposed;
};

/// A longest-common-subsequence diff on Unicode code points: a span never
/// splits one apart.
[[nodiscard]] TextDiff diffTexts(std::string_view original, std::string_view proposed);

} // namespace subedit::core
