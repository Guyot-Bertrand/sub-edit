// What the table costs, where the user feels it.
//
// **The promise of the thin adapter is measured here**: building the model must
// copy nothing, and `data()` must touch only the row it was asked for. Gaupol
// copies its four thousand subtitles into a separate store; we claim ours
// copies none, and a number beats a claim.

#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QLatin1Char>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_chronometer.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "full_length_project.hpp"

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::gui::SubtitleTableModel;
using subedit::test::fullLengthProject;

/// What a window shows at once, roughly: Qt asks `data()` only for the visible
/// cells, and that is what makes the adapter tenable.
constexpr int kVisibleRows = 40;

} // namespace

TEST_CASE("building a table over a full-length file", "[benchmark]") {
    Session session{fullLengthProject()};

    BENCHMARK("construction du modèle sur 4000 sous-titres") {
        const SubtitleTableModel model{session};
        return model.rowCount({});
    };
}

TEST_CASE("reading the cells a window shows", "[benchmark]") {
    Session session{fullLengthProject()};
    const SubtitleTableModel model{session};

    BENCHMARK("une fenêtre de 40 lignes, cinq colonnes") {
        int seen = 0;
        for (int row = 0; row < kVisibleRows; ++row) {
            for (int column = 0; column < SubtitleTableModel::kColumnCount; ++column)
                seen += model.data(model.index(row, column), Qt::DisplayRole).isValid() ? 1 : 0;
        }
        return seen;
    };
}

TEST_CASE("turning a whole-file change into signals", "[benchmark]") {
    // What #45 went before this ticket for: a shift of the whole file fits in
    // one run, therefore in one signal, where four thousand indices would have
    // called for four thousand emissions or a regluing.
    Session session{fullLengthProject()};
    SubtitleTableModel model{session};
    const std::vector<Change> whole = {
        Change{.kind = ChangeKind::Positions, .subtitles = Selection::all(session.project())}};

    BENCHMARK("rafraîchir après un décalage de 4000 sous-titres") {
        model.applied(whole);
        return whole.size();
    };
}

TEST_CASE("carrying a cell edit out", "[benchmark]") {
    // What a validated keystroke costs, from the `setData` Qt calls. The core
    // already measures the command alone; this number adds what the table puts
    // around it — the conversion of the string, the comparison that rules out
    // an empty validation, the signal emitted.
    //
    // **One session, reused**, for the reason that holds for the core benchmark
    // that does the same: an edit is too fast for a copy of the project per
    // iteration to fit in memory, and an open window does exactly this — typing
    // into a file it keeps.
    const int middle = static_cast<int>(subedit::test::kSubtitleCount) / 2;

    BENCHMARK_ADVANCED("édition d'une cellule de texte")
    (Catch::Benchmark::Chronometer meter) {
        Session session{fullLengthProject()};
        SubtitleTableModel model{session};

        // A different text on every round: an identical one would take the
        // road that does nothing, and that is not the one being measured.
        meter.measure([&](int run) {
            model.setData(model.index(middle, SubtitleTableModel::Text),
                          QStringLiteral("Autre ") + QString::number(run),
                          Qt::EditRole);
            return session.undoableCount();
        });
    };

    BENCHMARK_ADVANCED("édition d'une cellule de position")
    (Catch::Benchmark::Chronometer meter) {
        Session session{fullLengthProject()};
        SubtitleTableModel model{session};

        meter.measure([&](int run) {
            model.setData(model.index(middle, SubtitleTableModel::Start),
                          QStringLiteral("00:00:%1,000").arg(run % 60, 2, 10, QLatin1Char('0')),
                          Qt::EditRole);
            return session.undoableCount();
        });
    };
}
