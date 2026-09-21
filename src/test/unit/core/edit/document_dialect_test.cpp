// An operation on a document reads the tags of that document's format — issue
// #429, ADR 0032.
//
// Every case here has **two formats in one project**: the main document in
// SubRip, the translation in Advanced SSA, whose italic is `{\i1}` and not
// `<i>`. An operation that reads the main file's format on the translation gets
// the braces wrong, and each result below is one that only the right dialect
// produces.
//
// The reading-speed case is the other way round: an operation that stays on the
// main text must go on reading the main file's format when the translation
// holds another. The search, which #433 gave a document, is read both ways.

#include <subedit/core/command/command.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace {

using subedit::core::adjustDurations;
using subedit::core::ClipboardTexts;
using subedit::core::Command;
using subedit::core::copyTexts;
using subedit::core::Document;
using subedit::core::DurationAdjustment;
using subedit::core::DurationConstraints;
using subedit::core::findNext;
using subedit::core::findPrevious;
using subedit::core::HearingImpairedTally;
using subedit::core::LetterCase;
using subedit::core::PastedTexts;
using subedit::core::pasteTexts;
using subedit::core::PatternError;
using subedit::core::Project;
using subedit::core::ReadingSpeed;
using subedit::core::removeHearingImpaired;
using subedit::core::replaceAll;
using subedit::core::ReplacedAll;
using subedit::core::ReplacedMatch;
using subedit::core::replaceMatch;
using subedit::core::SearchOptions;
using subedit::core::SearchPattern;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::setDialogueDashes;
using subedit::core::setItalics;
using subedit::core::setLetterCase;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::tallyOf;
using subedit::core::TextMatch;
using subedit::core::Timestamp;
using subedit::core::wouldAddDialogueDashes;
using subedit::core::wouldItalicise;

constexpr SubtitleFormat kMain = SubtitleFormat::SubRip;
constexpr SubtitleFormat kTranslation = SubtitleFormat::AdvancedSubStationAlpha;

/// One subtitle: `mainText` in SubRip, `translationText` in Advanced SSA.
[[nodiscard]] Project translated(std::string_view mainText, std::string_view translationText) {
    Project project;
    project.setSubtitles({Subtitle{.start = Timestamp::fromMilliseconds(0),
                                   .end = Timestamp::fromMilliseconds(1500),
                                   .mainText = std::string{mainText},
                                   .translationText = std::string{translationText}}});
    project.setSourceFile(SourceFile{.format = kMain});
    project.setSourceFile(Document::Translation, SourceFile{.format = kTranslation});
    return project;
}

/// `text` as a plain-text pattern, or nothing when it does not compile.
[[nodiscard]] std::optional<SearchPattern> patternOf(std::string_view text) {
    std::expected<SearchPattern, PatternError> compiled =
        SearchPattern::compile(text, SearchOptions{.regex = false, .ignoreCase = false});
    if (!compiled.has_value())
        return std::nullopt;
    return std::move(*compiled);
}

/// A match in the only subtitle, over `start` to `end` of its visible text.
[[nodiscard]] TextMatch matchAt(std::size_t start, std::size_t end) {
    return TextMatch{.index = SubtitleIndex::fromValue(0), .start = start, .end = end};
}

[[nodiscard]] const Subtitle& only(const Project& project) {
    return project.subtitleAt(SubtitleIndex::fromValue(0));
}

} // namespace

TEST_CASE("the italic of a translation is written in the tags of its own format",
          "[edit][document]") {
    Project project = translated("Bonjour.", "Salut.");

    const std::unique_ptr<Command> command =
        setItalics(project, Selection::all(project), Document::Translation, true);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(only(project).translationText == R"({\i1}Salut.{\i0})");
    CHECK(only(project).mainText == "Bonjour.");
}

TEST_CASE("a translation already in italics in its own format is seen as such",
          "[edit][document]") {
    // The other reading of the same format: what would the gesture do — put
    // into italics, or take out. Read in SubRip, the braces are text, and the
    // answer would be « put in » on a translation that already is.
    const Project project = translated("Bonjour.", R"({\i1}Salut.{\i0})");

    CHECK_FALSE(wouldItalicise(project, Selection::all(project), Document::Translation));
    CHECK(wouldItalicise(project, Selection::all(project), Document::Main));
}

TEST_CASE("the dashes of a translation go after the tags of its own format", "[edit][document]") {
    Project project = translated("Bonjour.", R"({\i1}Salut.{\i0})");

    const std::unique_ptr<Command> command =
        setDialogueDashes(project, Selection::all(project), Document::Translation, true);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(only(project).translationText == R"({\i1}- Salut.{\i0})");
}

TEST_CASE("a translation already dashed in its own format is seen as such", "[edit][document]") {
    const Project project = translated("Bonjour.", R"({\i1}- Salut.{\i0})");

    CHECK_FALSE(wouldAddDialogueDashes(project, Selection::all(project), Document::Translation));
}

TEST_CASE("the case of a translation leaves the tags of its own format alone", "[edit][document]") {
    // Read in SubRip, `{\i1}` is text: the `i` would come out a capital and the
    // tag would stop being one.
    Project project = translated("bonjour", R"({\i1}salut{\i0} marie)");

    const std::unique_ptr<Command> command =
        setLetterCase(project, Selection::all(project), Document::Translation, LetterCase::Upper);
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(only(project).translationText == R"({\i1}SALUT{\i0} MARIE)");
}

TEST_CASE("what shows of a translation is judged in the tags of its own format",
          "[edit][document]") {
    // A translation left holding `{\i1}{\i0}` shows nothing in Advanced SSA, and
    // the rule empties it; in SubRip the braces would count as text, and it
    // would be rewritten instead, so the tally would name no emptying at all.
    //
    // Only the dialect is read here. What emptying a translation does to its
    // subtitle — nothing, the subtitle stays — is settled in
    // `hearing_impaired_removal_test.cpp`.
    const Project project = translated("Bonjour.", R"({\i1}[soupir]{\i0})");

    const std::unique_ptr<Command> command =
        removeHearingImpaired(project, Selection::all(project), Document::Translation);
    REQUIRE(command != nullptr);

    // Emptied, and counted as cleaned: one text rewritten, no subtitle taken.
    CHECK(tallyOf(*command) == HearingImpairedTally{.cleaned = 1, .removed = 0});
}

TEST_CASE("a copy says the format of the document it was taken from", "[edit][document]") {
    const Project project = translated("Bonjour.", "Salut.");

    CHECK(copyTexts(project, Selection::all(project), Document::Main).format == kMain);
    CHECK(copyTexts(project, Selection::all(project), Document::Translation).format ==
          kTranslation);
}

TEST_CASE("a paste into a translation is written in the tags of its own format",
          "[edit][document]") {
    Project project = translated("Bonjour.", "Salut.");
    const ClipboardTexts clipboard{.texts = {"<i>Coucou</i>"}, .format = SubtitleFormat::SubRip};

    PastedTexts pasted =
        pasteTexts(project, clipboard, SubtitleIndex::fromValue(0), Document::Translation);
    REQUIRE(pasted.command != nullptr);
    pasted.command->apply(project);

    CHECK(only(project).translationText == R"({\i1}Coucou{\i0})");
    CHECK(only(project).mainText == "Bonjour.");
}

TEST_CASE("the reading speed of an adjustment is measured in the tags of the main file",
          "[edit][document]") {
    // Ten visible characters at ten a second need one second, and the subtitle
    // lasts a second and a half: nothing to lengthen. Read in the translation's
    // format the `<i>` would be seventeen characters, and it would.
    const Project project = translated("<i>abcdefghij</i>", "Salut.");
    const DurationConstraints constraints{.speed = ReadingSpeed::create(10.0, true, false),
                                          .minimum = std::nullopt,
                                          .maximum = std::nullopt,
                                          .gap = std::nullopt};

    const DurationAdjustment adjustment =
        adjustDurations(project, Selection::all(project), constraints);

    CHECK(adjustment.adjusted == 0);
}

TEST_CASE("the search in the main text reads the tags of the main file", "[edit][document]") {
    // `Bonjour` straddles a SubRip tag: found in the main file's format, and
    // lost if the translation's were read.
    const Project project = translated("<i>Bon</i>jour", "Salut.");
    const std::optional<SearchPattern> pattern = patternOf("Bonjour");
    REQUIRE(pattern.has_value());

    CHECK(findNext(project, Selection::all(project), Document::Main, *pattern, std::nullopt) ==
          matchAt(0, 7));
}

TEST_CASE("the search in a translation reads the tags of its own format", "[edit][document]") {
    // The same word cut by a tag, this time in the translation's dialect: the
    // braces are not text there, and they would be in SubRip.
    const Project project = translated("Salut.", "{\\i1}Bon{\\i0}jour");
    const std::optional<SearchPattern> pattern = patternOf("Bonjour");
    REQUIRE(pattern.has_value());
    const Selection all = Selection::all(project);

    CHECK(findNext(project, all, Document::Translation, *pattern, std::nullopt) == matchAt(0, 7));
    CHECK(findPrevious(project, all, Document::Translation, *pattern, std::nullopt) ==
          matchAt(0, 7));
}

TEST_CASE("the search looks in the text of the document it is given, and in no other",
          "[edit][document]") {
    const Project project = translated("Marie.", "Sophie.");
    const Selection all = Selection::all(project);
    const std::optional<SearchPattern> marie = patternOf("Marie");
    const std::optional<SearchPattern> sophie = patternOf("Sophie");
    REQUIRE(marie.has_value());
    REQUIRE(sophie.has_value());

    CHECK(findNext(project, all, Document::Main, *marie, std::nullopt).has_value());
    CHECK_FALSE(findNext(project, all, Document::Main, *sophie, std::nullopt).has_value());
    CHECK(findNext(project, all, Document::Translation, *sophie, std::nullopt).has_value());
    CHECK_FALSE(findNext(project, all, Document::Translation, *marie, std::nullopt).has_value());
    CHECK_FALSE(
        findPrevious(project, all, Document::Translation, *marie, std::nullopt).has_value());
}

TEST_CASE("replacing a match in a translation writes that text and leaves the main one",
          "[edit][document]") {
    // The second « Marie » lies at 9..14 of what is read, which is the case
    // only when the braces are read as tags: in SubRip they would be text and
    // the span would fall elsewhere.
    Session session{translated("Marie et Marie.", "Marie {\\i1}et{\\i0} Marie.")};
    const std::optional<SearchPattern> pattern = patternOf("Marie");
    REQUIRE(pattern.has_value());

    std::optional<ReplacedMatch> replaced =
        replaceMatch(session.project(), Document::Translation, *pattern, matchAt(9, 14), "Sophie");
    REQUIRE(replaced.has_value());
    if (!replaced.has_value())
        return;
    ReplacedMatch result = std::move(*replaced);
    REQUIRE(result.command != nullptr);
    CHECK(result.written == matchAt(9, 15));
    static_cast<void>(session.apply(std::move(result.command)));

    CHECK(only(session.project()).translationText == "Marie {\\i1}et{\\i0} Sophie.");
    CHECK(only(session.project()).mainText == "Marie et Marie.");

    static_cast<void>(session.undo());
    CHECK(only(session.project()).translationText == "Marie {\\i1}et{\\i0} Marie.");
}

TEST_CASE("replacing a match in the main text leaves the translation", "[edit][document]") {
    Session session{translated("Marie.", "Marie.")};
    const std::optional<SearchPattern> pattern = patternOf("Marie");
    REQUIRE(pattern.has_value());

    std::optional<ReplacedMatch> replaced =
        replaceMatch(session.project(), Document::Main, *pattern, matchAt(0, 5), "Sophie");
    REQUIRE(replaced.has_value());
    if (!replaced.has_value())
        return;
    ReplacedMatch result = std::move(*replaced);
    REQUIRE(result.command != nullptr);
    static_cast<void>(session.apply(std::move(result.command)));

    CHECK(only(session.project()).mainText == "Sophie.");
    CHECK(only(session.project()).translationText == "Marie.");
}

TEST_CASE("replacing all in a translation is one entry, and the main text is not touched",
          "[edit][document]") {
    Session session{translated("Marie et Marie.", "{\\i1}Marie{\\i0} et Marie.")};
    const std::optional<SearchPattern> pattern = patternOf("Marie");
    REQUIRE(pattern.has_value());

    ReplacedAll replaced = replaceAll(session.project(),
                                      Selection::all(session.project()),
                                      Document::Translation,
                                      *pattern,
                                      "Sophie");
    REQUIRE(replaced.command != nullptr);
    CHECK(replaced.count == 2);
    static_cast<void>(session.apply(std::move(replaced.command)));

    CHECK(only(session.project()).translationText == "{\\i1}Sophie{\\i0} et Sophie.");
    CHECK(only(session.project()).mainText == "Marie et Marie.");

    static_cast<void>(session.undo());
    CHECK(only(session.project()).translationText == "{\\i1}Marie{\\i0} et Marie.");
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("replacing all in the main text leaves the translation", "[edit][document]") {
    Session session{translated("<i>Marie</i> et Marie.", "Marie et Marie.")};
    const std::optional<SearchPattern> pattern = patternOf("Marie");
    REQUIRE(pattern.has_value());

    ReplacedAll replaced = replaceAll(
        session.project(), Selection::all(session.project()), Document::Main, *pattern, "Sophie");
    REQUIRE(replaced.command != nullptr);
    CHECK(replaced.count == 2);
    static_cast<void>(session.apply(std::move(replaced.command)));

    CHECK(only(session.project()).mainText == "<i>Sophie</i> et Sophie.");
    CHECK(only(session.project()).translationText == "Marie et Marie.");
}
