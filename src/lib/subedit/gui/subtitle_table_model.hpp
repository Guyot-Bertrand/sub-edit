#pragma once

#include <QAbstractTableModel>
#include <QVariant>

#include <span>
#include <utility>
#include <vector>

/// **Declared rather than included, deliberately.** `moc` parses this header,
/// and it chokes on the C++20 library headers that `change.hpp` drags in
/// through `Selection` — its iterator pulls `<iterator>`, and `<concepts>`
/// follows. Declaring what the signatures need keeps `moc` out of all that; the
/// definitions come in the implementation file.
namespace subedit::core {
class Project;
class Session;
struct Change;
enum class ChangeKind;
enum class AnomalyKind;
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
/// **It edits through the session, and owns neither.** Issue #128 wrote the
/// opposite here — the model was to be handed what changed and nothing more —
/// and issue #129 had to undo it: Qt hands a finished cell edit to `setData`,
/// and routing it back out to whoever holds the session would cost either a
/// second translation of the same string or an interface class for one caller.
/// The frontier that mattered is still held, and by the compiler: what the
/// model reads is `session.project()`, which is constant, so the only road to
/// a change remains a command.
///
/// Two entrances, then, and they meet in the same place: `setData` builds the
/// command a cell edit calls for, and `applied` takes the report of a command
/// applied elsewhere — an undo, a dialog. Both end in `dataChanged`.
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

    /// Builds a table over `session`, which must outlive it.
    explicit SubtitleTableModel(core::Session& session, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    /// Says which cells an editor may open on.
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

    /// Carries a finished cell edit out, as a command.
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

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

    /// Says that everything on screen is stale.
    ///
    /// What a change of format calls for: the decimal separator every position
    /// is written with follows the format, so a « save as » that changes it
    /// makes every cell of two columns wrong at once. Not a `Change` — nothing
    /// about the document moved — so it does not go through `applied`.
    void refreshAll();

private:
    /// What is wrong with the document, by row.
    ///
    /// **Held, where the core computes on demand**, and the difference is a
    /// question of who asks: `scanAnomalies` walks the whole project, and a
    /// table asks its model once per visible cell. Recomputing there would
    /// make a repaint cost the file.
    ///
    /// Rebuilt after every change that could move a position — and after
    /// nothing else, because a text belongs to no anomaly.
    void rescanAnomalies();

public:
signals:
    /// Announces that the session may have moved.
    ///
    /// What an undo action listens to. It is **not** `dataChanged`: an edit
    /// that changes nothing has no row to refresh and must still be announced,
    /// because whoever recomputes the state of a menu has to be right after
    /// every operation, including the one that did nothing.
    void historyChanged();

private:
    /// The tint or the tooltip a row's anomalies call for, or nothing.
    [[nodiscard]] QVariant anomalyMark(const QModelIndex& index, int role) const;

    /// The anomalies of one row, most telling first, or empty.
    [[nodiscard]] std::span<const core::AnomalyKind> anomaliesAt(int row) const;

    /// The columns a change of that nature makes stale, as a closed span.
    [[nodiscard]] static std::pair<int, int> columnsFor(core::ChangeKind kind);

    core::Session* m_session;

    /// One entry per subtitle that carries at least one anomaly, ascending.
    std::vector<std::pair<int, std::vector<core::AnomalyKind>>> m_anomalies;
};

} // namespace subedit::gui
