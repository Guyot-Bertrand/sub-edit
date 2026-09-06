#pragma once

#include <subedit/core/config/theme.hpp>

class QPalette;

namespace subedit::gui {

/// The palette of a theme, laid down by us.
///
/// **Light and dark are palettes this file writes**, and that is what makes
/// them testable: a test asks for "dark" and reads what it gets. A reading of
/// the system's colour scheme would be neither testable nor reproducible under
/// Qt 6.4, which has no API for it — decision D3.
///
/// **`Theme::System` answers the palette as it stands**, the one the platform
/// laid down: it is the value that does nothing, and answering the current
/// palette is the most honest way to say so.
[[nodiscard]] QPalette paletteFor(core::Theme theme);

/// Lays the palette of the theme on the whole application, or lays nothing.
///
/// **Nothing at all for "system"**, and not "the original palette": laying back
/// a palette captured at start-up would freeze it, where laying nothing leaves
/// the toolkit to decide — including on the day it learns how.
void applyTheme(core::Theme theme);

} // namespace subedit::gui
