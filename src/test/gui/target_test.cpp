// What an operation applies to — issue #132.
//
// **The selection, or the whole file.** That is what phase 4 sent here, and the
// bridge is written once for the four dialogs.

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

/// The indices of a selection, flattened, so that a test can name them.
[[nodiscard]] std::vector<std::size_t> valuesOf(const Selection& selection) {
    std::vector<std::size_t> values;
    for (const SubtitleIndex index : selection.indices())
        values.push_back(index.value());
    return values;
}

/// Ticks one row in a selection model.
void select(QItemSelectionModel& selection, const SubtitleTableModel& model, int row) {
    selection.select(model.index(row, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("nothing selected means the whole file", "[gui][GUI-SHIFT-01]") {
    // The default that makes the dialogs usable: nobody selects four thousand
    // rows to shift a whole file.
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
    // The same answer as the empty selection, by another road: nothing tells
    // « everything ticked » from « nothing ticked », and that is as it should
    // be.
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
