#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

/// The nature of what a command did.
enum class ChangeKind {
    Positions,       ///< start, end, or both
    MainText,        ///< the main text
    TranslationText, ///< the translation
    Insertion,       ///< subtitles added
    Removal,         ///< subtitles taken away
    Reordering,      ///< subtitles moved, without any of them changing
};

/// What a command touched, and where.
///
/// The core knows no signal mechanism: it returns the information and the
/// caller does what it likes with it. This is what lets the interface refresh
/// the rows that moved instead of rebuilding the whole table.
struct Change {
    ChangeKind kind;
    std::vector<SubtitleIndex> indices;

    friend bool operator==(const Change&, const Change&) = default;
};

/// Tells whether a change of that nature makes `document` differ from the file
/// on disk.
///
/// A subtitle holds one pair of positions for both texts, so moving it moves
/// the translation with it; adding or removing a subtitle adds or removes both
/// of its texts at once. Only a change of text concerns a single document.
[[nodiscard]] constexpr bool affects(ChangeKind kind, Document document) {
    switch (kind) {
    case ChangeKind::MainText:
        return document == Document::Main;
    case ChangeKind::TranslationText:
        return document == Document::Translation;
    case ChangeKind::Positions:
    case ChangeKind::Insertion:
    case ChangeKind::Removal:
    case ChangeKind::Reordering:
        return true;
    }
    // Exhaustive above, and the compiler checks that it is. Only a cast from
    // an out-of-range integer could land here.
    std::unreachable();
}

} // namespace subedit::core
