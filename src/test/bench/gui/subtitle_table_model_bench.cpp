// Ce que coûte la table, là où l'utilisateur le sent.
//
// **La promesse de l'adaptateur mince se mesure ici** : construire le modèle ne
// doit rien copier, et `data()` ne doit toucher que la ligne demandée. Gaupol
// recopie ses quatre mille sous-titres dans un magasin séparé ; nous affirmons
// que le nôtre n'en recopie aucun, et un chiffre vaut mieux qu'une affirmation.

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

/// Ce qu'une fenêtre montre d'un coup, à la louche : Qt ne demande `data()` que
/// pour les cellules visibles, et c'est ce qui rend l'adaptateur tenable.
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
    // Ce pour quoi #45 est passée avant ce ticket : un décalage de tout le
    // fichier tient en une plage, donc en un signal, là où quatre mille indices
    // auraient demandé quatre mille appels ou un recollage.
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
    // Ce que coûte une frappe validée, depuis le `setData` que Qt appelle. Le
    // noyau mesure déjà la commande seule ; ce chiffre-ci lui ajoute ce que la
    // table met autour — la conversion de la chaîne, la comparaison qui écarte
    // une validation vide, le signal émis.
    //
    // **Une session réutilisée**, pour la raison qui vaut au benchmark de noyau
    // qui fait de même : l'édition est trop rapide pour qu'une copie de projet
    // par itération tienne en mémoire, et une fenêtre ouverte fait exactement
    // ça — taper dans un fichier qu'elle garde.
    const int middle = static_cast<int>(subedit::test::kSubtitleCount) / 2;

    BENCHMARK_ADVANCED("édition d'une cellule de texte")
    (Catch::Benchmark::Chronometer meter) {
        Session session{fullLengthProject()};
        SubtitleTableModel model{session};

        // Un texte différent à chaque tour : identique, il emprunterait le
        // chemin qui ne fait rien, et ce n'est pas celui qu'on mesure.
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
