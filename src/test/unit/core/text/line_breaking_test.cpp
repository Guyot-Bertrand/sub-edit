// Breaking lines to a length and a line count — issue #502, decision D5 of
// the spec of phase 12.
//
// **The cases are Gaupol's, not ours.** `line-break.cas` was written by the
// oracle of #495 from what Gaupol's `Liner` does with its own penalties; the
// test plays each case through `breakLines` and the shipped patterns and asks
// for the same text. A disagreement is this code's, never the case's.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_breaking.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "text_cases.hpp"

namespace {

using subedit::core::breakLines;
using subedit::core::BrokenTexts;
using subedit::core::CharacterLineMeasure;
using subedit::core::CorrectionPattern;
using subedit::core::FailureKind;
using subedit::core::IcuPatternEngine;
using subedit::core::LineBreakFields;
using subedit::core::LineMeasure;
using subedit::core::PatternCatalogue;
using subedit::core::PatternFlags;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::core::RealFileSystem;
using subedit::core::SkipLimits;

const PatternCatalogue& shippedPatterns() {
    static const PatternCatalogue catalogue = [] {
        const RealFileSystem files;
        return readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});
    }();
    return catalogue;
}

/// A record written in the test, for what the shipped files do not do.
CorrectionPattern recordOf(std::string expression, int group, double penalty) {
    CorrectionPattern pattern;
    pattern.code = "Test";
    pattern.rank = 1;
    pattern.name = "A record of the test";
    pattern.expression = std::move(expression);
    pattern.flags = PatternFlags{.dotAll = true, .multiline = true};
    pattern.fields = LineBreakFields{.group = group, .penalty = penalty};
    return pattern;
}

/// The three penalties `aeidon/test/test_liner.py` uses, next to the ones
/// Gaupol ships — `pattern-oracle.py`'s `TEST_PENALTIES`, and the `essai`
/// cases that ask for them.
const std::vector<CorrectionPattern>& testPenalties() {
    static const std::vector<CorrectionPattern> records{
        recordOf(R"(( )- )", 1, -1000.0),
        recordOf(R"([,.;:!?]( ))", 1, -100.0),
        recordOf(R"(\b(by|the|into|a)( ))", 2, 1000.0),
    };
    return records;
}

/// What a case's selection names: a record, a cascade — Gaupol's default
/// activation, or every record under `tous` —, `essai` for the three test
/// penalties above, or `aucune` for none at all. The same reading
/// `pattern-oracle.py`'s `line_break_penalties` gives.
std::vector<const CorrectionPattern*> patternsOf(std::string_view selection) {
    if (selection == "aucune")
        return {};
    if (selection == "essai") {
        std::vector<const CorrectionPattern*> chosen;
        for (const CorrectionPattern& one : testPenalties())
            chosen.push_back(&one);
        return chosen;
    }
    const PatternCatalogue& catalogue = shippedPatterns();
    if (selection.starts_with("cascade ")) {
        selection.remove_prefix(std::string_view{"cascade "}.size());
        std::vector<const CorrectionPattern*> chosen;
        for (const CorrectionPattern* one : catalogue.cascade(PatternKind::LineBreak, selection)) {
            if (one->enabled)
                chosen.push_back(one);
        }
        return chosen;
    }
    const std::size_t colon = selection.find(':');
    const std::string_view code = selection.substr(0, colon);
    const std::size_t rank = std::stoul(std::string{selection.substr(colon + 1)});
    for (const CorrectionPattern& one : catalogue.patterns()) {
        if (one.kind() == PatternKind::LineBreak && one.code == code && one.rank == rank)
            return {&one};
    }
    return {};
}

/// `<selection> <length>/<lines>` — a case's target, split the way
/// `pattern-oracle.py`'s `line_break_settings` splits it: the selection is
/// everything before the last space.
struct Settings {
    std::string selection;
    double maxLength = 0.0;
    int maxLines = 0;
};

Settings settingsOf(std::string_view target) {
    const std::size_t space = target.rfind(' ');
    REQUIRE(space != std::string_view::npos);
    const std::string_view sizing = target.substr(space + 1);
    const std::size_t slash = sizing.find('/');
    REQUIRE(slash != std::string_view::npos);
    return Settings{.selection = std::string{target.substr(0, space)},
                    .maxLength = std::stod(std::string{sizing.substr(0, slash)}),
                    .maxLines = std::stoi(std::string{sizing.substr(slash + 1)})};
}

} // namespace

TEST_CASE("lines are broken as Gaupol's Liner breaks them", "[text][line-break]") {
    const std::vector<subedit::test::TextCase> cases =
        subedit::test::textCasesOf("motifs/attendus/line-break.cas");
    REQUIRE(cases.size() >= 26);
    REQUIRE(shippedPatterns().diagnostics().empty());

    const CharacterLineMeasure measure;
    for (const subedit::test::TextCase& one : cases) {
        const std::size_t verb = std::min(one.name.find(" coupe — "), one.name.find(" intact — "));
        REQUIRE(verb != std::string::npos);
        const Settings settings = settingsOf(one.name.substr(0, verb));

        const std::vector<const CorrectionPattern*> chosen = patternsOf(settings.selection);

        const std::vector<std::string> given{one.input};
        const BrokenTexts done = breakLines(
            IcuPatternEngine{}, chosen, given, measure, settings.maxLength, settings.maxLines);

        INFO("cas ligne " << one.line << " : " << one.name);
        CHECK(done.failures.empty());
        REQUIRE(done.texts.size() == 1);
        CHECK(one.expected.has_value());
        CHECK(done.texts.front() == one.expected.value_or(""));
    }
}

namespace {

BrokenTexts brokenBy(const std::vector<CorrectionPattern>& records,
                     const std::vector<std::string>& texts,
                     const LineMeasure& measure,
                     double maxLength,
                     int maxLines,
                     std::optional<SkipLimits> skip = std::nullopt) {
    std::vector<const CorrectionPattern*> chosen;
    chosen.reserve(records.size());
    for (const CorrectionPattern& one : records)
        chosen.push_back(&one);
    return breakLines(IcuPatternEngine{}, chosen, texts, measure, maxLength, maxLines, skip);
}

/// Ten cents a character, one dollar for a digit — a measure with no
/// relation to how many characters a string holds, so a test that breaks
/// differently under it than under `CharacterLineMeasure` proves the
/// break-up follows whatever measure it is given.
class DigitsAreWideMeasure final : public LineMeasure {

public:
    [[nodiscard]] double lengthOf(std::string_view text) const override {
        double total = 0.0;
        for (const char c : text)
            total += (c >= '0' && c <= '9') ? 10.0 : 1.0;
        return total;
    }
};

} // namespace

TEST_CASE("the break-up follows whatever measure it is given, not a character count",
          "[text][line-break]") {
    const std::string text = "Room 100 is that way";
    const CharacterLineMeasure characters;
    const DigitsAreWideMeasure digitsWide;

    const BrokenTexts byCharacters = brokenBy({}, {text}, characters, 12, 2);
    const BrokenTexts byDigitsWide = brokenBy({}, {text}, digitsWide, 12, 2);

    CHECK(byCharacters.texts != byDigitsWide.texts);
    // Under the character count, "100" is cheap and a two-line split fits
    // comfortably. Weighted at ten per digit, "100" alone already costs 30 —
    // past every maximum a line could have here, on a line by itself or
    // shared with anything else — so no split at all keeps both lines within
    // 12, and the text comes back exactly as it was given.
    CHECK(byCharacters.texts == std::vector<std::string>{"Room 100\nis that way"});
    CHECK(byDigitsWide.texts == std::vector<std::string>{text});
}

TEST_CASE("skip leaves a text that already fits alone, whitespace and all", "[text][line-break]") {
    const CharacterLineMeasure measure;
    // Extra spaces a break-up would otherwise collapse: skip never even looks,
    // since the one line it makes already fits both limits.
    const BrokenTexts done =
        brokenBy({}, {"Hello   there"}, measure, 40, 2, SkipLimits{.maxLength = 40, .maxLines = 2});
    CHECK(done.texts == std::vector<std::string>{"Hello   there"});
}

TEST_CASE("skip breaks a text that violates a limit", "[text][line-break]") {
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy({},
                                      {"The night was cold and the road was long"},
                                      measure,
                                      24,
                                      2,
                                      SkipLimits{.maxLength = 24, .maxLines = 2});
    CHECK(done.texts == std::vector<std::string>{"The night was cold\nand the road was long"});
}

TEST_CASE("skip leaves a text alone that breaking would not improve", "[text][line-break]") {
    const CharacterLineMeasure measure;
    // A single, unbreakable word longer than the limit: no break-up helps it,
    // so skip's own rule — length down or lines down — leaves it as it came.
    const BrokenTexts done =
        brokenBy({}, {"Unbelievable"}, measure, 5, 2, SkipLimits{.maxLength = 5, .maxLines = 2});
    CHECK(done.texts == std::vector<std::string>{"Unbelievable"});
}

TEST_CASE("skip thresholds are their own, not the limits being broken to", "[text][line-break]") {
    const CharacterLineMeasure measure;
    const std::vector<std::string> text{"The night was cold and the road was long"}; // 40 wide
    // Broken to 24 wide, but a skip threshold of 40 holds this text back...
    CHECK(brokenBy({}, text, measure, 24, 2, SkipLimits{.maxLength = 40, .maxLines = 2}).texts ==
          text);
    // ...and one of 39 does not.
    CHECK(brokenBy({}, text, measure, 24, 2, SkipLimits{.maxLength = 39, .maxLines = 2}).texts ==
          std::vector<std::string>{"The night was cold\nand the road was long"});
}

TEST_CASE("a skip threshold left at its default never holds a text back", "[text][line-break]") {
    const CharacterLineMeasure measure;
    // Three lines, each short: only the line count can make this violate, and
    // the length threshold — Gaupol's "skip on length" turned off — is absent.
    const std::vector<std::string> text{"aa\nbb\ncc"};
    CHECK(brokenBy({}, text, measure, 24, 2, SkipLimits{.maxLines = 2}).texts !=
          brokenBy({}, text, measure, 24, 2, SkipLimits{.maxLines = 3}).texts);
    CHECK(brokenBy({}, text, measure, 24, 2, SkipLimits{.maxLines = 3}).texts == text);
}

TEST_CASE("a line-break pattern that cannot be applied is named, and the others are",
          "[text][line-break]") {
    const std::vector<CorrectionPattern> records{recordOf("(unclosed", 1, -1000.0),
                                                 recordOf(R"(( )- )", 1, -1000.0)};
    const BrokenTexts done =
        brokenBy(records, {"- Hello there - goodbye now"}, CharacterLineMeasure{}, 15, 2);

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::CompileError);
    CHECK(done.texts == std::vector<std::string>{"- Hello there\n- goodbye now"});
}

TEST_CASE("a line-break pattern that backtracks without end is given up on that text",
          "[text][line-break]") {
    const std::string catastrophic(40, 'a');
    const std::vector<CorrectionPattern> records{recordOf("(a+)+$( )", 1, -1000.0)};
    const BrokenTexts done =
        brokenBy(records, {catastrophic + "b two words"}, CharacterLineMeasure{}, 10, 2);

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TimedOut);
    CHECK(done.failures.front().text == std::optional<std::size_t>{0});
}

TEST_CASE("nothing is cut below the line count a text needs", "[text][line-break]") {
    const CharacterLineMeasure measure;
    // Two words, three lines allowed: `Liner` never tries fewer boxes than
    // lines, so nothing is cut even though the one line is over the limit.
    const BrokenTexts done = brokenBy({}, {"Unbelievable extraordinary"}, measure, 14, 3);
    CHECK(done.texts == std::vector<std::string>{"Unbelievable extraordinary"});
}

TEST_CASE("the line count is exceeded when the length demands it", "[text][line-break]") {
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy(
        {},
        {"The king's child went out into the forest and sat down by the side of the cool "
         "fountain; and when she was bored she took a golden ball, and threw it up high and "
         "caught it; and this ball was her favorite plaything."},
        measure,
        40,
        3);
    REQUIRE(done.texts.size() == 1);
    CHECK(std::ranges::count(done.texts.front(), '\n') > 2);
}

TEST_CASE("a maximum of one line keeps a short text unbroken", "[text][line-break]") {
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy({}, {"Hi there"}, measure, 20, 1);
    CHECK(done.texts == std::vector<std::string>{"Hi there"});
}

TEST_CASE("a text with nothing but blanks comes back empty", "[text][line-break]") {
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy({}, {"   "}, measure, 40, 2);
    CHECK(done.texts == std::vector<std::string>{""});
}

TEST_CASE("a line-break pattern whose chosen group is out of range contributes nothing",
          "[text][line-break]") {
    const CharacterLineMeasure measure;
    const std::vector<CorrectionPattern> records{recordOf(R"(( ))", 5, -1000.0)};
    const BrokenTexts withOutOfRange = brokenBy(records, {"Hello there my friend"}, measure, 12, 2);
    const BrokenTexts withNone = brokenBy({}, {"Hello there my friend"}, measure, 12, 2);

    CHECK(withOutOfRange.failures.empty());
    CHECK(withOutOfRange.texts == withNone.texts);
}

TEST_CASE("a line-break pattern whose chosen group does not always participate is skipped there",
          "[text][line-break]") {
    // Group 1 is only ever the first alternative, `(a)`; every match here
    // takes the second, `( )`, so group 1 never captured anything at all.
    const std::vector<CorrectionPattern> records{recordOf(R"((a)|( ))", 1, -1000.0)};
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy(records, {"Hello there my friend"}, measure, 12, 2);
    CHECK(done.failures.empty());
}

TEST_CASE("a line-break pattern matching nothing is skipped one position at a time",
          "[text][line-break]") {
    // `x*` matches the empty string everywhere: the same guard against
    // finding the same place for ever that the correction engines need
    // before a replacement is needed here before placing a penalty twice.
    const std::vector<CorrectionPattern> records{recordOf("x*", 0, -1000.0)};
    const CharacterLineMeasure measure;
    const BrokenTexts done = brokenBy(records, {"Hello there my friend"}, measure, 12, 2);
    CHECK(done.failures.empty());
}
