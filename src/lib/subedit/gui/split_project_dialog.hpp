#pragma once

#include <QDialog>

#include <cstddef>

class QSpinBox;
class QWidget;

namespace subedit::gui {

/// Where to cut a project in two — decision D6 of the phase-11 spec.
///
/// **A subtitle number, from 2 to the last**, as Gaupol asks: the first cannot
/// begin the tail, since nothing would stay in the project it leaves. Numbered
/// from one, as the table numbers them; `firstOfTail` gives the index the core
/// wants.
class SplitProjectDialog final : public QDialog {
    Q_OBJECT

public:
    /// `count` is how many subtitles the project has, at least two. `initial`
    /// is the number the box opens on, clamped into range.
    SplitProjectDialog(std::size_t count, std::size_t initial, QWidget* parent = nullptr);

    /// The first subtitle of the tail, as a zero-based index — a plain integer
    /// and not a `SubtitleIndex`, whose header `moc` cannot parse.
    [[nodiscard]] std::size_t firstOfTail() const;

    /// The field, for a test to set without clicking.
    [[nodiscard]] QSpinBox* subtitleBox() const { return m_subtitle; }

signals:
    /// The number in the box changed: `row` is the zero-based row it now
    /// names, for the table to show where the cut falls — issue #462.
    void rowChosen(int row);

private:
    QSpinBox* m_subtitle;
};

} // namespace subedit::gui
