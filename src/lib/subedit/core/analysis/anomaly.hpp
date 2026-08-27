#pragma once

#include <subedit/core/model/subtitle_index.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// What is wrong with a document, as a category.
///
/// **Distinct from `DiagnosticKind`, and the marker says why** — ADR 0018. A
/// diagnostic is what a *reading* ran into, and it points at a line; a line
/// only exists while a file is being read. An anomaly is what a *document is*,
/// and it points at a subtitle; an index survives an edition, and a table
/// highlights rows.
///
/// The three used to live among the diagnostics, where they were computed once
/// at read time and lost the moment anything moved.
enum class AnomalyKind {
    EndBeforeStart,       ///< a subtitle that ends before it starts
    OverlappingSubtitles, ///< a subtitle starting before the previous one ends
    OutOfOrder,           ///< a subtitle starting before the previous one starts
};

/// One thing wrong with a document, and which subtitle carries it.
struct Anomaly {
    AnomalyKind kind;
    SubtitleIndex index;

    [[nodiscard]] friend bool operator==(const Anomaly&, const Anomaly&) = default;
};

/// Returns everything wrong with `project`, in the order of the subtitles.
///
/// **An inference, and it lives where the inferences live** — issue #227. « This
/// subtitle ends before it starts » is not a datum the file carries: it is a
/// judgement passed on it, exactly like the frame rate deduction next door.
/// This landed in `core/model/` for want of a better place, before
/// `core/analysis/` existed; ADR 0021 created the place and left the move to be
/// made, since it buys nothing while there is only one analysis. There are
/// several now.
///
/// A pure query, computed on demand: the model never keeps it, so it is never
/// stale. One subtitle may carry several anomalies — starting before the
/// previous one ends *and* before it starts — and each is reported on its own,
/// because each is fixed differently.
///
/// **The index reported is the one that is out of place**, not the one it is
/// out of place against: that is the row an interface has to mark, and the
/// subtitle a report has to name.
///
/// Equal starts are not disorder — neither precedes the other — though they
/// may well overlap, which is reported as such.
[[nodiscard]] std::vector<Anomaly> scanAnomalies(const Project& project);

} // namespace subedit::core
