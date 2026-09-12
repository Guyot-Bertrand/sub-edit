// The two commands of #379: what they add to the rules, and what they refuse.
//
// The rules themselves live in `core/text/`, case by case in five `.cas`
// corpora. What is under test here is the composition — the format read from
// the document, the texts already right left alone, and the whole of it undone
// as one.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::LetterCase;
using subedit::core::Project;
using subedit::core::recasedCount;
using subedit::core::Selection;
using subedit::core::setDialogueDashes;
using subedit::core::setLetterCase;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::wouldAddDialogueDashes;

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

TEST_CASE("the case reads the format from the document", "[edit]") {
    // An `.ass` writes its italic in braces, and the braces are not text: they
    // must come out of a text put in capitals exactly as they went in.
    Project project = documentOf(SubtitleFormat::AdvancedSubStationAlpha,
                                 {saying(R"({\i1}bonjour{\i0} marie)", 0)});

    const std::unique_ptr<Command> command =
        setLetterCase(project, Selection::all(project), Document::Main, LetterCase::Upper);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{R"({\i1}BONJOUR{\i0} MARIE)"});
    CHECK(command->kind() == CommandKind::ChangeCase);
}

TEST_CASE("a target already in the case asked for yields no command", "[edit]") {
    const Project project = documentOf(SubtitleFormat::SubRip, {saying("bonjour", 0)});

    CHECK(setLetterCase(project, Selection::all(project), Document::Main, LetterCase::Lower) ==
          nullptr);
}

TEST_CASE("the count is read from the command, and counts subtitles", "[edit]") {
    const Project project = documentOf(
        SubtitleFormat::SubRip, {saying("bonjour", 0), saying("MARIE", 2000), saying("", 4000)});

    const std::unique_ptr<Command> command =
        setLetterCase(project, Selection::all(project), Document::Main, LetterCase::Upper);
    REQUIRE(command != nullptr);

    // One, and not three: the one already shouting is not rewritten, and the
    // blank one has nothing to case.
    CHECK(recasedCount(*command) == 1);
}

TEST_CASE("the dashes name which way they went, in the history", "[edit]") {
    Project project =
        documentOf(SubtitleFormat::SubRip, {saying("Bonjour", 0), saying("- Marie", 2000)});

    const std::unique_ptr<Command> on =
        setDialogueDashes(project, Selection::all(project), Document::Main, true);
    REQUIRE(on != nullptr);
    CHECK(on->kind() == CommandKind::AddDialogueDashes);
    on->apply(project);
    CHECK(textsOf(project) == std::vector<std::string>{"- Bonjour", "- Marie"});

    const std::unique_ptr<Command> off =
        setDialogueDashes(project, Selection::all(project), Document::Main, false);
    REQUIRE(off != nullptr);
    CHECK(off->kind() == CommandKind::RemoveDialogueDashes);
}

TEST_CASE("one subtitle without a dash sends the whole selection into them", "[edit]") {
    const Project mixed =
        documentOf(SubtitleFormat::SubRip, {saying("- Bonjour", 0), saying("Marie", 2000)});
    CHECK(wouldAddDialogueDashes(mixed, Selection::all(mixed), Document::Main));

    const Project whole =
        documentOf(SubtitleFormat::SubRip, {saying("- Bonjour", 0), saying("- Marie", 2000)});
    CHECK_FALSE(wouldAddDialogueDashes(whole, Selection::all(whole), Document::Main));
}

TEST_CASE("a target whose dashes are already there yields no command", "[edit]") {
    const Project project =
        documentOf(SubtitleFormat::SubRip, {saying("- Bonjour", 0), saying("- Marie", 2000)});

    CHECK(setDialogueDashes(project, Selection::all(project), Document::Main, true) == nullptr);
}

TEST_CASE("reverting either puts every text back exactly", "[edit]") {
    const Project before = documentOf(
        SubtitleFormat::SubRip, {saying("<i>bonjour</i> marie", 0), saying("AU REVOIR", 2000)});
    Project project = before;

    const std::unique_ptr<Command> command =
        setLetterCase(project, Selection::all(project), Document::Main, LetterCase::Title);
    REQUIRE(command != nullptr);
    command->apply(project);
    CHECK(textsOf(project) != textsOf(before));

    command->revert(project);
    CHECK(textsOf(project) == textsOf(before));
}
