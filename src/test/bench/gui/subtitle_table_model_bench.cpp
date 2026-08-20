// Ce que coûte la table, là où l'utilisateur le sent.
//
// **La promesse de l'adaptateur mince se mesure ici** : construire le modèle ne
// doit rien copier, et `data()` ne doit toucher que la ligne demandée. Gaupol
// recopie ses quatre mille sous-titres dans un magasin séparé ; nous affirmons
// que le nôtre n'en recopie aucun, et un chiffre vaut mieux qu'une affirmation.

#include <subedit/core/command/change.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QModelIndex>
#include <QVariant>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "full_length_project.hpp"

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::gui::SubtitleTableModel;
using subedit::test::fullLengthProject;

/// Ce qu'une fenêtre montre d'un coup, à la louche : Qt ne demande `data()` que
/// pour les cellules visibles, et c'est ce qui rend l'adaptateur tenable.
constexpr int kVisibleRows = 40;

} // namespace

TEST_CASE("building a table over a full-length file", "[benchmark]") {
    const Project project = fullLengthProject();

    BENCHMARK("construction du modèle sur 4000 sous-titres") {
        const SubtitleTableModel model{project};
        return model.rowCount({});
    };
}

TEST_CASE("reading the cells a window shows", "[benchmark]") {
    const Project project = fullLengthProject();
    const SubtitleTableModel model{project};

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
    // Ce pour quoi #45 est passée avant ce ticket : un décalage de tout le
    // fichier tient en une plage, donc en un signal, là où quatre mille indices
    // auraient demandé quatre mille appels ou un recollage.
    const Project project = fullLengthProject();
    SubtitleTableModel model{project};
    const std::vector<Change> whole = {
        Change{.kind = ChangeKind::Positions, .subtitles = Selection::all(project)}};

    BENCHMARK("rafraîchir après un décalage de 4000 sous-titres") {
        model.applied(whole);
        return whole.size();
    };
}
