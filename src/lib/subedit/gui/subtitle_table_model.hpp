#pragma once

#include <QAbstractTableModel>
#include <QVariant>

#include <span>
#include <utility>

/// **Declared rather than included, deliberately.** `moc` parses this header,
/// and it chokes on the C++20 library headers that `change.hpp` drags in
/// through `Selection` — its iterator pulls `<iterator>`, and `<concepts>`
/// follows. Declaring what the signatures need keeps `moc` out of all that; the
/// definitions come in the implementation file.
namespace subedit::core {
class Project;
struct Change;
enum class ChangeKind;
} // namespace subedit::core

namespace subedit::gui {

/// The subtitles of a project, as a table.
///
/// **A thin adapter, and nothing else** — ADR 0019. `data()` reaches into the
/// project and formats on the spot; not one subtitle is copied. Gaupol keeps a
/// second copy of everything in a `Gtk.ListStore` and pays for it in its own
/// code: it unplugs the model from the view past fifty removed rows, "because a
/// large batch of separate live updates is slow".
///
/// The model **does not own the project and does not edit it**. It is handed
/// what changed and turns that into signals; whoever applies a command is the
/// one who knows.
class SubtitleTableModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    /// The columns, in the order the table shows them.
    ///
    /// `Number` is a rank and not a datum — it is `row + 1`, computed and never
    /// stored. `Duration` is derived from the two positions.
    enum Column {
        Number = 0,
        Start,
        End,
        Duration,
        Text,
    };

    /// Combien il y en a.
    ///
    /// Une constante et non un dernier énumérateur : une sentinelle dans
    /// l'énumération oblige chaque `switch` exhaustif à lui donner un cas, que
    /// rien n'atteint jamais.
    static constexpr int kColumnCount = 5;

    /// Builds a table over `project`, which must outlive it.
    explicit SubtitleTableModel(const core::Project& project, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    /// Rend un index, y compris hors table — ce qu’un test doit pouvoir
    /// fabriquer pour éprouver les gardes.
    using QAbstractTableModel::createIndex;

    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role) const override;

    /// Turns what a command reported into the signals a view listens for.
    ///
    /// One signal per run rather than one per row: `Change` carries runs since
    /// issue #45, and Qt refreshes a table by its top-left and bottom-right
    /// corners. A shift over four thousand subtitles costs one signal.
    ///
    /// A change of structure resets the model instead. Qt wants insertions and
    /// removals bracketed **before** they happen, and a session only reports
    /// them after — no command can predict them, since the hearing-impaired
    /// removal decides what it empties while it runs.
    void applied(std::span<const core::Change> changes);

private:
    /// The columns a change of that nature makes stale, as a closed span.
    [[nodiscard]] static std::pair<int, int> columnsFor(core::ChangeKind kind);

    const core::Project* m_project;
};

} // namespace subedit::gui
