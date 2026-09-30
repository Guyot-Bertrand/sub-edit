#pragma once

// What the table and its editor need to show the length of a line — issue
// #526.
//
// **The window decides, the delegate draws**: which unit, whether either place
// shows a length, and which format's tags a text is read in are all the
// window's to say (ADR 0022), the same way `SpellCheckerSource` leaves the
// dictionary to it. What comes through here is only what a paint needs.

#include <subedit/core/config/editor_settings.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <QFont>

#include <functional>
#include <memory>
#include <optional>

namespace subedit::gui {

/// How to measure the lines of a text, and in which vocabulary its tags are
/// written.
struct LineLengthDisplay {
    /// Already cached: cells are repainted all the time, and the same line
    /// gets measured over and over.
    std::shared_ptr<const core::LineMeasure> measure;
    core::MarkupVocabulary vocabulary = core::MarkupVocabulary::None;
};

/// Asked at each paint, and each time an editor opens. **Empty when the length
/// is not to be shown** — the setting is off — and the cell or the editor is
/// then drawn exactly as it was before lengths existed.
using LineLengthSource = std::function<std::optional<LineLengthDisplay>()>;

/// The measure the window hands out, for one unit and one font.
///
/// **Its cache lives as long as the font and the unit do** — decision D5: a
/// table that outlived the font could answer a length from a font nobody uses
/// any more. Asked for another unit or another font, it builds a new measure
/// and drops the old table; a measure already handed out stays valid, held by
/// whoever has it, until they let go.
class LengthMeasures {

public:
    /// The measure for `unit` under `font`, cached; the same object until one
    /// of the two changes.
    [[nodiscard]] std::shared_ptr<const core::LineMeasure> measureFor(core::LengthUnit unit,
                                                                      const QFont& font);

private:
    std::shared_ptr<const core::LineMeasure> m_measure;
    core::LengthUnit m_unit = core::LengthUnit::Ems;
    QFont m_font;
};

/// `font`, smaller: what a length is written in, so that it reads as an aside
/// to the line it follows.
[[nodiscard]] QFont smallerFont(const QFont& font);

} // namespace subedit::gui
