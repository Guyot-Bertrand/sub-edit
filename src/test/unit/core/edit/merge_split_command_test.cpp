// Merging and splitting subtitles — issue #380.
//
// Both are built out of a removal and an insertion, which already undo
// themselves exactly. What is under test here is what the two add: where the
// positions go, what becomes of the texts, and that the group counts as one.

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/merge_split_command.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::IndexRange;
using subedit::core::mergeSubtitles;
using subedit::core::OrderPolicy;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::splitSubtitle;
using subedit::core::SubRipExtras;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle saying(std::string_view text, std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = std::string{text}};
}

/// Four subtitles, one second each, two seconds apart.
[[nodiscard]] Project fourSubtitles() {
    Project project;
    project.setSubtitles({saying("Un.", 0, 1000),
                          saying("Deux.", 2000, 3000),
                          saying("Trois.", 4000, 5000),
                          saying("Quatre.", 6000, 7000)});
    return project;
}

[[nodiscard]] IndexRange run(std::size_t first, std::size_t last) {
    return IndexRange{.first = SubtitleIndex::fromValue(first),
                      .last = SubtitleIndex::fromValue(last)};
}

[[nodiscard]] SubtitleIndex at(std::size_t index) {
    return SubtitleIndex::fromValue(index);
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("merging keeps the start of the first and the end of the last", "[edit][merge]") {
    Project project = fourSubtitles();

    const std::unique_ptr<Command> command = mergeSubtitles(project, run(1, 3));
    REQUIRE(command != nullptr);
    command->apply(project);

    REQUIRE(project.count() == 2);
    const Subtitle& merged = project.subtitleAt(at(1));
    CHECK(merged.start == Timestamp::fromMilliseconds(2000));
    CHECK(merged.end == Timestamp::fromMilliseconds(7000));
    CHECK(merged.mainText == "Deux.\nTrois.\nQuatre.");
    CHECK(project.subtitleAt(at(0)).mainText == "Un.");
}

TEST_CASE("merging skips the empty texts rather than leaving blank lines", "[edit][merge]") {
    // `"\n".join(filter(None, texts))`: an empty text contributes nothing, not
    // even its separator.
    Project project;
    project.setSubtitles(
        {saying("", 0, 1000), saying("Deux.", 2000, 3000), saying("", 4000, 5000)});

    const std::unique_ptr<Command> command = mergeSubtitles(project, run(0, 2));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"Deux."});
}

TEST_CASE("merging glues the translations the same way", "[edit][merge]") {
    Project project = fourSubtitles();
    {
        std::vector<Subtitle> subtitles{project.subtitles().begin(), project.subtitles().end()};
        subtitles[0].translationText = "One.";
        subtitles[1].translationText = "Two.";
        project.setSubtitles(std::move(subtitles));
    }

    const std::unique_ptr<Command> command = mergeSubtitles(project, run(0, 2));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(at(0)).translationText == "One.\nTwo.");
}

TEST_CASE("merging keeps what the first subtitle carried beyond its text", "[edit][merge]") {
    // A style, a layer, coordinates: Gaupol loses them by building a new
    // subtitle. The first's are the ones that invent nothing.
    Project project = fourSubtitles();
    {
        std::vector<Subtitle> subtitles{project.subtitles().begin(), project.subtitles().end()};
        subtitles[1].extras = SubRipExtras{.coordinates = subedit::core::Rectangle{.x1 = 10}};
        project.setSubtitles(std::move(subtitles));
    }
    const Subtitle first = project.subtitleAt(at(1));

    const std::unique_ptr<Command> command = mergeSubtitles(project, run(1, 2));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(at(1)).extras == first.extras);
}

TEST_CASE("merging a single subtitle is no operation", "[edit][merge]") {
    const Project project = fourSubtitles();

    CHECK(mergeSubtitles(project, run(2, 2)) == nullptr);
}

TEST_CASE("undoing a merge gives the document back as it was", "[edit][merge]") {
    const Project before = fourSubtitles();
    Session session{before};

    static_cast<void>(session.apply(mergeSubtitles(session.project(), run(0, 3))));
    REQUIRE(session.project().count() == 1);

    static_cast<void>(session.undo());
    CHECK(std::vector<Subtitle>{session.project().subtitles().begin(),
                                session.project().subtitles().end()} ==
          std::vector<Subtitle>{before.subtitles().begin(), before.subtitles().end()});

    static_cast<void>(session.redo());
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Un.\nDeux.\nTrois.\nQuatre."});
}

TEST_CASE("merging several subtitles is one entry in the history", "[edit][merge]") {
    Session session{fourSubtitles()};

    static_cast<void>(session.apply(mergeSubtitles(session.project(), run(0, 3))));
    CHECK(session.nextUndoKind() == CommandKind::Merge);

    static_cast<void>(session.undo());
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("a merge is a change of structure", "[edit][merge]") {
    // The table rebuilds itself on a removal or an insertion; a merge that
    // reported only texts would leave it showing rows that are gone.
    const Project project = fourSubtitles();

    const std::unique_ptr<Command> command = mergeSubtitles(project, run(1, 2));
    REQUIRE(command != nullptr);

    const std::vector<subedit::core::Change> changes = command->describe();
    REQUIRE(changes.size() == 2);
    CHECK(changes[0].kind == ChangeKind::Removal);
    CHECK(changes[1].kind == ChangeKind::Insertion);
}

TEST_CASE("splitting cuts at the middle of the duration", "[edit][split]") {
    Project project = fourSubtitles();

    const std::unique_ptr<Command> command = splitSubtitle(project, at(1));
    REQUIRE(command != nullptr);
    command->apply(project);

    REQUIRE(project.count() == 5);
    const Subtitle& first = project.subtitleAt(at(1));
    const Subtitle& second = project.subtitleAt(at(2));
    CHECK(first.start == Timestamp::fromMilliseconds(2000));
    CHECK(first.end == Timestamp::fromMilliseconds(2500));
    CHECK(second.start == Timestamp::fromMilliseconds(2500));
    CHECK(second.end == Timestamp::fromMilliseconds(3000));
    CHECK(project.subtitleAt(at(3)).mainText == "Trois.");
}

TEST_CASE("splitting leaves the whole text to the first half", "[edit][split]") {
    // And not cut at the line break: right on two lines, invented on one or
    // three. The rule that can be predicted is this one.
    Project project;
    project.setSubtitles({saying("- Bonjour.\n- Salut.", 0, 2000)});

    const std::unique_ptr<Command> command = splitSubtitle(project, at(0));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"- Bonjour.\n- Salut.", ""});
}

TEST_CASE("an odd duration is split to the nearest millisecond", "[edit][split]") {
    // Through `Ratio`, which rounds halves away from zero: 1001 ms give 501
    // and 500, and the two halves still meet.
    Project project;
    project.setSubtitles({saying("Un.", 0, 1001)});

    const std::unique_ptr<Command> command = splitSubtitle(project, at(0));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(at(0)).end == Timestamp::fromMilliseconds(501));
    CHECK(project.subtitleAt(at(1)).start == Timestamp::fromMilliseconds(501));
    CHECK(project.subtitleAt(at(1)).end == Timestamp::fromMilliseconds(1001));
}

TEST_CASE("the first half keeps what the subtitle carried, the second is blank", "[edit][split]") {
    Project project;
    Subtitle styled = saying("Un.", 0, 1000);
    styled.translationText = "One.";
    styled.extras = SubRipExtras{.coordinates = subedit::core::Rectangle{.x1 = 10}};
    project.setSubtitles({styled});

    const std::unique_ptr<Command> command = splitSubtitle(project, at(0));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(at(0)).extras == styled.extras);
    CHECK(project.subtitleAt(at(0)).translationText == "One.");
    CHECK(project.subtitleAt(at(1)).translationText.empty());
    CHECK(project.subtitleAt(at(1)).extras == Subtitle{}.extras);
}

TEST_CASE("undoing a split gives the document back as it was", "[edit][split]") {
    const Project before = fourSubtitles();
    Session session{before};

    static_cast<void>(session.apply(splitSubtitle(session.project(), at(3))));
    REQUIRE(session.project().count() == 5);
    CHECK(session.nextUndoKind() == CommandKind::Split);

    static_cast<void>(session.undo());
    CHECK(std::vector<Subtitle>{session.project().subtitles().begin(),
                                session.project().subtitles().end()} ==
          std::vector<Subtitle>{before.subtitles().begin(), before.subtitles().end()});
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("a split may break the order, a merge may not", "[edit][split][merge]") {
    // A subtitle overlapping the next: its second half starts after it. A
    // merge starts where its first did, and nothing after it started earlier.
    CHECK(subedit::core::mayBreakOrder(CommandKind::Split));
    CHECK_FALSE(subedit::core::mayBreakOrder(CommandKind::Merge));

    Project project;
    project.setSubtitles({saying("Long.", 0, 10000), saying("Court.", 3000, 4000)});
    Session session{project, OrderPolicy::Strict};

    static_cast<void>(session.apply(splitSubtitle(session.project(), at(0))));

    CHECK(session.project().isInOrder());
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Long.", "Court.", ""});
}
