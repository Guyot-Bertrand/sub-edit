#pragma once

namespace subedit::core {

/// The subtitle formats the library handles.
///
/// **In the model and not in `format/`, since ADR 0018.** A document *has* a
/// format the way it has line endings: it is a property of what was read, kept
/// so that writing can put it back. `format/` holds the operations that turn
/// bytes into a model and back; the vocabulary those operations speak about a
/// document belongs here.
///
/// **Nine since phase 9, and the order is the one the spec tables use** —
/// roughly from the richest to the poorest, which is also the order in which
/// the readers are written. The list is closed: it is Gaupol's, and matching it
/// is the whole point of the project.
enum class SubtitleFormat {
    SubRip,
    WebVtt,
    SubViewer2,
    SubStationAlpha,
    AdvancedSubStationAlpha,
    MicroDvd,
    Mpl2,
    TMPlayer,
    Lrc,
};

} // namespace subedit::core
