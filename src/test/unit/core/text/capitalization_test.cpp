// Capitalizing texts — issue #500, decision D7 of the spec of phase 12.
//
// **The cases are Gaupol's, not ours.** `capitalization.cas` was written by
// the oracle of #494 from what Gaupol does with its own files, before this
// engine existed; the test plays each case through the engine and the
// shipped patterns and asks for the same text. A disagreement is this code's,
// never the case's.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/capitalization.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "text_cases.hpp"

namespace {

using subedit::core::CapitalizationFields;
using subedit::core::CapitalizeAt;
using subedit::core::correctCapitalization;
using subedit::core::CorrectedTexts;
using subedit::core::CorrectionPattern;
using subedit::core::FailureKind;
using subedit::core::IcuPatternEngine;
using subedit::core::PatternCatalogue;
using subedit::core::PatternFlags;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::core::RealFileSystem;
using subedit::core::SubtitleFormat;
using subedit::test::TextCase;
using subedit::test::textCasesOf;

const PatternCatalogue& shippedPatterns() {
    static const PatternCatalogue catalogue = [] {
        const RealFileSystem files;
        return readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});
    }();
    return catalogue;
}

/// The patterns a case's target names: one record, or a cascade — the same
/// reading `common_errors_test.cpp` gives, for `PatternKind::Capitalization`.
std::vector<const CorrectionPattern*> patternsOf(std::string_view target) {
    const PatternCatalogue& catalogue = shippedPatterns();
    if (target.starts_with("cascade ")) {
        target.remove_prefix(std::string_view{"cascade "}.size());
        std::vector<const CorrectionPattern*> chosen;
        for (const CorrectionPattern* one :
             catalogue.cascade(PatternKind::Capitalization, target)) {
            if (one->enabled)
                chosen.push_back(one);
        }
        return chosen;
    }
    const std::size_t colon = target.find(':');
    const std::string_view code = target.substr(0, colon);
    const std::size_t rank = std::stoul(std::string{target.substr(colon + 1)});
    for (const CorrectionPattern& one : catalogue.patterns()) {
        if (one.kind() == PatternKind::Capitalization && one.code == code && one.rank == rank)
            return {&one};
    }
    return {};
}

/// A record written in the test, for what the shipped files do not do.
CorrectionPattern recordOf(std::string expression, CapitalizeAt at) {
    CorrectionPattern pattern;
    pattern.code = "Test";
    pattern.rank = 1;
    pattern.name = "A record of the test";
    pattern.expression = std::move(expression);
    pattern.flags = PatternFlags{.dotAll = true, .multiline = true};
    pattern.fields = CapitalizationFields{.capitalize = at};
    return pattern;
}

CorrectedTexts capitalizedBy(const std::vector<CorrectionPattern>& records,
                             const std::vector<std::string>& texts) {
    std::vector<const CorrectionPattern*> chosen;
    chosen.reserve(records.size());
    for (const CorrectionPattern& one : records)
        chosen.push_back(&one);
    return correctCapitalization(IcuPatternEngine{}, chosen, texts, SubtitleFormat::SubRip);
}

/// The `[k/m]` a case's name ends with, when it has one — several texts
/// separated by `;` in `capitalization.entrees` become one case per text,
/// the oracle numbering them so the test can play them back as the
/// consecutive subtitles they were.
struct SequenceTag {
    std::string base;
    int index = 0;
};

std::optional<SequenceTag> sequenceTagOf(const std::string& name) {
    if (!name.ends_with(']'))
        return std::nullopt;
    const std::size_t open = name.rfind(" [");
    if (open == std::string::npos)
        return std::nullopt;
    const std::string inner = name.substr(open + 2, name.size() - open - 3);
    const std::size_t slash = inner.find('/');
    if (slash == std::string::npos)
        return std::nullopt;
    const std::string first = inner.substr(0, slash);
    const std::string rest = inner.substr(slash + 1);
    const auto isDigits = [](const std::string& s) {
        return !s.empty() && std::ranges::all_of(s, [](char c) { return std::isdigit(c) != 0; });
    };
    if (!isDigits(first) || !isDigits(rest))
        return std::nullopt;
    return SequenceTag{.base = name.substr(0, open), .index = std::stoi(first)};
}

} // namespace

TEST_CASE("capitalization is applied as Gaupol's capitalize does it", "[text][pattern][engine]") {
    const std::vector<TextCase> cases = textCasesOf("motifs/attendus/capitalization.cas");
    REQUIRE(cases.size() > 15);
    REQUIRE(shippedPatterns().diagnostics().empty());

    // **One case is excepted, and the exception is written down, not silent.**
    // Decision D9 of the spec: Gaupol's `SubRip.clean` moves a space that sits
    // right after an opening tag to before it, cosmetic touch-up this engine
    // does not port — `MarkupParser` leaves a tag exactly where a
    // transformation put it. The PR of #501 names it.
    const std::vector<std::string> exceptedByD9{
        "Latn:1 corrige — le mot capitalisé est balisé",
    };

    // Cases of the same run share a base name and are already in order — the
    // oracle wrote them that way. Grouping them back into one run is what
    // lets a single pass of the corpus play both the single-text cases and
    // the ones that carry state from one subtitle to the next.
    std::size_t index = 0;
    while (index < cases.size()) {
        const TextCase& first = cases[index];
        const std::optional<SequenceTag> tag = sequenceTagOf(first.name);

        std::string runName = first.name;
        std::vector<std::string> inputs{first.input};
        std::vector<std::optional<std::string>> expecteds{first.expected};
        const int runLine = first.line;
        ++index;

        if (tag) {
            runName = tag->base;
            while (index < cases.size()) {
                const std::optional<SequenceTag> next = sequenceTagOf(cases[index].name);
                if (!next || next->base != tag->base)
                    break;
                inputs.push_back(cases[index].input);
                expecteds.push_back(cases[index].expected);
                ++index;
            }
        }

        const std::size_t verb = std::min(runName.find(" corrige — "), runName.find(" intact — "));
        REQUIRE(verb != std::string::npos);
        const std::string target = runName.substr(0, verb);

        const std::vector<const CorrectionPattern*> chosen = patternsOf(target);
        REQUIRE_FALSE(chosen.empty());

        const CorrectedTexts done =
            correctCapitalization(IcuPatternEngine{}, chosen, inputs, SubtitleFormat::SubRip);

        INFO("cas ligne " << runLine << " : " << runName);
        CHECK(done.failures.empty());
        REQUIRE(done.texts.size() == inputs.size());
        for (std::size_t k = 0; k < inputs.size(); ++k) {
            // Capitalization never removes a subtitle: `supprimé` is the
            // mentions'.
            CHECK(expecteds[k].has_value());
            if (std::ranges::find(exceptedByD9, runName) != exceptedByD9.end())
                continue;
            CHECK(done.texts[k] == expecteds[k].value_or(""));
        }
    }
}

TEST_CASE("only the capitalization patterns are applied", "[text][pattern][engine]") {
    CorrectionPattern other = recordOf("a", CapitalizeAt::Start);
    other.fields = subedit::core::CommonErrorFields{.classes = {}, .replacement = "b"};

    // Index 0 still gets its automatic capital — that rule owes nothing to
    // any pattern — but the record itself, being of another kind, changes
    // nothing past it.
    CHECK(capitalizedBy({other}, {"abc"}).texts == std::vector<std::string>{"Abc"});
}

TEST_CASE("Start capitalizes at the match, After capitalizes past it", "[text][pattern][engine]") {
    // The text under test sits at index 1, out of the automatic capital
    // index 0 gets regardless of any pattern — an empty text ahead of it
    // matches nothing and leaves no state behind.
    const std::vector<CorrectionPattern> start{recordOf("b", CapitalizeAt::Start)};
    CHECK(capitalizedBy(start, {"", "abc"}).texts == std::vector<std::string>{"", "aBc"});

    const std::vector<CorrectionPattern> after{recordOf("b", CapitalizeAt::After)};
    CHECK(capitalizedBy(after, {"", "abc"}).texts == std::vector<std::string>{"", "abC"});
}

TEST_CASE("the first subtitle of the document is always capitalized", "[text][pattern][engine]") {
    // No pattern matches, and it still gets a capital: Gaupol capitalizes
    // index 0 of the run unconditionally.
    CHECK(capitalizedBy({}, {"well then"}).texts == std::vector<std::string>{"Well then"});
}

TEST_CASE("After leaves a capital for the next text, across the run", "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> after{recordOf(R"(\.)", CapitalizeAt::After)};
    const CorrectedTexts done = capitalizedBy(after, {"He left.", "and so did she."});

    CHECK(done.texts == std::vector<std::string>{"He left.", "And so did she."});
}

TEST_CASE("an ellipsis right before a letter blocks the automatic capital",
          "[text][pattern][engine]") {
    // Gaupol's `_re_capitalizable` refuses a letter immediately preceded by
    // three literal dots, within what it was asked to search from — the
    // dedicated "after an ellipsis" record is what capitalizes it instead.
    // Two dots do not block: the lookbehind asks for exactly three.
    CHECK(capitalizedBy({}, {"...maybe"}).texts == std::vector<std::string>{"...maybe"});
    CHECK(capitalizedBy({}, {"..maybe"}).texts == std::vector<std::string>{"..Maybe"});
    // The single character `…` blocks exactly as the three dots do.
    CHECK(capitalizedBy({}, {"…maybe"}).texts == std::vector<std::string>{"…maybe"});
}

TEST_CASE("a word character can be an underscore or a digit, never punctuation",
          "[text][pattern][engine]") {
    // `\w` is `[\p{L}\p{N}_]` — D1 of the spec. An underscore right at the
    // start still counts as the letter to capitalize, title-cased like any
    // other: title-casing `_` leaves it as it is.
    CHECK(capitalizedBy({}, {"_bc"}).texts == std::vector<std::string>{"_bc"});
    CHECK(capitalizedBy({}, {"9bc"}).texts == std::vector<std::string>{"9bc"});
}

TEST_CASE("a match with nothing in it is skipped one character at a time, and ends",
          "[text][pattern][engine]") {
    // `x*` matches the empty string everywhere: the same guard against finding
    // the same place for ever that `common_errors.cpp` needs before a
    // replacement is needed here before capitalizing nothing twice over.
    const std::vector<CorrectionPattern> records{recordOf("x*", CapitalizeAt::Start)};
    CHECK(capitalizedBy(records, {"", "abc"}).texts == std::vector<std::string>{"", "ABC"});
}

TEST_CASE("a capitalization pattern that backtracks without end is given up on that text",
          "[text][pattern][engine]") {
    // `$` anchors the end: appending `b` is what forces the backtracking,
    // exactly as it does in `common_errors_test.cpp` — forty `a` alone would
    // match on the first attempt, and give up nothing to time out on.
    const std::string catastrophic(40, 'a');
    const std::vector<CorrectionPattern> records{recordOf("(a+)+$", CapitalizeAt::Start)};
    const CorrectedTexts done = capitalizedBy(records, {"", catastrophic + "b"});

    // The text at index 1 is left exactly as it was given: the automatic
    // capital of index 0 does not reach it, and the pattern that gave up
    // leaves no trace.
    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TimedOut);
    CHECK(done.failures.front().text == std::optional<std::size_t>{1});
    CHECK(done.texts == std::vector<std::string>{"", catastrophic + "b"});
}

TEST_CASE("a capitalization pattern that cannot be applied is named, and the others are",
          "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> records{
        recordOf("(unclosed", CapitalizeAt::Start), // the engine refuses it
        recordOf("b", CapitalizeAt::Start)};
    const CorrectedTexts done = capitalizedBy(records, {"", "abc"});

    CHECK(done.texts == std::vector<std::string>{"", "aBc"});
    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::CompileError);
    CHECK_FALSE(done.failures.front().text.has_value());
}
