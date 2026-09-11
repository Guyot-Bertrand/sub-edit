// What the command adds to the rule: a project, a format, an undo, and one
// entry in the history — issue #365.
//
// The rule itself lives in `italics_test.cpp`, vocabulary by vocabulary. What
// is under test here is the composition: the format read from the document, the
// texts already right left alone, and the whole of it undone as one.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::setItalics;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::wouldItalicise;

[[nodiscard]] Subtitle saying(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project documentOf(SubtitleFormat format, std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(SourceFile{.format = format});
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("the command writes the tags of the format the document came from", "[edit]") {
    Project project = documentOf(SubtitleFormat::AdvancedSubStationAlpha,
                                 {saying("Bonjour.", 0), saying("Au revoir.", 2000)});

    const std::unique_ptr<Command> command =
        setItalics(project, Selection::all(project), Document::Main, true);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(textsOf(project) ==
          std::vector<std::string>{R"({\i1}Bonjour.{\i0})", R"({\i1}Au revoir.{\i0})"});
}

TEST_CASE("the command names which way it went, in the history", "[edit]") {
    const Project project =
        documentOf(SubtitleFormat::SubRip, {saying("Bonjour.", 0), saying("<i>Ici</i>", 2000)});

    const std::unique_ptr<Command> on =
        setItalics(project, Selection::all(project), Document::Main, true);
    REQUIRE(on != nullptr);
    CHECK(on->kind() == CommandKind::Italicise);

    const std::unique_ptr<Command> off =
        setItalics(project, Selection::all(project), Document::Main, false);
    REQUIRE(off != nullptr);
    CHECK(off->kind() == CommandKind::Unitalicise);
}

TEST_CASE("a selection already the way it was asked for yields no command", "[edit]") {
    const Project project = documentOf(SubtitleFormat::SubRip, {saying("<i>Bonjour.</i>", 0)});
    CHECK(setItalics(project, Selection::all(project), Document::Main, true) == nullptr);

    const Project plain = documentOf(SubtitleFormat::SubRip, {saying("Bonjour.", 0)});
    CHECK(setItalics(plain, Selection::all(plain), Document::Main, false) == nullptr);
}

TEST_CASE("a format that writes no style yields no command either", "[edit]") {
    const Project project = documentOf(SubtitleFormat::Lrc, {saying("Bonjour.", 0)});
    CHECK(setItalics(project, Selection::all(project), Document::Main, true) == nullptr);
}

TEST_CASE("only the selection is touched", "[edit]") {
    Project project = documentOf(
        SubtitleFormat::SubRip,
        {saying("Un", 0), saying("Deux", 2000), saying("Trois", 4000), saying("Quatre", 6000)});

    const std::unique_ptr<Command> command =
        setItalics(project,
                   Selection::range(SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2)),
                   Document::Main,
                   true);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(textsOf(project) ==
          std::vector<std::string>{"Un", "<i>Deux</i>", "<i>Trois</i>", "Quatre"});
}

TEST_CASE("reverting puts every text back exactly", "[edit]") {
    const Project before = documentOf(SubtitleFormat::MicroDvd,
                                      {saying("{C:$0000ff}Bonjour.", 0), saying("{Y:b}Ici", 2000)});
    Project project = before;

    const std::unique_ptr<Command> command =
        setItalics(project, Selection::all(project), Document::Main, true);
    REQUIRE(command != nullptr);
    command->apply(project);
    CHECK(textsOf(project) != std::vector<std::string>{"{C:$0000ff}Bonjour.", "{Y:b}Ici"});

    command->revert(project);
    CHECK(textsOf(project) == textsOf(before));
}

TEST_CASE("one subtitle out of italics sends the whole selection into them", "[edit]") {
    // Gaupol's rule, and the only one that makes a single button usable: a
    // mixed selection goes one way whole rather than inverting subtitle by
    // subtitle.
    const Project mixed = documentOf(SubtitleFormat::SubRip,
                                     {saying("<i>Bonjour.</i>", 0), saying("Au revoir.", 2000)});
    CHECK(wouldItalicise(mixed, Selection::all(mixed), Document::Main));

    const Project whole = documentOf(
        SubtitleFormat::SubRip, {saying("<i>Bonjour.</i>", 0), saying("<i>Au revoir.</i>", 2000)});
    CHECK_FALSE(wouldItalicise(whole, Selection::all(whole), Document::Main));
}

TEST_CASE("a blank row does not make a selection of italics ask for more", "[edit]") {
    const Project project =
        documentOf(SubtitleFormat::SubRip, {saying("<i>Bonjour.</i>", 0), saying("", 2000)});
    CHECK_FALSE(wouldItalicise(project, Selection::all(project), Document::Main));

    const std::unique_ptr<Command> command =
        setItalics(project, Selection::all(project), Document::Main, true);
    CHECK(command == nullptr);
}

TEST_CASE("the count comes from the command, and counts subtitles", "[edit]") {
    const Project project =
        documentOf(SubtitleFormat::SubRip,
                   {saying("Un", 0), saying("<i>Deux</i>", 2000), saying("Trois", 4000)});

    const std::unique_ptr<Command> command =
        setItalics(project, Selection::all(project), Document::Main, true);
    REQUIRE(command != nullptr);

    // Two, and not three: the one already in italics is not rewritten.
    CHECK(subedit::core::italicisedCount(*command) == 2);
}
