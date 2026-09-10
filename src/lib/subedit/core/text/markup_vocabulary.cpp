#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <utility>

namespace subedit::core {

MarkupVocabulary vocabularyOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
    case SubtitleFormat::WebVtt:
    case SubtitleFormat::SubViewer2:
        return MarkupVocabulary::Html;
    case SubtitleFormat::SubStationAlpha:
    case SubtitleFormat::AdvancedSubStationAlpha:
        return MarkupVocabulary::SubStationAlpha;
    case SubtitleFormat::MicroDvd:
        return MarkupVocabulary::MicroDvd;
    case SubtitleFormat::Mpl2:
        return MarkupVocabulary::Mpl2;
    case SubtitleFormat::TMPlayer:
    case SubtitleFormat::Lrc:
        return MarkupVocabulary::None;
    }
    std::unreachable();
}

bool StyleAbilities::can(StyleAttribute attribute) const {
    switch (attribute) {
    case StyleAttribute::Bold:
        return bold;
    case StyleAttribute::Italic:
        return italic;
    case StyleAttribute::Underline:
        return underline;
    case StyleAttribute::Colour:
        return colour;
    case StyleAttribute::Font:
        return font;
    case StyleAttribute::Size:
        return size;
    }
    std::unreachable();
}

StyleAbilities abilitiesOf(SubtitleFormat format) {
    switch (format) {
    // `<b>`, `<i>`, `<u>` and `<font color="#RRGGBB">`. No font name, no size:
    // SubRip's `<font>` carries a colour and nothing else here, which is what
    // the format's own application writes.
    case SubtitleFormat::SubRip:
    case SubtitleFormat::SubViewer2:
        return StyleAbilities{.bold = true, .italic = true, .underline = true, .colour = true};
    // The same three tags, and no `<font>` at all — the one difference inside
    // the HTML vocabulary, and the reason abilities are asked of a format
    // rather than of its vocabulary.
    case SubtitleFormat::WebVtt:
        return StyleAbilities{.bold = true, .italic = true, .underline = true};
    // **No underline, and it is Gaupol's line.** `{\u1}` is an Advanced SSA
    // override; `ssa.py` neither writes it nor reads it, and iso-functionality
    // is what settles a question the format's own documentation leaves open.
    case SubtitleFormat::SubStationAlpha:
        return StyleAbilities{
            .bold = true, .italic = true, .colour = true, .font = true, .size = true};
    case SubtitleFormat::AdvancedSubStationAlpha:
    case SubtitleFormat::MicroDvd:
    case SubtitleFormat::Mpl2:
        return StyleAbilities{.bold = true,
                              .italic = true,
                              .underline = true,
                              .colour = true,
                              .font = true,
                              .size = true};
    case SubtitleFormat::TMPlayer:
    case SubtitleFormat::Lrc:
        return StyleAbilities{};
    }
    std::unreachable();
}

} // namespace subedit::core
