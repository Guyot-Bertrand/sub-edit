#pragma once

#include <QDialog>

class QCheckBox;
class QSpinBox;

namespace subedit::gui {

/// About two frames at 25 images a second, in milliseconds.
inline constexpr int kDefaultShortestMs = 80;

/// What `Make Ranges Disjoint…` is asked for — the three settings of the
/// operation, in plain numbers so that this header need not know the core's.
struct DisjointChoice {
    /// An interval shorter than this many milliseconds is taken over by a
    /// neighbour.
    int shortestMs = kDefaultShortestMs;

    /// The most lines a merged subtitle may hold; zero lifts the limit.
    int mostLines = 2;

    /// A dash before the text each subtitle contributes to a merged one.
    bool dashes = false;

    friend bool operator==(const DisjointChoice&, const DisjointChoice&) = default;
};

/// Asks for the settings of an operation that cuts overlapping subtitles so that
/// no range overlaps another.
///
/// It acts on the whole project, never on a selection: a group of overlapping
/// subtitles is cut where it overlaps, and half a group has no meaning.
class DisjointDialog final : public QDialog {
    Q_OBJECT

public:
    /// Opens on `initial`, which is what the last use of this window asked for.
    explicit DisjointDialog(const DisjointChoice& initial, QWidget* parent = nullptr);

    /// The form as the fields say it.
    [[nodiscard]] DisjointChoice choice() const;

    [[nodiscard]] QSpinBox* shortestBox() const { return m_shortest; }

    [[nodiscard]] QSpinBox* linesBox() const { return m_lines; }

    [[nodiscard]] QCheckBox* dashesCheck() const { return m_dashes; }

private:
    QSpinBox* m_shortest;
    QSpinBox* m_lines;
    QCheckBox* m_dashes;
};

} // namespace subedit::gui
