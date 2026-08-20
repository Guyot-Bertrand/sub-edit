#include <subedit/core/command/change.hpp>
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
#include <vector>

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Project;
using subedit::core::Selection;
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
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK(model.rowCount({}) == 3);
    CHECK(model.columnCount({}) == 5);
}

TEST_CASE("the number column counts from one and is never stored", "[gui][GUI-TABLE-01]") {
    // A rank, not a datum: Gaupol computes it from the row too, and storing it
    // would mean renumbering every line after an insertion.
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK(textAt(model, 0, 0) == "1");
    CHECK(textAt(model, 2, 0) == "3");
}

TEST_CASE("the table shows start, end, duration and text", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

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
    const SubtitleTableModel model{project};

    CHECK(textAt(model, 0, 1) == "00:00:01.000");
}

TEST_CASE("every column says what it holds", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    const auto header = [&model](int column) {
        return model.headerData(column, Qt::Horizontal, Qt::DisplayRole).toString().toStdString();
    };

    CHECK(header(0) == "N°");
    CHECK(header(1) == "Début");
    CHECK(header(2) == "Fin");
    CHECK(header(3) == "Durée");
    CHECK(header(4) == "Texte");
}

TEST_CASE("an index outside the table holds nothing", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK_FALSE(model.data(model.index(9, 0), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data({}, Qt::DisplayRole).isValid());
}

TEST_CASE("a change of positions refreshes the columns that show them", "[gui][GUI-TABLE-01]") {
    // The whole point of a command reporting what it touched: three rows moved
    // out of four thousand should cost three refreshes, not a redraw.
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
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
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
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
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
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
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
    const QSignalSpy reset{&model, &SubtitleTableModel::modelReset};

    const std::vector<Change> report =
        reporting(ChangeKind::Removal,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)));
    model.applied(report);

    CHECK(reset.count() == 1);
}

TEST_CASE("a report with nothing in it refreshes nothing", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    model.applied(std::vector<Change>{});

    CHECK(refreshed.count() == 0);
}

TEST_CASE("a column outside the table holds nothing", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK_FALSE(model.data(model.index(0, 9), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(9, Qt::Horizontal, Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(-1, Qt::Horizontal, Qt::DisplayRole).isValid());
}

TEST_CASE("a cell before the first one holds nothing", "[gui][GUI-TABLE-01]") {
    // Qt builds indices, and a caller can build a wrong one. Answering empty
    // beats reading whatever lies before the vector.
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK_FALSE(model.data(model.createIndex(-1, 0), Qt::DisplayRole).isValid());
    CHECK_FALSE(model.data(model.createIndex(0, -1), Qt::DisplayRole).isValid());
}

TEST_CASE("only the horizontal header says anything", "[gui][GUI-TABLE-01]") {
    // The rows are numbered by a column of their own, so Qt's own row header is
    // hidden and has nothing to say.
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK_FALSE(model.headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
    CHECK_FALSE(model.headerData(0, Qt::Horizontal, Qt::ToolTipRole).isValid());
}

TEST_CASE("a cell holds nothing for a role the table does not serve", "[gui][GUI-TABLE-01]") {
    const Project project = threeSubtitles();
    const SubtitleTableModel model{project};

    CHECK_FALSE(model.data(model.index(0, 0), Qt::DecorationRole).isValid());
}

TEST_CASE("a change of translation refreshes nothing, for now", "[gui][GUI-TABLE-01]") {
    // No column shows it: the translation document exists in the model since
    // phase 1, and the interface builds it in phase 11. Reporting it is right;
    // refreshing a column that is not there would not be.
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
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
    const Project project = threeSubtitles();
    SubtitleTableModel model{project};
    const QSignalSpy refreshed{&model, &SubtitleTableModel::dataChanged};

    const std::vector<Change> report =
        reporting(ChangeKind::Reordering,
                  Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(2)));
    model.applied(report);

    REQUIRE(refreshed.count() == 1);
    CHECK(refreshed.at(0).at(0).toModelIndex().column() == 0);
    CHECK(refreshed.at(0).at(1).toModelIndex().column() == 4);
}
