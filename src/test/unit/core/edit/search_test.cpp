// Finding and replacing — issue #384.
//
// The rule of a replacement that crosses a tag is the parser's, written case by
// case in two corpora; what is under test here is the search around it: the
// two options, the visible text looked in, the walk through the target and
// round again, and the one history entry of `Replace All`.

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <text_cases.hpp>
#include <utility>
#include <vector>

namespace {

using subedit::core::CommandKind;
using subedit::core::findNext;
using subedit::core::findPrevious;
using subedit::core::PatternError;
using subedit::core::Project;
using subedit::core::replaceAll;
using subedit::core::ReplacedAll;
using subedit::core::ReplacedMatch;
using subedit::core::replaceMatch;
using subedit::core::SearchOptions;
using subedit::core::SearchPattern;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::TextMatch;
using subedit::core::Timestamp;

constexpr SearchOptions kPlain{.regex = false, .ignoreCase = false};
constexpr SearchOptions kRegex{.regex = true, .ignoreCase = false};

[[nodiscard]] Project projectOf(std::initializer_list<std::string_view> texts,
                                SubtitleFormat format = SubtitleFormat::SubRip) {
    std::vector<Subtitle> subtitles;
    std::int64_t start = 0;
    for (const std::string_view text : texts) {
        subtitles.push_back(Subtitle{.start = Timestamp::fromMilliseconds(start),
                                     .end = Timestamp::fromMilliseconds(start + 1000),
                                     .mainText = std::string{text}});
        start += 2000;
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(SourceFile{.format = format});
    return project;
}

[[nodiscard]] SearchPattern patternOf(std::string_view text, SearchOptions options) {
    std::expected<SearchPattern, PatternError> compiled = SearchPattern::compile(text, options);
    REQUIRE(compiled.has_value());
    return std::move(*compiled);
}

[[nodiscard]] TextMatch at(std::size_t index, std::size_t start, std::size_t end) {
    return TextMatch{.index = SubtitleIndex::fromValue(index), .start = start, .end = end};
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

/// Replaces every match over the whole of a one-subtitle project, as the
/// corpora ask.
[[nodiscard]] std::string replacedEverywhere(const std::string& text,
                                             const std::string& pattern,
                                             const std::string& replacement,
                                             SubtitleFormat format) {
    Project project = projectOf({text}, format);
    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf(pattern, kPlain), replacement);
    if (replaced.command != nullptr)
        replaced.command->apply(project);
    return project.subtitleAt(SubtitleIndex::fromValue(0)).mainText;
}

} // namespace

TEST_CASE("the defaults are Gaupol's: plain text, the case ignored", "[edit][search]") {
    const SearchOptions defaults;

    CHECK_FALSE(defaults.regex);
    CHECK(defaults.ignoreCase);
}

TEST_CASE("an empty pattern or a broken expression cannot be searched for", "[edit][search]") {
    const auto empty = SearchPattern::compile("", SearchOptions{});
    REQUIRE_FALSE(empty.has_value());
    CHECK(empty.error().kind == PatternError::Kind::Empty);

    const auto broken = SearchPattern::compile("(Bonjour", kRegex);
    REQUIRE_FALSE(broken.has_value());
    CHECK(broken.error().kind == PatternError::Kind::InvalidExpression);
    CHECK_FALSE(broken.error().reason.empty());

    // The same text, read as plain text, is only text.
    CHECK(SearchPattern::compile("(Bonjour", kPlain).has_value());
}

TEST_CASE("ignoring the case knows accented letters", "[edit][search]") {
    const Project project = projectOf({"Élise arrive."});

    CHECK(findNext(project, Selection::all(project), patternOf("élise", SearchOptions{}), {}) ==
          at(0, 0, 6));
    CHECK_FALSE(
        findNext(project, Selection::all(project), patternOf("élise", kPlain), {}).has_value());
}

TEST_CASE("the text searched is the visible one", "[edit][search]") {
    // A tag is not text, and a word cut by one is still one word.
    const Project project = projectOf({"<i>Bon</i>jour Marie"});
    const Selection all = Selection::all(project);

    CHECK_FALSE(findNext(project, all, patternOf("<i>", kPlain), {}).has_value());
    CHECK(findNext(project, all, patternOf("Bonjour", kPlain), {}) == at(0, 0, 7));
}

TEST_CASE("a plain pattern is not an expression", "[edit][search]") {
    const Project project = projectOf({"Un. Deux."});

    CHECK(findNext(project, Selection::all(project), patternOf(".", kPlain), {}) == at(0, 2, 3));
}

TEST_CASE("finding next walks the target and comes round again", "[edit][search]") {
    const Project project = projectOf({"Marie.", "Rien.", "Marie et Marie."});
    const Selection all = Selection::all(project);
    const SearchPattern pattern = patternOf("Marie", kPlain);

    std::optional<TextMatch> found = findNext(project, all, pattern, {});
    CHECK(found == at(0, 0, 5));
    found = findNext(project, all, pattern, found);
    CHECK(found == at(2, 0, 5));
    found = findNext(project, all, pattern, found);
    CHECK(found == at(2, 9, 14));
    // Past the last, from the top.
    found = findNext(project, all, pattern, found);
    CHECK(found == at(0, 0, 5));
}

TEST_CASE("finding previous walks the other way round", "[edit][search]") {
    const Project project = projectOf({"Marie.", "Rien.", "Marie et Marie."});
    const Selection all = Selection::all(project);
    const SearchPattern pattern = patternOf("Marie", kPlain);

    std::optional<TextMatch> found = findPrevious(project, all, pattern, {});
    CHECK(found == at(2, 9, 14));
    found = findPrevious(project, all, pattern, found);
    CHECK(found == at(2, 0, 5));
    found = findPrevious(project, all, pattern, found);
    CHECK(found == at(0, 0, 5));
    found = findPrevious(project, all, pattern, found);
    CHECK(found == at(2, 9, 14));
}

TEST_CASE("a single match is found again, and nothing is nothing", "[edit][search]") {
    const Project project = projectOf({"Marie.", "Rien."});
    const Selection all = Selection::all(project);

    CHECK(findNext(project, all, patternOf("Marie", kPlain), at(0, 0, 5)) == at(0, 0, 5));
    CHECK_FALSE(findNext(project, all, patternOf("Sophie", kPlain), {}).has_value());
    CHECK_FALSE(findPrevious(project, all, patternOf("Sophie", kPlain), {}).has_value());
}

TEST_CASE("the search stays in its target", "[edit][search]") {
    const Project project = projectOf({"Marie.", "Rien.", "Marie."});
    const Selection second =
        Selection::range(SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2));
    const SearchPattern pattern = patternOf("Marie", kPlain);

    std::optional<TextMatch> found = findNext(project, second, pattern, {});
    CHECK(found == at(2, 0, 5));
    CHECK(findNext(project, second, pattern, found) == at(2, 0, 5));
}

TEST_CASE("replacing a match rewrites that one and says where it now lies", "[edit][search]") {
    Session session{projectOf({"Marie et Marie."})};
    const SearchPattern pattern = patternOf("Marie", kPlain);

    std::optional<ReplacedMatch> replaced =
        replaceMatch(session.project(), pattern, at(0, 9, 14), "Sophie");
    REQUIRE(replaced.has_value());
    // A guard the analysis can read: it follows neither a `REQUIRE` nor a
    // `.value()` into the accesses after them.
    if (!replaced.has_value())
        return;
    ReplacedMatch result = std::move(*replaced);
    REQUIRE(result.command != nullptr);
    CHECK(result.command->kind() == CommandKind::Replace);
    CHECK(result.written == at(0, 9, 15));
    static_cast<void>(session.apply(std::move(result.command)));

    CHECK(textsOf(session.project()) == std::vector<std::string>{"Marie et Sophie."});

    static_cast<void>(session.undo());
    CHECK(textsOf(session.project()) == std::vector<std::string>{"Marie et Marie."});
}

TEST_CASE("a match the text no longer holds is not replaced", "[edit][search]") {
    const Project project = projectOf({"Rien."});

    CHECK_FALSE(
        replaceMatch(project, patternOf("Marie", kPlain), at(0, 0, 5), "Sophie").has_value());
    CHECK_FALSE(replaceMatch(project, patternOf("Rien", kPlain), at(4, 0, 4), "Tout").has_value());
}

TEST_CASE("replacing all is one entry in the history, and counts", "[edit][search]") {
    Session session{projectOf({"Marie.", "Rien.", "Marie et Marie."})};

    ReplacedAll replaced = replaceAll(
        session.project(), Selection::all(session.project()), patternOf("Marie", kPlain), "Sophie");
    REQUIRE(replaced.command != nullptr);
    CHECK(replaced.command->kind() == CommandKind::ReplaceAll);
    CHECK(replaced.count == 3);
    CHECK(replaced.matched == 3);
    static_cast<void>(session.apply(std::move(replaced.command)));

    CHECK(textsOf(session.project()) ==
          std::vector<std::string>{"Sophie.", "Rien.", "Sophie et Sophie."});

    static_cast<void>(session.undo());
    CHECK(textsOf(session.project()) ==
          std::vector<std::string>{"Marie.", "Rien.", "Marie et Marie."});
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("a replacement containing the pattern is not found again", "[edit][search]") {
    Project project = projectOf({"Marie."});

    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf("Marie", kPlain), "Marie-Claire");
    REQUIRE(replaced.command != nullptr);
    replaced.command->apply(project);

    CHECK(replaced.count == 1);
    CHECK(textsOf(project) == std::vector<std::string>{"Marie-Claire."});
}

TEST_CASE("replacing what nothing matches builds nothing", "[edit][search]") {
    const Project project = projectOf({"<i >Rien</I >."});

    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf("Marie", kPlain), "Sophie");

    CHECK(replaced.command == nullptr);
    CHECK(replaced.count == 0);
    CHECK(replaced.matched == 0);
}

TEST_CASE("a replacement that only widens a tag across a word still counts as a change",
          "[edit][search]") {
    // Issue #402: a boundary that cuts a word is pushed to its edge as soon as
    // a replacement reaches it, whatever the replacement's own text is. The
    // visible text here does not change — "arie" replaces "arie" — but the
    // italic that used to stop mid-word now covers the whole of it, which is a
    // real change to the file.
    Project project = projectOf({"<i>Ma</i>rie"});

    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf("arie", kPlain), "arie");
    REQUIRE(replaced.command != nullptr);
    replaced.command->apply(project);

    CHECK(replaced.count == 1);
    CHECK(replaced.matched == 1);
    CHECK(textsOf(project) == std::vector<std::string>{"<i>Marie</i>"});
}

TEST_CASE("replacing a match by itself changes nothing and counts nothing", "[edit][search]") {
    const Project project = projectOf({"Bonjour Marie."});

    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf("Marie", kPlain), "Marie");

    CHECK(replaced.count == 0);
    CHECK(replaced.command == nullptr);
    // The pattern is in the document, though: found is not the same as changed,
    // and the window tells "nothing to change" from "not found" by this.
    CHECK(replaced.matched == 1);
}

TEST_CASE("an expression's groups and escapes are expanded", "[edit][search]") {
    Project project = projectOf({"Marie Curie"});

    const ReplacedAll replaced = replaceAll(
        project, Selection::all(project), patternOf(R"((\w+) (\w+))", kRegex), R"($2,\n$1 \$$9)");
    REQUIRE(replaced.command != nullptr);
    replaced.command->apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"Curie,\nMarie $"});
}

TEST_CASE("an expression that matches nothing wide still advances", "[edit][search]") {
    // `^` at every line: a dash at the head of each, and not an endless one at
    // the head of the first.
    Project project = projectOf({"Oui ?\nNon."});

    const ReplacedAll replaced =
        replaceAll(project, Selection::all(project), patternOf("^", kRegex), "- ");
    REQUIRE(replaced.command != nullptr);
    replaced.command->apply(project);

    CHECK(replaced.count == 2);
    CHECK(textsOf(project) == std::vector<std::string>{"- Oui ?\n- Non."});
}

TEST_CASE("the cases written in the HTML vocabulary pass through replace all", "[edit][search]") {
    subedit::test::checkReplacementCases(
        subedit::test::replacementCasesOf("textes/recherche.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacedEverywhere(text, pattern, replacement, SubtitleFormat::SubRip);
        });
}

TEST_CASE("the cases written in braces pass through replace all", "[edit][search]") {
    subedit::test::checkReplacementCases(
        subedit::test::replacementCasesOf("textes/recherche-accolades.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacedEverywhere(
                text, pattern, replacement, SubtitleFormat::AdvancedSubStationAlpha);
        });
}

TEST_CASE("an empty match past an accented letter moves by a whole character", "[edit][search]") {
    // `\b` matches nothing wide, at each edge of a word. After the edge before
    // « été », the next search starts one character on — two bytes, not one —
    // and after the edge at the very end, there is nowhere left to start.
    Project project = projectOf({"été ou"});
    const SearchPattern edges = patternOf(R"(\b)", kRegex);

    CHECK(findNext(project, Selection::all(project), edges, at(0, 0, 0)) == at(0, 5, 5));

    const ReplacedAll replaced = replaceAll(project, Selection::all(project), edges, "|");
    REQUIRE(replaced.command != nullptr);
    replaced.command->apply(project);

    CHECK(replaced.count == 4);
    CHECK(textsOf(project) == std::vector<std::string>{"|été| |ou|"});
}

TEST_CASE("an empty target holds no match", "[edit][search]") {
    const Project project = projectOf({"Marie."});
    const Selection nothing = Selection::of({});

    CHECK_FALSE(findNext(project, nothing, patternOf("Marie", kPlain), {}).has_value());
}

TEST_CASE("a match outside the target starts the walk again from its edge", "[edit][search]") {
    // The target changed since the last match was found: forwards, the walk
    // starts from the top of the new target; backwards, from its bottom.
    const Project project = projectOf({"Marie.", "Marie.", "Rien.", "Marie."});
    const Selection firstTwo =
        Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1));
    const Selection lastTwo =
        Selection::range(SubtitleIndex::fromValue(2), SubtitleIndex::fromValue(3));
    const SearchPattern pattern = patternOf("Marie", kPlain);

    CHECK(findNext(project, firstTwo, pattern, at(3, 0, 5)) == at(0, 0, 5));
    CHECK(findPrevious(project, lastTwo, pattern, at(0, 0, 5)) == at(3, 0, 5));
}

TEST_CASE("a match that moved within its text is not replaced", "[edit][search]") {
    // The pattern is still there, but no longer where it was found.
    const Project project = projectOf({"Rien, Marie."});

    CHECK_FALSE(
        replaceMatch(project, patternOf("Marie", kPlain), at(0, 0, 5), "Sophie").has_value());
}

TEST_CASE("a compiled pattern moves, and keeps its options", "[edit][search]") {
    SearchPattern pattern = patternOf("Marie", kPlain);
    CHECK(pattern.options() == kPlain);

    pattern = patternOf("M.rie", kRegex);
    CHECK(pattern.options() == kRegex);

    const Project project = projectOf({"Marie."});
    CHECK(findNext(project, Selection::all(project), pattern, {}) == at(0, 0, 5));
}
