// Ce sur quoi une opération porte — issue #132.
//
// **La sélection, ou tout le fichier.** C'est ce que la phase 4 avait renvoyé
// ici, et le pont est écrit une fois pour les quatre dialogues.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>

#include <QItemSelectionModel>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::gui::SubtitleTableModel;
using subedit::gui::targetOf;

[[nodiscard]] Subtitle at(std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = "x"};
}

[[nodiscard]] Project four() {
    Project project;
    project.setSubtitles({at(1000), at(3000), at(5000), at(7000)});
    return project;
}

/// Les indices d'une sélection, à plat, pour qu'un test les nomme.
[[nodiscard]] std::vector<std::size_t> valuesOf(const Selection& selection) {
    std::vector<std::size_t> values;
    for (const SubtitleIndex index : selection.indices())
        values.push_back(index.value());
    return values;
}

/// Coche une ligne dans un modèle de sélection.
void select(QItemSelectionModel& selection, const SubtitleTableModel& model, int row) {
    selection.select(model.index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("nothing selected means the whole file", "[gui][GUI-SHIFT-01]") {
    // Le défaut qui rend les dialogues utilisables : personne ne sélectionne
    // quatre mille lignes pour décaler un fichier entier.
    Session session{four()};
    SubtitleTableModel model{session};
    const QItemSelectionModel selection{&model};

    CHECK(valuesOf(targetOf(selection, session.project())) == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("what is selected is what an operation touches", "[gui][GUI-SHIFT-01]") {
    Session session{four()};
    SubtitleTableModel model{session};
    QItemSelectionModel selection{&model};

    select(selection, model, 1);
    select(selection, model, 2);

    CHECK(valuesOf(targetOf(selection, session.project())) == std::vector<std::size_t>{1, 2});
}

TEST_CASE("a scattered selection keeps its holes", "[gui][GUI-SHIFT-01]") {
    Session session{four()};
    SubtitleTableModel model{session};
    QItemSelectionModel selection{&model};

    select(selection, model, 0);
    select(selection, model, 3);

    CHECK(valuesOf(targetOf(selection, session.project())) == std::vector<std::size_t>{0, 3});
}

TEST_CASE("selecting every row is the whole file, and says so once", "[gui][GUI-SHIFT-01]") {
    // Le même résultat que la sélection vide, par un autre chemin : rien ne
    // distingue « tout coché » de « rien coché », et c'est bien ainsi.
    Session session{four()};
    SubtitleTableModel model{session};
    QItemSelectionModel selection{&model};

    for (int row = 0; row < 4; ++row)
        select(selection, model, row);

    CHECK(valuesOf(targetOf(selection, session.project())) == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("a selection over an empty project is empty", "[gui][GUI-SHIFT-01]") {
    Session session{Project{}};
    SubtitleTableModel model{session};
    const QItemSelectionModel selection{&model};

    CHECK(valuesOf(targetOf(selection, session.project())).empty());
}
