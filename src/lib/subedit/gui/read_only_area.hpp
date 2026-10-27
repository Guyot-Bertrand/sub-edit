#pragma once

class QPlainTextEdit;

namespace subedit::gui {

/// Gives a text area that is only shown, never typed in, its look: **a thin frame and a background
/// of its own**, so that one sees where it begins and ends and that it is not a field to type in.
///
/// The background is the palette's `AlternateBase`, which both palettes (light and dark) make a
/// shade off the window's and off an input's, and the frame is the style's own. The area stays out
/// of the tab chain's way (a click gives it the focus, so that its text can be copied).
///
/// **One place for every read-only area of a dialog**, so that the next one gets the look without
/// thinking of it.
void styleAsReadOnly(QPlainTextEdit& area);

} // namespace subedit::gui
