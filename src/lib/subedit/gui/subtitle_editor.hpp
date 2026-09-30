#pragma once

// The multiline field a text cell opens, and the margin that shows the length
// of each of its lines — issues #322 and #526.

#include <subedit/gui/line_length_display.hpp>

#include <QPlainTextEdit>
#include <QSize>

#include <optional>
#include <vector>

class QPaintEvent;
class QResizeEvent;
class QWidget;

namespace subedit::gui {

/// What a `QPlainTextEdit` adds around its lines, top and bottom together.
///
/// The frame counts twice, the margin of its document counts twice, and one
/// rounding pixel above once — see `cell_delegates.cpp` for how the pixel was
/// measured. The style is asked rather than a number written down: the window
/// serves two, and the frame is one pixel under Fusion and two under Windows.
[[nodiscard]] int roomAroundTheLines(const QWidget* widget);

/// The multiline field a text cell opens, and the height it asks for.
///
/// **A `QPlainTextEdit` asks for the same height whatever it holds** — a
/// default of several lines, meant for a field of its own — and a table sizing
/// a row around a persistent editor takes it at its word: a two-line subtitle
/// opened a cell two hundred pixels tall. What this one asks for is the height
/// of its document, which is the height its row already has.
///
/// No `Q_OBJECT`: it declares neither signal nor slot, and a macro that buys
/// nothing costs a generated file. `SubtitleTable` is here for the same reason.
class SubtitleEditor final : public QPlainTextEdit {

public:
    explicit SubtitleEditor(QWidget* parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;

    /// What its lines take, room around them included.
    ///
    /// **`QPlainTextDocumentLayout` measures its document in lines**, not in
    /// pixels — the one place in Qt where the height of a `QSizeF` is a count.
    /// That is what is wanted here: a line the editor wrapped for want of width
    /// counts as much as one the user broke.
    [[nodiscard]] int heightOfItsLines() const;

    /// Shows the length of each line in a margin on the right, or takes the
    /// margin away when `display` is empty.
    ///
    /// The lengths follow the text as it is typed. **Gaupol's `ruler.py`, the
    /// same margin**: a number aligned with the first row of each line, the
    /// margin as wide as the largest number, and nothing drawn for an empty
    /// text.
    void showLengths(std::optional<LineLengthDisplay> display);

    /// The width of the margin, zero when there is none.
    [[nodiscard]] int gutterWidth() const;

    /// The length drawn beside each line — one entry per line of the text, and
    /// none for an empty text, which draws nothing.
    [[nodiscard]] const std::vector<int>& gutterLengths() const { return m_lengths; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    class Gutter;

    void refreshLengths();
    void placeGutter();
    void paintGutter(QPaintEvent* event);

    std::optional<LineLengthDisplay> m_display;
    std::vector<int> m_lengths;
    Gutter* m_gutter = nullptr;
};

} // namespace subedit::gui
