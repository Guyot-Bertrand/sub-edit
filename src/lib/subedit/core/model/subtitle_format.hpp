#pragma once

#include <array>

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
/// the readers are written. The nine are Gaupol's, all of them; phase 17 adds
/// formats Gaupol does not read. A format added here has its rendering of the
/// scene, its rows in the manual and its extension in the detection probe, or
/// `format_corpus_test` and `format_enumerations_test` fail.
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

/// The nine, in the order the enumeration declares them.
///
/// **For walking them, never for deciding anything.** A `switch` is what says
/// what a format does; this array is what a loop needs to visit all of them,
/// and it exists so that no caller writes the list a second time.
inline constexpr std::array<SubtitleFormat, 9> kSubtitleFormats = {
    SubtitleFormat::SubRip,
    SubtitleFormat::WebVtt,
    SubtitleFormat::SubViewer2,
    SubtitleFormat::SubStationAlpha,
    SubtitleFormat::AdvancedSubStationAlpha,
    SubtitleFormat::MicroDvd,
    SubtitleFormat::Mpl2,
    SubtitleFormat::TMPlayer,
    SubtitleFormat::Lrc,
};

} // namespace subedit::core
