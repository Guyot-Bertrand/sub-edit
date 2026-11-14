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
/// **The first nine are Gaupol's, in the order the spec tables of phase 9
/// use** — roughly from the richest to the poorest, which is also the order in
/// which their readers were written. Phase 17 adds formats Gaupol does not
/// read, after them, in the order they arrive (spec 17). A format added here has its rendering of
/// the scene, its rows in the manual and its extension in the detection probe, or
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
    YouTubeSbv,
};

/// Every format, in the order the enumeration declares them.
///
/// **For walking them, never for deciding anything.** A `switch` is what says
/// what a format does; this array is what a loop needs to visit all of them,
/// and it exists so that no caller writes the list a second time.
inline constexpr std::array<SubtitleFormat, 10> kSubtitleFormats = {
    SubtitleFormat::SubRip,
    SubtitleFormat::WebVtt,
    SubtitleFormat::SubViewer2,
    SubtitleFormat::SubStationAlpha,
    SubtitleFormat::AdvancedSubStationAlpha,
    SubtitleFormat::MicroDvd,
    SubtitleFormat::Mpl2,
    SubtitleFormat::TMPlayer,
    SubtitleFormat::Lrc,
    SubtitleFormat::YouTubeSbv,
};

} // namespace subedit::core
