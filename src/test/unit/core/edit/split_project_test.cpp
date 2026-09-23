// Splitting a project in two — issue #439, decision D6.
//
// The exact inverse of appending: the tail is copied into a project of its
// own, shifted back by the end of the last subtitle that stays, so that
// splitting and then appending gives the project back. What is under test is
// that round trip, what the new project inherits, the refusal of a cut no file
// could write, and the single entry in the history.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::appendFile;
using subedit::core::CommandKind;
using subedit::core::Encoding;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::splitProject;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

/// A project of `texts`, three seconds apart, each lasting two.
[[nodiscard]] Project projectOf(std::initializer_list<std::string_view> texts) {
    std::vector<Subtitle> subtitles;
    std::int64_t start = 0;
    for (const std::string_view text : texts) {
        subtitles.push_back(Subtitle{.start = Timestamp::fromMilliseconds(start),
                                     .end = Timestamp::fromMilliseconds(start + 2000),
                                     .mainText = std::string{text},
                                     .translationText = "T:" + std::string{text}});
        start += 3000;
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(
        SourceFile{.path = std::filesystem::path{"film.srt"}, .format = SubtitleFormat::SubRip});
    return project;
}

[[nodiscard]] SubtitleIndex at(std::size_t index) {
    return SubtitleIndex::fromValue(index);
}

} // namespace

TEST_CASE("splitting copies the tail, shifted back by the end of the last subtitle that stays",
          "[edit][split-project]") {
    const Project project = projectOf({"Un.", "Deux.", "Trois.", "Quatre."}); // 2nd ends at 5000

    auto split = splitProject(project, at(2));

    REQUIRE(split.has_value());
    REQUIRE(split->tail.count() == 2);
    CHECK(split->tail.subtitleAt(at(0)).mainText == "Trois.");
    CHECK(split->tail.subtitleAt(at(0)).start == Timestamp::fromMilliseconds(1000));
    CHECK(split->tail.subtitleAt(at(0)).end == Timestamp::fromMilliseconds(3000));
    CHECK(split->tail.subtitleAt(at(1)).start == Timestamp::fromMilliseconds(4000));
    CHECK(split->tail.subtitleAt(at(1)).end == Timestamp::fromMilliseconds(6000));
}

TEST_CASE("the command takes the tail out of the project, in one entry, and undoes it",
          "[edit][split-project]") {
    const Project project = projectOf({"Un.", "Deux.", "Trois.", "Quatre."});
    auto split = splitProject(project, at(2));
    REQUIRE(split.has_value());
    REQUIRE(split->command != nullptr);
    CHECK(split->command->kind() == CommandKind::SplitProject);

    Session session{project};
    session.apply(std::move(split->command));

    CHECK(session.project().count() == 2);
    CHECK(session.undoableCount() == 1);

    session.undo();
    REQUIRE(session.project().count() == 4);
    CHECK(session.project().subtitleAt(at(3)).mainText == "Quatre.");
    CHECK(session.project().subtitleAt(at(3)).start == Timestamp::fromMilliseconds(9000));
}

TEST_CASE("splitting and then appending gives the project back, both texts included",
          "[edit][split-project]") {
    const Project project = projectOf({"Un.", "Deux.", "Trois.", "Quatre."});
    auto split = splitProject(project, at(2));
    REQUIRE(split.has_value());

    Project origin = project;
    split->command->apply(origin);

    // An appended file carries no translation (D6); the round trip is on the
    // main text and on the positions, which is what the appending gives back.
    auto appended = appendFile(origin, split->tail);
    REQUIRE(appended.command != nullptr);
    appended.command->apply(origin);

    REQUIRE(origin.count() == project.count());
    for (std::size_t index = 0; index < project.count(); ++index) {
        CHECK(origin.subtitleAt(at(index)).mainText == project.subtitleAt(at(index)).mainText);
        CHECK(origin.subtitleAt(at(index)).start == project.subtitleAt(at(index)).start);
        CHECK(origin.subtitleAt(at(index)).end == project.subtitleAt(at(index)).end);
    }
}

TEST_CASE("the new project keeps both texts, the format and the encoding, and has no path",
          "[edit][split-project]") {
    Project project = projectOf({"Un.", "Deux.", "Trois."});
    SourceFile main = project.sourceFile();
    main.format = SubtitleFormat::WebVtt;
    main.encoding = Encoding::utf8(subedit::core::ByteOrderMark::Present);
    project.setSourceFile(main);
    project.setSourceFile(
        subedit::core::Document::Translation,
        SourceFile{.path = std::filesystem::path{"film.en.srt"}, .format = SubtitleFormat::SubRip});

    auto split = splitProject(project, at(1));

    REQUIRE(split.has_value());
    CHECK(split->tail.subtitleAt(at(0)).translationText == "T:Deux.");
    CHECK(split->tail.sourceFile().format == SubtitleFormat::WebVtt);
    CHECK(split->tail.sourceFile().encoding == main.encoding);
    CHECK_FALSE(split->tail.sourceFile().path.has_value());
    CHECK(split->tail.translationFile().has_value());
    const SourceFile& translation = split->tail.sourceFile(subedit::core::Document::Translation);
    CHECK_FALSE(translation.path.has_value());
    CHECK(translation.format == SubtitleFormat::SubRip);
    CHECK(split->tail.frameRate() == project.frameRate());
}

TEST_CASE("a cut that would put a subtitle before the origin is refused, naming it",
          "[edit][split-project]") {
    // The second subtitle ends at 5000; the third starts at 4000 — the two
    // halves overlap, and shifting the tail back by 5000 lands it at -1000.
    Project project = projectOf({"Un.", "Deux.", "Trois."});
    project.subtitleAt(at(2)).start = Timestamp::fromMilliseconds(4000);

    auto split = splitProject(project, at(2));

    REQUIRE_FALSE(split.has_value());
    CHECK(split.error().before == at(2));
}

TEST_CASE("landing exactly on the origin is allowed", "[edit][split-project]") {
    Project project = projectOf({"Un.", "Deux.", "Trois."});
    project.subtitleAt(at(2)).start = Timestamp::fromMilliseconds(5000);

    auto split = splitProject(project, at(2));

    REQUIRE(split.has_value());
    CHECK(split->tail.subtitleAt(at(0)).start == Timestamp::fromMilliseconds(0));
}

TEST_CASE("a project born from another can be told it was never written", "[edit][split-project]") {
    auto split = splitProject(projectOf({"Un.", "Deux."}), at(1));
    REQUIRE(split.has_value());
    Session session{std::move(split->tail)};
    REQUIRE_FALSE(session.hasUnsavedChanges(subedit::core::Document::Main));

    session.markUnsaved(subedit::core::Document::Main);

    CHECK(session.hasUnsavedChanges(subedit::core::Document::Main));
    session.markSaved(subedit::core::Document::Main);
    CHECK_FALSE(session.hasUnsavedChanges(subedit::core::Document::Main));
}

TEST_CASE("only a cut that leaves something on each side is a cut", "[edit][split-project]") {
    const Project project = projectOf({"Un.", "Deux."});

    CHECK_THROWS_AS((void)splitProject(project, at(0)), std::out_of_range);
    CHECK_THROWS_AS((void)splitProject(project, at(2)), std::out_of_range);
    CHECK(splitProject(project, at(1)).has_value());
}
