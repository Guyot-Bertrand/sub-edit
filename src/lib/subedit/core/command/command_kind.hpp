#pragma once

namespace subedit::core {

/// What a command is, as a category.
///
/// An enumeration and not a string, for the reason that already holds for
/// `DiagnosticKind`: it gets translated, and a test can hold on to it without
/// comparing prose that will be reworded. This is what an interface turns into
/// « Undo: shift positions ».
enum class CommandKind {
    SetText,
    SetStart,
    SetEnd,
    Insert,
    Remove,
    Shift,
    Transform,
    ConvertFrameRate,
    Snap,
    Sort,
    RemoveHearingImpaired,
};

} // namespace subedit::core
