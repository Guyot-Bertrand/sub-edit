#pragma once

namespace subedit::core {

/// The subtitle formats the library handles.
///
/// **In the model and not in `format/`, since ADR 0018.** A document *has* a
/// format the way it has line endings: it is a property of what was read, kept
/// so that writing can put it back. `format/` holds the operations that turn
/// bytes into a model and back; the vocabulary those operations speak about a
/// document belongs here.
enum class SubtitleFormat {
    SubRip,
    WebVtt,
};

} // namespace subedit::core
