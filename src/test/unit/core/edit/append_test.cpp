// Appending a file to the end of a project — issue #434, decision D6.
//
// The rule is Gaupol's: nothing is inserted between the two files, so the
// appended one is shifted from the end of the last subtitle already there.
// What is under test here is the shift, the crossing into the project's own
// format (the rule and the words of pasting texts from another format,
// `GUI-CLIP-02`), and the single entry in the history.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::AppendedFile;
using subedit::core::appendFile;
using subedit::core::CommandKind;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

/// A project of `texts`, three seconds apart, in `format`.
[[nodiscard]] Project projectOf(std::initializer_list<std::string_view> texts,
                                SubtitleFormat format = SubtitleFormat::SubRip) {
    std::vector<Subtitle> subtitles;
    std::int64_t start = 0;
    for (const std::string_view text : texts) {
        subtitles.push_back(Subtitle{.start = Timestamp::fromMilliseconds(start),
                                     .end = Timestamp::fromMilliseconds(start + 2000),
                                     .mainText = std::string{text}});
        start += 3000;
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(SourceFile{.format = format});
    return project;
}

} // namespace

TEST_CASE("appending shifts the file from the end of the last subtitle", "[edit][append]") {
    const Project project = projectOf({"Un.", "Deux."});        // last one ends at 5000
    const Project appended = projectOf({"Bonjour.", "Salut."}); // own ends at 2000, 5000

    AppendedFile plan = appendFile(project, appended);
    REQUIRE(plan.command != nullptr);
    CHECK(plan.inserted == 2);
    CHECK(plan.command->kind() == CommandKind::Append);

    Project written = project;
    plan.command->apply(written);

    REQUIRE(written.count() == 4);
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(2)).start ==
          Timestamp::fromMilliseconds(5000));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(2)).end == Timestamp::fromMilliseconds(7000));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(2)).mainText == "Bonjour.");
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(3)).start ==
          Timestamp::fromMilliseconds(8000));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(3)).end ==
          Timestamp::fromMilliseconds(10000));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(3)).mainText == "Salut.");

    // Nothing before the append moved.
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(0)).start == Timestamp::fromMilliseconds(0));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(1)).start ==
          Timestamp::fromMilliseconds(3000));
}

TEST_CASE("appending to an empty project shifts by nothing", "[edit][append]") {
    const Project project;
    const Project appended = projectOf({"Bonjour."});

    AppendedFile plan = appendFile(project, appended);
    REQUIRE(plan.command != nullptr);

    Project written = project;
    plan.command->apply(written);

    REQUIRE(written.count() == 1);
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(0)).start == Timestamp::fromMilliseconds(0));
    CHECK(written.subtitleAt(SubtitleIndex::fromValue(0)).end == Timestamp::fromMilliseconds(2000));
}

TEST_CASE("an appended file of another format has its tags translated", "[edit][append]") {
    // ADR 0031 at the append's door: the braces of an Advanced SSA do not
    // land in a SubRip as they are.
    const Project project = projectOf({"Un."});
    const Project appended =
        projectOf({R"({\i1}Bonjour{\i0})"}, SubtitleFormat::AdvancedSubStationAlpha);

    AppendedFile plan = appendFile(project, appended);
    REQUIRE(plan.command != nullptr);
    CHECK(plan.loss.tags == 0);

    Project written = project;
    plan.command->apply(written);

    CHECK(written.subtitleAt(SubtitleIndex::fromValue(1)).mainText == "<i>Bonjour</i>");
}

TEST_CASE("what the project's format cannot hold is counted", "[edit][append]") {
    // TMPlayer carries no style at all: the italic goes, and the count says so.
    const Project project = projectOf({"Un."}, SubtitleFormat::TMPlayer);
    const Project appended = projectOf({"<i>Bonjour</i>"}, SubtitleFormat::SubRip);

    const AppendedFile plan = appendFile(project, appended);
    REQUIRE(plan.command != nullptr);
    // One pair of tags, one post: the count is of styles lost, not of brackets.
    CHECK(plan.loss.tags == 1);
}

TEST_CASE("an appended file with nothing to add gives no command", "[edit][append]") {
    const Project project = projectOf({"Un."});
    const Project appended;

    const AppendedFile plan = appendFile(project, appended);

    CHECK(plan.command == nullptr);
    CHECK(plan.inserted == 0);
}

TEST_CASE("an appended subtitle carries no translation", "[edit][append]") {
    Project appended = projectOf({"Bonjour."});
    std::vector<Subtitle> subtitles{appended.subtitles().begin(), appended.subtitles().end()};
    subtitles[0].translationText = "Hello.";
    appended.setSubtitles(std::move(subtitles));

    const Project project = projectOf({"Un."});
    AppendedFile plan = appendFile(project, appended);
    REQUIRE(plan.command != nullptr);

    Project written = project;
    plan.command->apply(written);

    CHECK(written.subtitleAt(SubtitleIndex::fromValue(1)).translationText.empty());
}

TEST_CASE("appending twice shifts the second from the newly appended end", "[edit][append]") {
    Session session{projectOf({"Un."})};          // ends at 2000
    const Project first = projectOf({"Deux."});   // own end at 2000
    const Project second = projectOf({"Trois."}); // own end at 2000

    AppendedFile plan1 = appendFile(session.project(), first);
    REQUIRE(plan1.command != nullptr);
    static_cast<void>(session.apply(std::move(plan1.command)));

    // The project now ends at 4000 (2000 + `first`'s own last end): the second
    // append must read that, not the project as it stood before.
    REQUIRE(session.project().subtitleAt(SubtitleIndex::fromValue(1)).end ==
            Timestamp::fromMilliseconds(4000));

    AppendedFile plan2 = appendFile(session.project(), second);
    REQUIRE(plan2.command != nullptr);
    static_cast<void>(session.apply(std::move(plan2.command)));

    REQUIRE(session.project().count() == 3);
    CHECK(session.project().subtitleAt(SubtitleIndex::fromValue(2)).start ==
          Timestamp::fromMilliseconds(4000));
}

TEST_CASE("undoing an append removes everything it added, in one entry", "[edit][append]") {
    Session session{projectOf({"Un."})};
    AppendedFile plan = appendFile(session.project(), projectOf({"Deux.", "Trois."}));
    REQUIRE(plan.command != nullptr);
    static_cast<void>(session.apply(std::move(plan.command)));
    REQUIRE(session.project().count() == 3);

    static_cast<void>(session.undo());

    CHECK(session.project().count() == 1);
    CHECK_FALSE(session.canUndo());
}
