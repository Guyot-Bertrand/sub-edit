// The correction assistant's calculation, at the core — issue #504, decision
// D8 of the spec of phase 12.
//
// The corrections themselves are not retested here — `common-error.cas`,
// `capitalization.cas`, `hearing-impaired.cas` and `line-break.cas` already
// cover them, case by case, through the functions this file composes. What
// is under test is the composition: proposing changes without touching a
// project, applying an accepted subset as one command per project, the class
// filter of decision D4, and the order Gaupol runs its tasks in.

#include <subedit/core/command/command.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::AppliedCorrection;
using subedit::core::applyCorrections;
using subedit::core::CharacterLineMeasure;
using subedit::core::CorrectionSettings;
using subedit::core::CorrectionTally;
using subedit::core::CorrectionTarget;
using subedit::core::Document;
using subedit::core::IcuPatternEngine;
using subedit::core::PatternActivation;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::Project;
using subedit::core::proposeCorrections;
using subedit::core::ProposedCorrection;
using subedit::core::readPatternCatalogue;
using subedit::core::RealFileSystem;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::tallyOf;
using subedit::core::Timestamp;

const PatternCatalogue& shippedPatterns() {
    static const PatternCatalogue catalogue = [] {
        const RealFileSystem files;
        return readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});
    }();
    return catalogue;
}

[[nodiscard]] Subtitle saying(std::string text, std::int64_t startMs) {
    return Subtitle{.start = Timestamp::fromMilliseconds(startMs),
                    .end = Timestamp::fromMilliseconds(startMs + 900),
                    .mainText = std::move(text)};
}

[[nodiscard]] Project projectOf(std::vector<std::string> texts) {
    Project project;
    std::vector<Subtitle> subtitles;
    subtitles.reserve(texts.size());
    std::int64_t start = 0;
    for (std::string& text : texts) {
        subtitles.push_back(saying(std::move(text), start));
        start += 1000;
    }
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

[[nodiscard]] bool sameSubtitles(const Project& a, const Project& b) {
    if (a.count() != b.count())
        return false;
    for (std::size_t i = 0; i < a.count(); ++i) {
        if (!(a.subtitles()[i] == b.subtitles()[i]))
            return false;
    }
    return true;
}

[[nodiscard]] CorrectionTarget wholeProject(Project& project, Document document = Document::Main) {
    return CorrectionTarget{
        .project = &project, .selection = Selection::all(project), .document = document};
}

} // namespace

TEST_CASE("proposing corrections touches no project", "[text][assistant]") {
    Project first = projectOf({"Bonjour  Marie", "Ça va bien"});
    Project second = projectOf({"Au  revoir", "A bientot"});
    const Project firstBefore = first;
    const Project secondBefore = second;

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};

    const std::vector<CorrectionTarget> targets{wholeProject(first), wholeProject(second)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    CHECK(sameSubtitles(first, firstBefore));
    CHECK(sameSubtitles(second, secondBefore));

    // Only the double space is a real change; each project contributes one.
    REQUIRE(proposed.size() == 2);
    CHECK(proposed[0].project == &first);
    CHECK(proposed[0].original == "Bonjour  Marie");
    CHECK(proposed[0].proposed == std::optional<std::string>{"Bonjour Marie"});
    CHECK(proposed[1].project == &second);
    CHECK(proposed[1].original == "Au  revoir");
    CHECK(proposed[1].proposed == std::optional<std::string>{"Au revoir"});
}

TEST_CASE("applying accepted corrections is one command per project, undone as one",
          "[text][assistant]") {
    Project first = projectOf({"Bonjour  Marie"});
    Project second = projectOf({"Au  revoir"});
    const Project firstBefore = first;
    const Project secondBefore = second;

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};

    const std::vector<CorrectionTarget> targets{wholeProject(first), wholeProject(second)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);
    REQUIRE(proposed.size() == 2);

    const std::vector<AppliedCorrection> applied =
        applyCorrections(proposed, /*removeBlankSubtitles=*/true);
    REQUIRE(applied.size() == 2);

    CHECK(textsOf(first) == std::vector<std::string>{"Bonjour Marie"});
    CHECK(textsOf(second) == std::vector<std::string>{"Au revoir"});

    const CorrectionTally tally = tallyOf(applied);
    CHECK(tally.corrected == 2);
    CHECK(tally.removed == 0);

    for (const AppliedCorrection& one : applied)
        one.command->revert(*one.project);

    CHECK(sameSubtitles(first, firstBefore));
    CHECK(sameSubtitles(second, secondBefore));
}

TEST_CASE("a project with nothing accepted is absent from what was applied", "[text][assistant]") {
    CHECK(applyCorrections({}, true).empty());
}

TEST_CASE("a class unchecked corrects nothing of what it alone carries", "[text][assistant]") {
    // "Space between digits" is OCR alone; "Musical notes" is Human alone,
    // and disabled by its own shipped `.conf` — turned on here so that the
    // class filter, and not that default, is what the assertion reads.
    Project project = projectOf({"12 34", "♪ La la ♪"});

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    settings.human = true;
    settings.ocr = false;
    settings.patternActivations = {PatternActivation{.kind = PatternKind::CommonError,
                                                     .code = "Zyyy",
                                                     .name = "Musical notes",
                                                     .enabled = true}};

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    REQUIRE(proposed.size() == 1);
    CHECK(proposed.front().original == "♪ La la ♪");
}

TEST_CASE("a record of two classes applies if either is checked", "[text][assistant]") {
    // "Multiple consecutive spaces" carries both Human and OCR.
    Project project = projectOf({"Bonjour  Marie"});

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    settings.human = false;
    settings.ocr = true;

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    REQUIRE(proposed.size() == 1);
    CHECK(proposed.front().proposed == std::optional<std::string>{"Bonjour Marie"});
}

TEST_CASE("a mention that empties a translation blanks it rather than removing the subtitle",
          "[text][assistant]") {
    Project project = projectOf({"Something else"});
    project.subtitleAt(SubtitleIndex::fromValue(0)).translationText = "[Door slams]";

    CorrectionSettings settings;
    settings.mentions = {.enabled = true, .code = "Zyyy"};
    settings.soundInBrackets = true;

    const std::vector<CorrectionTarget> targets{wholeProject(project, Document::Translation)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    REQUIRE(proposed.size() == 1);
    CHECK(proposed.front().proposed == std::optional<std::string>{""});

    const std::vector<AppliedCorrection> applied =
        applyCorrections(proposed, /*removeBlankSubtitles=*/true);
    REQUIRE(applied.size() == 1);
    // The subtitle stays — only its translation went blank.
    REQUIRE(project.count() == 1);
    CHECK(project.subtitles().front().translationText.empty());
    CHECK(project.subtitles().front().mainText == "Something else");
}

TEST_CASE("a mention that empties the main text is proposed as a removal, kept when the "
          "checkbox is off",
          "[text][assistant]") {
    Project project = projectOf({"[Door slams]", "Hello there"});

    CorrectionSettings settings;
    settings.mentions = {.enabled = true, .code = "Zyyy"};
    settings.soundInBrackets = true;

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    REQUIRE(proposed.size() == 1);
    CHECK_FALSE(proposed.front().proposed.has_value());

    SECTION("removed when the checkbox is on") {
        const std::vector<AppliedCorrection> applied =
            applyCorrections(proposed, /*removeBlankSubtitles=*/true);
        REQUIRE(applied.size() == 1);
        CHECK(textsOf(project) == std::vector<std::string>{"Hello there"});
        CHECK(tallyOf(applied).removed == 1);
    }

    SECTION("left blank when the checkbox is off") {
        const std::vector<AppliedCorrection> applied =
            applyCorrections(proposed, /*removeBlankSubtitles=*/false);
        REQUIRE(applied.size() == 1);
        CHECK(textsOf(project) == std::vector<std::string>{"", "Hello there"});
        CHECK(tallyOf(applied).removed == 0);
    }
}

TEST_CASE("a subtitle a mention removes plays no part in the tasks that follow it",
          "[text][assistant]") {
    // Were "[Door slams]" still fed to capitalization, the first-of-run rule
    // would force a capital on whatever mentions left of it. Instead, it is
    // gone before capitalization ever runs, and "hello there" — now the run's
    // first survivor — is the one that gets it.
    Project project = projectOf({"[Door slams]", "hello there"});

    CorrectionSettings settings;
    settings.mentions = {.enabled = true, .code = "Zyyy"};
    settings.soundInBrackets = true;
    settings.capitalization = {.enabled = true, .code = "Latn"};

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    const auto forSecond = std::ranges::find_if(
        proposed, [](const ProposedCorrection& one) { return one.original == "hello there"; });
    REQUIRE(forSecond != proposed.end());
    CHECK(forSecond->proposed == std::optional<std::string>{"Hello there"});
}

TEST_CASE("an activation override turns a normally-active pattern off", "[text][assistant]") {
    Project project = projectOf({"Bonjour  Marie"});

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    settings.patternActivations = {PatternActivation{.kind = PatternKind::CommonError,
                                                     .code = "Zyyy",
                                                     .name = "Multiple consecutive spaces",
                                                     .enabled = false}};

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    CHECK(proposed.empty());
}

TEST_CASE("a target with nothing selected proposes nothing", "[text][assistant]") {
    Project project = projectOf({"Bonjour  Marie"});
    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};

    const std::vector<CorrectionTarget> targets{
        CorrectionTarget{.project = &project,
                         .selection = Selection::of(std::vector<subedit::core::SubtitleIndex>{}),
                         .document = Document::Main}};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    CHECK(proposed.empty());
}

TEST_CASE("the line-break task fits within a maximum length and line count", "[text][assistant]") {
    Project project = projectOf({"The night was cold and the road was long"});

    CorrectionSettings settings;
    settings.lineBreak = {.enabled = true, .code = "Zyyy"};
    settings.lineBreakMaxLength = 24.0;
    settings.lineBreakMaxLines = 2;

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const std::vector<ProposedCorrection> proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);

    REQUIRE(proposed.size() == 1);
    CHECK(proposed.front().proposed ==
          std::optional<std::string>{"The night was cold\nand the road was long"});
}
