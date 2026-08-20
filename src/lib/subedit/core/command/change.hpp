#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/selection.hpp>
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
///
/// **A `Selection` and not a list of indices, since issue #45.** The two carry
/// the same thing under the same invariants, and describing a shift over four
/// thousand subtitles used to hand back four thousand indices — on every
/// apply, undo and redo. A table model wants runs anyway: Qt refreshes by
/// top-left and bottom-right corners, not row by row.
struct Change {
    ChangeKind kind;
    Selection subtitles;

    friend bool operator==(const Change&, const Change&) = default;
};

/// Returns what the same change becomes when it is undone.
///
/// Only a change of structure has a direction: undoing an insertion removes the
/// rows it added, and undoing a removal puts them back. Everything else names
/// the same rows whichever way it is played — a text put back is still a text
/// changed.
///
/// **The indices do not move.** A removal hands back the subtitles at the very
/// indices it took them from, so the same `Selection` describes both directions.
[[nodiscard]] constexpr ChangeKind invert(ChangeKind kind) {
    switch (kind) {
    case ChangeKind::Insertion:
        return ChangeKind::Removal;
    case ChangeKind::Removal:
        return ChangeKind::Insertion;
    case ChangeKind::Positions:
    case ChangeKind::MainText:
    case ChangeKind::TranslationText:
    case ChangeKind::Reordering:
        return kind;
    }
    // Exhaustive above, and the compiler checks that it is.
    std::unreachable();
}

/// Returns `changes` as they read when they are undone.
[[nodiscard]] inline std::vector<Change> inverted(std::vector<Change> changes) {
    for (Change& change : changes)
        change.kind = invert(change.kind);

    return changes;
}

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
