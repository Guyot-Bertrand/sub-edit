#pragma once

// What the table and its editor show of a line's length — issue #526, Gaupol's
// `editor.length_unit`, `editor.show_lengths_cell` and `editor.show_lengths_edit`.

namespace subedit::core {

/// The unit a line's length is shown in.
enum class LengthUnit {
    /// The width of the line under the application's font, in Gaupol's ems.
    Ems,
    /// The number of Unicode code points.
    Characters,
};

/// **Not the assistant's unit**: `correction.line-break.in-ems` sets what the
/// line-breaker aims at, and these three set what the table shows. Gaupol keeps
/// them apart too (`text_assistant.length_unit` and `editor.length_unit`), and
/// so does anyone who reads lengths in ems and breaks lines in characters.
struct EditorSettings {
    /// Ems by default, as in Gaupol.
    LengthUnit lengthUnit = LengthUnit::Ems;
    /// Each line of a text cell followed by its length — on by default.
    bool showLengthsInCells = true;
    /// A margin beside the cell editor with the length of each line — on by
    /// default.
    bool showLengthsInEditor = true;

    friend bool operator==(const EditorSettings&, const EditorSettings&) = default;
};

} // namespace subedit::core
