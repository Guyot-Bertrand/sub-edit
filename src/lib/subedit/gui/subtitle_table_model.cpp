#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QBrush>
#include <QColor>
#include <QString>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

using core::AnomalyKind;
using core::Change;
using core::ChangeKind;
using core::DecimalMark;
using core::SubtitleFormat;

/// Which anomaly a row is named and tinted by when it carries several.
///
/// **A subtitle in disorder almost always overlaps too**, and that is
/// arithmetic rather than coincidence: starting before its predecessor starts
/// means starting before it ends, unless the predecessor is itself broken.
/// Taking whichever came first would therefore have shown the overlap and
/// never the disorder.
///
/// So the order is the order of repair. A subtitle broken in itself is wrong
/// whatever its neighbours do. One in the wrong place is put back, and its
/// overlap goes with it. An overlap alone is the one that is only about timing.
[[nodiscard]] int repairOrderOf(AnomalyKind kind) {
    switch (kind) {
    case AnomalyKind::EndBeforeStart:
        return 0;
    case AnomalyKind::OutOfOrder:
        return 1;
    case AnomalyKind::OverlappingSubtitles:
        return 2;
    }

    std::unreachable();
}

/// The tint a kind of anomaly paints its positions with.
///
/// **Translucent on purpose.** The window follows the desktop's palette, which
/// may be light or dark; a solid colour would be right against one and unread-
/// able against the other. An alpha wash tints whatever is underneath.
///
/// Three hues because the three are repaired differently — a subtitle broken in
/// itself, one that runs into its neighbour, one that is in the wrong place.
/// How much of the tint reaches the eye, out of 255.
///
/// Enough to be seen at a glance on a table of four thousand rows, little
/// enough that the text stays readable over it, light or dark.
constexpr int kWash = 64;

/// Red for a subtitle broken in itself, amber for an overlap, blue for a line
/// in the wrong place. Three hues that are told apart even by an eye that does
/// not tell red from green.
constexpr QColor kBrokenTint{200, 40, 40, kWash};
constexpr QColor kOverlapTint{220, 140, 0, kWash};
constexpr QColor kDisorderTint{70, 90, 210, kWash};

/// Green for the row a playing film is showing.
///
/// A fourth hue, told apart from the three above by an eye that does not tell
/// red from green — it is the only one of the four that is not warm, and the
/// only one that moves.
constexpr QColor kShowingTint{40, 160, 90, kWash};

[[nodiscard]] QBrush tintOf(AnomalyKind kind) {
    switch (kind) {
    case AnomalyKind::EndBeforeStart:
        return QBrush{kBrokenTint};
    case AnomalyKind::OverlappingSubtitles:
        return QBrush{kOverlapTint};
    case AnomalyKind::OutOfOrder:
        return QBrush{kDisorderTint};
    }

    std::unreachable();
}

/// The mark the file will be written with, so that the separator on screen is
/// the one the file will hold.
///
/// **The separator, and that alone.** The table always writes the hours, where
/// WebVTT leaves them out below one hour; a column whose width followed the
/// position would be worse to read than one that is merely not the file's exact
/// spelling.
[[nodiscard]] DecimalMark markOf(const core::Project& project) {
    return project.sourceFile().format == SubtitleFormat::WebVtt ? DecimalMark::Period
                                                                 : DecimalMark::Comma;
}

[[nodiscard]] QString written(core::Timestamp position, DecimalMark mark) {
    return QString::fromStdString(position.format(mark));
}

} // namespace

SubtitleTableModel::SubtitleTableModel(core::Session& session, QObject* parent)
    : QAbstractTableModel(parent), m_session(&session) {
    rescanAnomalies();
}

void SubtitleTableModel::rescanAnomalies() {
    m_anomalies.clear();

    // Grouped by subtitle: `scanAnomalies` returns them in the order of the
    // subtitles, so one index arrives in a row.
    for (const core::Anomaly& anomaly : core::scanAnomalies(m_session->project())) {
        const int row = static_cast<int>(anomaly.index.value());
        if (m_anomalies.empty() || m_anomalies.back().first != row)
            m_anomalies.emplace_back(row, std::vector<AnomalyKind>{});

        m_anomalies.back().second.push_back(anomaly.kind);
    }

    // What is repaired first is read first, and gives the tint.
    for (auto& [row, kinds] : m_anomalies)
        std::ranges::sort(kinds, {}, repairOrderOf);
}

std::span<const AnomalyKind> SubtitleTableModel::anomaliesAt(int row) const {
    // **By bisection, and not by sweeping.** `data()` asks the question once
    // per visible cell; on a badly damaged file, a linear search would make
    // every cell pay the number of anomalies of the document. The entries are
    // ordered by ascending row, which is all it takes.
    const auto found = std::ranges::lower_bound(
        m_anomalies, row, {}, &std::pair<int, std::vector<AnomalyKind>>::first);
    if (found == m_anomalies.end() || found->first != row)
        return {};

    return found->second;
}

int SubtitleTableModel::rowCount(const QModelIndex& parent) const {
    // A table has no children: a valid parent asks for the rows *under* a cell,
    // and there are none.
    return parent.isValid() ? 0 : static_cast<int>(m_session->project().count());
}

int SubtitleTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : kColumnCount;
}

QVariant SubtitleTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    if (role == Qt::BackgroundRole || role == Qt::ToolTipRole) {
        QVariant marked = anomalyMark(index, role);
        if (marked.isValid() || role != Qt::BackgroundRole)
            return marked;

        // Nothing wrong with this row, and a film is showing it.
        return index.row() == m_showing ? QVariant{QBrush{kShowingTint}} : QVariant{};
    }

    // Both roles return the same thing, and that is what makes the inherited
    // delegates work: `setEditorData` reads `Qt::EditRole`, and a position
    // editor has to open on the timestamp as it is written on screen.
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    if (index.row() < 0 || index.row() >= rowCount({}))
        return {};

    // A column outside the table is a caller's mistake, not a case to serve:
    // it returns an empty value rather than choosing for them.
    if (index.column() < 0 || index.column() >= kColumnCount)
        return {};

    const core::SubtitleIndex position =
        core::SubtitleIndex::fromValue(static_cast<std::size_t>(index.row()));
    const core::Subtitle& subtitle = m_session->project().subtitleAt(position);
    const DecimalMark mark = markOf(m_session->project());

    // Cast, and not switched on the integer: the guard above brought the value
    // back into the domain of the enumeration, and that is what lets the
    // compiler check that the five columns are handled.
    switch (static_cast<Column>(index.column())) {
    case Number:
        // Computed, never stored: an insertion would otherwise renumber every
        // line after it.
        return QString::number(position.number());
    case Start:
        return written(subtitle.start, mark);
    case End:
        return written(subtitle.end, mark);
    case Duration:
        // Derived, and read-only for that reason: the core has no command that
        // sets a duration, and inventing one here would be core work smuggled
        // into an interface.
        return written(core::Timestamp::origin() + subtitle.duration(), mark);
    case Text:
        return QString::fromStdString(subtitle.mainText);
    }

    // The five columns are handled, the guard above rules out the rest, and
    // the compiler checks that the enumeration is exhausted.
    std::unreachable();
}

QVariant SubtitleTableModel::anomalyMark(const QModelIndex& index, int role) const {
    // The positions and what is derived from them. Tinting the text would
    // suggest it has something to do with an anomaly that is only about time.
    const int column = index.column();
    if (column != Start && column != End && column != Duration)
        return {};

    const std::span<const AnomalyKind> kinds = anomaliesAt(index.row());
    if (kinds.empty())
        return {};

    // The first in the order of repair governs the tint.
    if (role == Qt::BackgroundRole)
        return tintOf(kinds.front());

    QString named;
    for (const AnomalyKind kind : kinds) {
        if (!named.isEmpty())
            named += QStringLiteral("\n");
        named += QString::fromUtf8(core::nameOf(kind));
    }
    return named;
}

Qt::ItemFlags SubtitleTableModel::flags(const QModelIndex& index) const {
    const Qt::ItemFlags inherited = QAbstractTableModel::flags(index);
    if (!index.isValid())
        return inherited;

    // The number is a rank and the duration a difference: no command of the
    // core sets either, so no editor opens on them.
    const int column = index.column();
    if (column != Start && column != End && column != Text)
        return inherited;

    return inherited | Qt::ItemIsEditable;
}

bool SubtitleTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (index.row() < 0 || index.row() >= rowCount({}))
        return false;

    if (index.column() < 0 || index.column() >= kColumnCount)
        return false;

    const core::SubtitleIndex position =
        core::SubtitleIndex::fromValue(static_cast<std::size_t>(index.row()));
    const core::Subtitle& subtitle = m_session->project().subtitleAt(position);
    std::string typed = value.toString().toStdString();

    switch (static_cast<Column>(index.column())) {
    case Number:
    case Duration:
        // `flags()` has already told the view, which will open no editor here.
        // The refusal is for whoever calls `setData` without a view.
        return false;
    case Start:
    case End: {
        const bool start = index.column() == Start;
        const std::optional<core::Timestamp> wanted = core::Timestamp::parse(typed);

        // **Unreadable leaves the cell as it was** rather than inventing a
        // position. Getting a position wrong is silent: nothing on screen tells
        // a badly timed subtitle from a well timed one.
        if (!wanted.has_value())
            return false;

        if (*wanted == (start ? subtitle.start : subtitle.end)) {
            emit historyChanged();
            return true;
        }

        applied(m_session->apply(std::make_unique<core::SetPositionCommand>(
            m_session->project(),
            position,
            start ? core::Boundary::Start : core::Boundary::End,
            *wanted)));
        return true;
    }
    case Text:
        // A validation that changes nothing produces no command: the history
        // has no business remembering a keystroke followed by an `Enter` on an
        // identical text, or undoing would fill up with non-events.
        if (typed == subtitle.mainText) {
            emit historyChanged();
            return true;
        }

        applied(m_session->apply(std::make_unique<core::SetTextCommand>(
            m_session->project(), position, core::Document::Main, std::move(typed))));
        return true;
    }

    std::unreachable();
}

QVariant SubtitleTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= kColumnCount)
        return {};

    switch (static_cast<Column>(section)) {
    case Number:
        return QStringLiteral("#");
    case Start:
        return QStringLiteral("Start");
    case End:
        return QStringLiteral("End");
    case Duration:
        return QStringLiteral("Duration");
    case Text:
        return QStringLiteral("Text");
    }

    std::unreachable();
}

std::pair<int, int> SubtitleTableModel::columnsFor(ChangeKind kind) {
    switch (kind) {
    case ChangeKind::Positions:
        // The duration among them: it is derived from the two positions, so it
        // goes stale with them.
        return {Start, Duration};
    case ChangeKind::MainText:
        return {Text, Text};
    case ChangeKind::TranslationText:
    case ChangeKind::Insertion:
    case ChangeKind::Removal:
        // Either no column shows it — the translation arrives in phase 11 — or
        // the change is structural, and `applied` has already reset the model
        // rather than asking.
        return {-1, -1};
    case ChangeKind::Reordering:
        return {Number, Text};
    }

    std::unreachable();
}

void SubtitleTableModel::refreshAll() {
    if (rowCount({}) == 0)
        return;

    emit dataChanged(index(0, 0), index(rowCount({}) - 1, kColumnCount - 1));
}

void SubtitleTableModel::applied(std::span<const Change> changes) {
    const bool structural = std::ranges::any_of(changes, [](const Change& change) {
        return change.kind == ChangeKind::Insertion || change.kind == ChangeKind::Removal;
    });

    if (structural) {
        beginResetModel();
        rescanAnomalies();
        endResetModel();
        emit historyChanged();
        return;
    }

    // A text belongs to no anomaly; everything else may move a position, so
    // everything else calls for a recount.
    const bool positional = std::ranges::any_of(changes, [](const Change& change) {
        return change.kind == ChangeKind::Positions || change.kind == ChangeKind::Reordering;
    });
    if (positional)
        rescanAnomalies();

    for (const Change& change : changes) {
        const auto [first, last] = columnsFor(change.kind);
        if (first < 0)
            continue;

        for (const core::IndexRange& run : change.subtitles.ranges()) {
            emit dataChanged(index(static_cast<int>(run.first.value()), first),
                             index(static_cast<int>(run.last.value()), last));
        }
    }

    // Announced once per operation, and unconditionally: an undo that reports
    // nothing has emptied a stack all the same.
    emit historyChanged();
}

void SubtitleTableModel::setShowing(std::optional<core::SubtitleIndex> shown) {
    // Named `shown` and not `index`: the parameter would otherwise hide
    // `QAbstractTableModel::index`, which the repaint below needs.
    const int row = shown.has_value() ? static_cast<int>(shown->value()) : -1;
    if (row == m_showing)
        return;

    // Both rows are repainted, and only they: the one that stops being shown
    // and the one that starts. Repainting the table ten times a second — which
    // is what the ticker asks for — would undo what ADR 0019 bought.
    const int left = m_showing;
    m_showing = row;
    for (const int touched : {left, row}) {
        if (touched < 0 || touched >= rowCount({}))
            continue;

        Q_EMIT dataChanged(index(touched, 0), index(touched, Text), {Qt::BackgroundRole});
    }
}

} // namespace subedit::gui
