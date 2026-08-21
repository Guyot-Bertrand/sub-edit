#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QModelIndex>
#include <QSignalSpy>
#include <QVariant>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <utility>
#include <vector>

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::gui::SubtitleTableModel;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, const char* text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = text};
}

[[nodiscard]] Project threeSubtitles() {
    Project project;
    project.setSubtitles(
        {at(1000, 2500, "Un."), at(3000, 4000, "Deux."), at(5000, 6200, "Trois.")});
    return project;
}

/// A report of one change, named so that a span can point at it.
[[nodiscard]] std::vector<Change> reporting(ChangeKind kind, const Selection& subtitles) {
    return {Change{.kind = kind, .subtitles = subtitles}};
}

[[nodiscard]] std::string textAt(const SubtitleTableModel& model, int row, int column) {
    return model.data(model.index(row, column), Qt::DisplayRole).toString().toStdString();
}

} // namespace

TEST_CASE("the table has one row per subtitle and five columns", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK(model.rowCount({}) == 3);
    CHECK(model.columnCount({}) == 5);
}

TEST_CASE("the number column counts from one and is never stored", "[gui][GUI-TABLE-01]") {
    // A rank, not a datum: Gaupol computes it from the row too, and storing it
    // would mean renumbering every line after an insertion.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK(textAt(model, 0, 0) == "1");
    CHECK(textAt(model, 2, 0) == "3");
}

TEST_CASE("the table shows start, end, duration and text", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK(textAt(model, 0, 1) == "00:00:01,000");
    CHECK(textAt(model, 0, 2) == "00:00:02,500");
    CHECK(textAt(model, 0, 3) == "00:00:01,500");
    CHECK(textAt(model, 0, 4) == "Un.");
}

TEST_CASE("the decimal mark follows the format the file will be written in",
          "[gui][GUI-TABLE-01]") {
    // So that what is read on screen is what will end up in the file: SubRip
    // writes a comma, WebVTT a period.
    Project project = threeSubtitles();
    project.setSourceFile(subedit::core::SourceFile{.format = SubtitleFormat::WebVtt});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    CHECK(textAt(model, 0, 1) == "00:00:01.000");
}

TEST_CASE("every column says what it holds", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    const auto header = [&model](int column) {
        return model.headerData(column, Qt::Horizontal, Qt::DisplayRole).toString().toStdString();
    };

    CHECK(header(0) == "#");
    CHECK(header(1) == "Start");
    CHECK(header(2) == "End");
    CHECK(header(3) == "Duration");
    CHECK(header(4) == "Text");
}

TEST_CASE("an index outside the table holds nothing", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    // `index()` refuse de fabriquer hors table et rend un index invalide :
    // c'est la première garde qu'il éprouve, pas celle du rang. Atteindre
    // celle-là demande `createIndex`, qui ne demande rien à personne — et c'est
    // pour ça que l'en-tête l'expose.
    CHECK_FALSE(model.data(model.index(9, 0), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data(model.createIndex(9, 0), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data({}, Qt::DisplayRole).isValid());
}

TEST_CASE("a change of positions refreshes the columns that show them", "[gui][GUI-TABLE-01]") {
    // The whole point of a command reporting what it touched: three rows moved
    // out of four thousand should cost three refreshes, not a redraw.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<Change> report =
        reporting(ChangeKind::Positions,
                  Selection::range(SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2)));
    model.applied(report);

    REQUIRE(refreshed.count() == 1);
    const QModelIndex topLeft = refreshed.at(0).at(0).toModelIndex();
    const QModelIndex bottomRight = refreshed.at(0).at(1).toModelIndex();
    CHECK(topLeft.row() == 1);
    CHECK(topLeft.column() == 1);
    CHECK(bottomRight.row() == 2);
    CHECK(bottomRight.column() == 3);
}

TEST_CASE("a change of text refreshes the text column alone", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<Change> report =
        reporting(ChangeKind::MainText,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
    model.applied(report);

    REQUIRE(refreshed.count() == 1);
    CHECK(refreshed.at(0).at(0).toModelIndex().column() == 4);
    CHECK(refreshed.at(0).at(1).toModelIndex().column() == 4);
}

TEST_CASE("one signal per run, not per row", "[gui][GUI-TABLE-01]") {
    // What issue #45 was moved ahead of this ticket for: `Change` carries runs,
    // and Qt refreshes by corners. Four thousand rows in one run cost one
    // signal.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<SubtitleIndex> scattered = {SubtitleIndex::fromValue(0),
                                                  SubtitleIndex::fromValue(2)};
    const std::vector<Change> report = reporting(ChangeKind::MainText, Selection::of(scattered));
    model.applied(report);

    CHECK(refreshed.count() == 2);
}

TEST_CASE("a change of structure rebuilds the table", "[gui][GUI-TABLE-01]") {
    // Qt wants a structural change bracketed *before* it happens, and a session
    // only reports it after — no command can predict it. The reset is the price
    // of that order, and ADR 0019 carries its trigger for reconsidering.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy reset{&model, &SubtitleTableModel::modelReset};

    const std::vector<Change> report =
        reporting(ChangeKind::Removal,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
    model.applied(report);

    CHECK(reset.count() == 1);
}

TEST_CASE("a report with nothing in it refreshes nothing", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    model.applied(std::vector<Change>{});

    CHECK(refreshed.count() == 0);
}

TEST_CASE("a column outside the table holds nothing", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK_FALSE(model.data(model.index(0, 9), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data(model.createIndex(0, 9), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(9, Qt::Horizontal, Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(-1, Qt::Horizontal, Qt::DisplayRole).isValid());
}

TEST_CASE("a cell before the first one holds nothing", "[gui][GUI-TABLE-01]") {
    // Qt builds indices, and a caller can build a wrong one. Answering empty
    // beats reading whatever lies before the vector.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK_FALSE(model.data(model.createIndex(-1, 0), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data(model.createIndex(0, -1), Qt::DisplayRole).isValid());
}

TEST_CASE("only the horizontal header says anything", "[gui][GUI-TABLE-01]") {
    // The rows are numbered by a column of their own, so Qt's own row header is
    // hidden and has nothing to say.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK_FALSE(model.headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(0, Qt::Horizontal, Qt::ToolTipRole).isValid());
}

TEST_CASE("a cell holds nothing for a role the table does not serve", "[gui][GUI-TABLE-01]") {
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    CHECK_FALSE(model.data(model.index(0, 0), Qt::DecorationRole).isValid());
}

TEST_CASE("a change of translation refreshes nothing, for now", "[gui][GUI-TABLE-01]") {
    // No column shows it: the translation document exists in the model since
    // phase 1, and the interface builds it in phase 11. Reporting it is right;
    // refreshing a column that is not there would not be.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<Change> report =
        reporting(ChangeKind::TranslationText,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
    model.applied(report);

    CHECK(refreshed.count() == 0);
}

TEST_CASE("a reordering refreshes every column", "[gui][GUI-TABLE-01]") {
    // The numbers move with the rows, so the leftmost column goes stale too —
    // it is the one place where the rank being computed rather than stored
    // shows.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<Change> report =
        reporting(ChangeKind::Reordering,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(2)));
    model.applied(report);

    REQUIRE(refreshed.count() == 1);
    CHECK(refreshed.at(0).at(0).toModelIndex().column() == 0);
    CHECK(refreshed.at(0).at(1).toModelIndex().column() == 4);
}

// L'édition en place — issue #129.
//
// Le modèle édite, et c'est un choix : la vue appelle le délégué, le délégué
// appelle `setData`, et détourner ce chemin coûterait soit une traduction en
// double, soit une classe de plus. Ce que le modèle sait faire est ce qu'il
// faut ici — formater une position et la relire —, et la commande qu'il
// fabrique part dans la session comme n'importe quelle autre.

namespace {

/// Ce qu'une cellule vaut après édition, relu par la vue.
[[nodiscard]] bool edits(SubtitleTableModel& model, int row, int column, const char* typed) {
    return model.setData(model.index(row, column), QString::fromUtf8(typed), Qt::EditRole);
}

[[nodiscard]] bool editable(const SubtitleTableModel& model, int column) {
    return model.flags(model.index(0, column)).testFlag(Qt::ItemIsEditable);
}

} // namespace

TEST_CASE("the two positions and the text are what a cell edit can reach", "[gui][GUI-EDIT-01]") {
    // Le numéro est un rang et la durée une différence : ni l'un ni l'autre
    // n'est une donnée, et le noyau n'a pas de commande qui les poserait.
    Session session{threeSubtitles()};
    const SubtitleTableModel model{session};

    // Un index invalide n'est pas une cellule : il rend les fanions hérités,
    // sans le droit d'édition qu'aucune vue ne lui demanderait.
    CHECK_FALSE(model.flags({}).testFlag(Qt::ItemIsEditable));

    CHECK_FALSE(editable(model, 0));
    CHECK(editable(model, 1));
    CHECK(editable(model, 2));
    CHECK_FALSE(editable(model, 3));
    CHECK(editable(model, 4));
}

TEST_CASE("editing a text cell changes that subtitle and nothing else", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 1, 4, "Deux, autrement."));

    CHECK(textAt(model, 1, 4) == "Deux, autrement.");
    CHECK(textAt(model, 0, 4) == "Un.");
    CHECK(textAt(model, 1, 1) == "00:00:03,000");
    CHECK(textAt(model, 1, 2) == "00:00:04,000");
}

TEST_CASE("a multiline text is read back exactly as it was typed", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 0, 4, "Deux lignes,\net la seconde."));

    CHECK(textAt(model, 0, 4) == "Deux lignes,\net la seconde.");
}

TEST_CASE("an edited cell makes exactly one undoable action", "[gui][GUI-EDIT-01]") {
    // La question de groupement laissée ouverte par la phase 2 se referme ici,
    // et sans mécanisme : un délégué valide une fois, donc une cellule éditée
    // produit une commande. Il n'y a rien à grouper.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 0, 4, "Autre chose."));

    CHECK(session.undoableCount() == 1);
    CHECK(session.hasUnsavedChanges(Document::Main));
}

TEST_CASE("undoing an edited cell puts back what was there", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    REQUIRE(edits(model, 0, 4, "Autre chose."));

    model.applied(session.undo());

    CHECK(textAt(model, 0, 4) == "Un.");
}

TEST_CASE("editing a cell refreshes the column it changed", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    REQUIRE(edits(model, 1, 4, "Deux, autrement."));

    REQUIRE(refreshed.count() == 1);
    const QModelIndex topLeft = refreshed.at(0).at(0).toModelIndex();
    CHECK(topLeft.row() == 1);
    CHECK(topLeft.column() == 4);
}

TEST_CASE("the number and the duration refuse an edit", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK_FALSE(edits(model, 0, 0, "7"));
    CHECK_FALSE(edits(model, 0, 3, "00:00:09,000"));

    CHECK(session.undoableCount() == 0);
}

TEST_CASE("an edit outside the table, or in another role, changes nothing", "[gui][GUI-EDIT-01]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    // Les deux premières frappent la garde d'index, les deux suivantes celles
    // du rang et de la colonne : `createIndex` fabrique ce que `index()`
    // refuse, et c'est ce qui laisse éprouver le fond des gardes.
    CHECK_FALSE(edits(model, 9, 4, "hors table"));
    CHECK_FALSE(model.setData(model.index(0, 4), QStringLiteral("affichage"), Qt::DisplayRole));
    CHECK_FALSE(model.setData(model.createIndex(9, 4), QStringLiteral("hors rang"), Qt::EditRole));
    CHECK_FALSE(
        model.setData(model.createIndex(0, 9), QStringLiteral("hors colonne"), Qt::EditRole));

    CHECK(session.undoableCount() == 0);
}

TEST_CASE("editing a start reads a permissive timestamp", "[gui][GUI-EDIT-02]") {
    // Permissif comme les fichiers réels l'exigent, et donc comme une saisie
    // l'est aussi : heures omises, un chiffre par champ, une seule décimale,
    // point pour virgule.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 0, 1, "1:02.5"));

    CHECK(textAt(model, 0, 1) == "00:01:02,500");
}

TEST_CASE("editing an end moves that end alone, and the duration follows", "[gui][GUI-EDIT-02]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 0, 2, "00:00:09,000"));

    CHECK(textAt(model, 0, 1) == "00:00:01,000");
    CHECK(textAt(model, 0, 2) == "00:00:09,000");
    CHECK(textAt(model, 0, 3) == "00:00:08,000");
}

TEST_CASE("a position edit refreshes the two ends and the duration", "[gui][GUI-EDIT-02]") {
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    REQUIRE(edits(model, 0, 1, "00:00:00,500"));

    REQUIRE(refreshed.count() == 1);
    CHECK(refreshed.at(0).at(0).toModelIndex().column() == 1);
    CHECK(refreshed.at(0).at(1).toModelIndex().column() == 3);
}

TEST_CASE("an unreadable position leaves the cell as it was", "[gui][GUI-EDIT-02]") {
    // Plutôt qu'inventer une position. Soixante-dix minutes n'en est pas une :
    // `Timestamp::parse` refuse un champ hors bornes, et c'est ce refus qui
    // remonte jusqu'ici.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK_FALSE(edits(model, 0, 1, "00:70:00,000"));
    CHECK_FALSE(edits(model, 0, 2, "bientôt"));

    CHECK(textAt(model, 0, 1) == "00:00:01,000");
    CHECK(textAt(model, 0, 2) == "00:00:02,500");
    CHECK(session.undoableCount() == 0);
}

TEST_CASE("validating a text that did not change writes nothing to the history",
          "[gui][GUI-EDIT-03]") {
    // Ouvrir une cellule, ne rien taper, appuyer sur Entrée. L'édition est
    // acceptée — il n'y a pas d'erreur à signaler — et n'a rien produit.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    CHECK(edits(model, 0, 4, "Un."));

    CHECK(session.undoableCount() == 0);
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));
    CHECK(refreshed.count() == 0);
}

TEST_CASE("validating a position that did not change writes nothing to the history",
          "[gui][GUI-EDIT-03]") {
    // Comparé sur la position et non sur la chaîne : `1.0` et `00:00:01,000`
    // sont le même instant écrit deux fois.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};

    CHECK(edits(model, 0, 1, "0:01.0"));

    CHECK(session.undoableCount() == 0);
    CHECK_FALSE(session.hasUnsavedChanges(Document::Main));
}

TEST_CASE("every accepted edit is announced, even one that changed nothing", "[gui][GUI-UNDO-01]") {
    // Le contrat que l'en-tête annonce, et il ne se lit pas par ses effets :
    // une édition qui ne change rien laisse l'historique où il était, donc
    // quiconque recalculerait un menu trouverait le même état. C'est le signal
    // lui-même qui est promis, et c'est donc lui qu'on éprouve.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy announced{&model, &SubtitleTableModel::historyChanged};

    REQUIRE(edits(model, 0, 4, "Autre chose."));
    CHECK(announced.count() == 1);

    REQUIRE(edits(model, 0, 4, "Autre chose."));
    CHECK(announced.count() == 2);

    REQUIRE(edits(model, 0, 1, "0:01.0"));
    CHECK(announced.count() == 3);
}

TEST_CASE("a refused edit announces nothing", "[gui][GUI-UNDO-01]") {
    // Rien n'a été tenté que le modèle ait accepté : il n'y a rien à annoncer,
    // et annoncer quand même ferait du signal un compteur de frappes.
    Session session{threeSubtitles()};
    SubtitleTableModel model{session};
    const QSignalSpy announced{&model, &SubtitleTableModel::historyChanged};

    CHECK_FALSE(edits(model, 0, 1, "bientôt"));
    CHECK_FALSE(edits(model, 0, 0, "7"));

    CHECK(announced.count() == 0);
}
