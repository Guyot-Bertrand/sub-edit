// What the table shows of a damaged document — issue #134.
//
// **Marked, and named.** An anomaly is not a reading error: it is what a
// document *is*, and it survives editing. The table carries it, then, rather
// than a report read once on opening.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QBrush>
#include <QColor>
#include <QString>
#include <QVariant>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace {

using subedit::core::Duration;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::ShiftCommand;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::gui::SubtitleTableModel;

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = "x"};
}

/// Two sound subtitles, one after the other.
[[nodiscard]] Project sound() {
    Project project;
    project.setSubtitles({from(1000, 2000), from(3000, 4000)});
    return project;
}

[[nodiscard]] bool tinted(const SubtitleTableModel& model, int row, int column) {
    return model.data(model.index(row, column), Qt::BackgroundRole).isValid();
}

[[nodiscard]] std::string tipAt(const SubtitleTableModel& model, int row, int column) {
    return model.data(model.index(row, column), Qt::ToolTipRole).toString().toStdString();
}

[[nodiscard]] QColor tintAt(const SubtitleTableModel& model, int row) {
    return model.data(model.index(row, 1), Qt::BackgroundRole).value<QBrush>().color();
}

} // namespace

TEST_CASE("a sound document is not marked anywhere", "[gui][GUI-TABLE-02]") {
    Session session{sound()};
    const SubtitleTableModel model{session};

    CHECK_FALSE(tinted(model, 0, 1));
    CHECK_FALSE(tinted(model, 1, 1));
    CHECK(tipAt(model, 0, 1).empty());
}

TEST_CASE("a subtitle that ends before it starts is marked and named", "[gui][GUI-TABLE-02]") {
    Project project;
    project.setSubtitles({from(3000, 1000)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    CHECK(tinted(model, 0, 1));
    CHECK(tinted(model, 0, 2));
    CHECK(tipAt(model, 0, 1) == "ends before it starts");
}

TEST_CASE("an overlap marks the subtitle that is out of place, not the one before",
          "[gui][GUI-TABLE-02]") {
    // It is the line one moves, not the one it runs into.
    Project project;
    project.setSubtitles({from(1000, 5000), from(3000, 6000)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    CHECK_FALSE(tinted(model, 0, 1));
    CHECK(tinted(model, 1, 1));
    CHECK(tipAt(model, 1, 1) == "starts before the previous one ends");
}

TEST_CASE("disorder is marked as such, and leads what it is named by", "[gui][GUI-TABLE-02]") {
    // **A subtitle out of order almost always overlaps its predecessor**, and
    // that is arithmetic: starting before it starts means starting before it
    // ends, unless it is itself broken. So the disorder governs — putting the
    // line back in place takes the overlap with it.
    Project project;
    project.setSubtitles({from(4000, 5000), from(1000, 2000)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    CHECK(tipAt(model, 1, 1).starts_with("starts before the previous one starts"));
}

TEST_CASE("the three kinds are told apart by their tint", "[gui][GUI-TABLE-02]") {
    // They are repaired differently, so they are told apart.
    Project broken;
    broken.setSubtitles({from(3000, 1000)});
    Session brokenSession{std::move(broken)};
    const SubtitleTableModel brokenModel{brokenSession};

    Project overlapping;
    overlapping.setSubtitles({from(1000, 5000), from(3000, 6000)});
    Session overlappingSession{std::move(overlapping)};
    const SubtitleTableModel overlappingModel{overlappingSession};

    Project disordered;
    disordered.setSubtitles({from(4000, 5000), from(1000, 2000)});
    Session disorderedSession{std::move(disordered)};
    const SubtitleTableModel disorderedModel{disorderedSession};

    CHECK_FALSE(tintAt(brokenModel, 0) == tintAt(overlappingModel, 1));
    CHECK_FALSE(tintAt(overlappingModel, 1) == tintAt(disorderedModel, 1));
}

TEST_CASE("only the position columns are tinted", "[gui][GUI-TABLE-02]") {
    // An anomaly is a matter of positions; tinting the text would suggest it
    // has something to do with it.
    Project project;
    project.setSubtitles({from(3000, 1000)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    CHECK_FALSE(tinted(model, 0, 0));
    CHECK(tinted(model, 0, 1));
    CHECK(tinted(model, 0, 2));
    CHECK(tinted(model, 0, 3));
    CHECK_FALSE(tinted(model, 0, 4));
}

TEST_CASE("a subtitle carrying two anomalies names both", "[gui][GUI-TABLE-02]") {
    // Each is repaired differently, so each is said.
    Project project;
    project.setSubtitles({from(4000, 5000), from(1000, 2000)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    const std::string tip = tipAt(model, 1, 1);
    CHECK(tip.find("starts before the previous one ends") != std::string::npos);
    CHECK(tip.find("starts before the previous one starts") != std::string::npos);
}

TEST_CASE("the marking follows a change of positions", "[gui][GUI-TABLE-02]") {
    // Recomputed, never remembered: an anomaly is what the document is now,
    // not what it was on opening.
    Session session{sound()};
    SubtitleTableModel model{session};
    REQUIRE_FALSE(tinted(model, 1, 1));

    const SubtitleIndex second = SubtitleIndex::fromValue(1);
    model.applied(session.apply(std::make_unique<ShiftCommand>(Selection::range(second, second),
                                                               Duration::fromMilliseconds(-2500))));

    CHECK(tinted(model, 1, 1));
}

TEST_CASE("the marking follows an undo", "[gui][GUI-TABLE-02]") {
    Session session{sound()};
    SubtitleTableModel model{session};
    const SubtitleIndex second = SubtitleIndex::fromValue(1);
    model.applied(session.apply(std::make_unique<ShiftCommand>(Selection::range(second, second),
                                                               Duration::fromMilliseconds(-2500))));
    REQUIRE(tinted(model, 1, 1));

    model.applied(session.undo());

    CHECK_FALSE(tinted(model, 1, 1));
}

TEST_CASE("the marking follows a cell edited by hand", "[gui][GUI-TABLE-02]") {
    Session session{sound()};
    SubtitleTableModel model{session};

    REQUIRE(model.setData(model.index(1, 1), QStringLiteral("00:00:00,500"), Qt::EditRole));

    CHECK(tinted(model, 1, 1));
}

TEST_CASE("editing a text changes no marking", "[gui][GUI-TABLE-02]") {
    // A text belongs to no anomaly: recomputing would have no effect, and not
    // recomputing would show here.
    Project project;
    project.setSubtitles({from(3000, 1000)});
    Session session{std::move(project)};
    SubtitleTableModel model{session};
    REQUIRE(tinted(model, 0, 1));

    REQUIRE(model.setData(model.index(0, 4), QStringLiteral("autre"), Qt::EditRole));

    CHECK(tinted(model, 0, 1));
}

TEST_CASE("a subtitle carrying all three is named by the one to repair first",
          "[gui][GUI-TABLE-02]") {
    // The worst case, and the one that puts the precedence to the test: this
    // subtitle ends before it starts, starts before the previous one, and
    // overlaps it. Tinting it as an overlap would send the user off adjusting a
    // timing while the line is broken in itself.
    Project project;
    project.setSubtitles({from(4000, 5000), from(1000, 500)});
    Session session{std::move(project)};
    const SubtitleTableModel model{session};

    const std::string tip = tipAt(model, 1, 1);
    CHECK(tip.starts_with("ends before it starts"));
    CHECK(tip.find("starts before the previous one starts") != std::string::npos);
    CHECK(tip.find("starts before the previous one ends") != std::string::npos);

    // And the tint is the first one's, not either of the other two.
    Project alone;
    alone.setSubtitles({from(3000, 1000)});
    Session aloneSession{std::move(alone)};
    const SubtitleTableModel aloneModel{aloneSession};
    CHECK(tintAt(model, 1) == tintAt(aloneModel, 0));
}
