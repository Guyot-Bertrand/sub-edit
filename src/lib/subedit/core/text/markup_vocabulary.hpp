#pragma once

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>

namespace subedit::core {

/// The ways the nine formats write a style.
///
/// **Nine formats, five vocabularies** — ADR 0031 counts them. Three share the
/// HTML tags, the two Sub Station Alpha share their braces, and three write
/// their own thing or nothing at all.
///
/// It says how a style is **written**, not which ones a format can write: SubRip
/// and WebVTT spell italics the same way and disagree about colour. That second
/// question is `abilitiesOf`.
enum class MarkupVocabulary {
    /// TMPlayer and LRC, which have no way to say anything.
    None,
    /// SubRip, WebVTT, SubViewer 2 — `<i>…</i>`.
    Html,
    /// Sub Station Alpha and Advanced SSA — `{\i1}…{\i0}`.
    SubStationAlpha,
    /// MicroDVD — `{Y:i}`, to the end of the line or of the subtitle.
    MicroDvd,
    /// MPL2 — `/` at the head of a line, plus MicroDVD's braces.
    Mpl2,
};

/// How `format` writes a style.
[[nodiscard]] MarkupVocabulary vocabularyOf(SubtitleFormat format);

/// What a format can say about a run of text.
///
/// **Not the same question as the vocabulary, and the pair that proves it is
/// SubRip and WebVTT**: both write `<i>…</i>`, and only one of them has a
/// `<font color>`. A colour converted from SubRip to WebVTT is therefore lost
/// without a single tag changing shape.
///
/// The lists are Gaupol's, method by method, which is why Sub Station Alpha
/// cannot underline where Advanced SSA can: `ssa.py` has no `underline`, and
/// `ass.py` adds one.
struct StyleAbilities {
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool colour = false;
    bool font = false;
    bool size = false;

    /// Tells whether a format so equipped can write `attribute`.
    [[nodiscard]] bool can(StyleAttribute attribute) const;
};

/// What `format` can write of a style.
[[nodiscard]] StyleAbilities abilitiesOf(SubtitleFormat format);

} // namespace subedit::core
