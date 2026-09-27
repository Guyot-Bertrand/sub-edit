// The correction of common errors — issue #499, decisions D1 and D3.
//
// **The cases are Gaupol's, not ours.** `common-error.cas` was written by the
// oracle of #494 from what Gaupol does with its own files, before this engine
// existed; the test plays each case through the engine and the shipped patterns
// and asks for the same text. A disagreement is this code's, never the case's.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "text_cases.hpp"

namespace {

using subedit::core::CommonErrorFields;
using subedit::core::correctCommonErrors;
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

const PatternCatalogue& shippedPatterns() {
    static const PatternCatalogue catalogue = [] {
        const RealFileSystem files;
        return readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});
    }();
    return catalogue;
}

/// The patterns a case's target names: one record, or a cascade — as Gaupol
/// activates it by default, or every record of it under `tous`.
std::vector<const CorrectionPattern*> patternsOf(std::string_view target) {
    const PatternCatalogue& catalogue = shippedPatterns();
    if (target.starts_with("cascade ")) {
        target.remove_prefix(std::string_view{"cascade "}.size());
        const bool everything = target.ends_with(" tous");
        if (everything)
            target.remove_suffix(std::string_view{" tous"}.size());
        std::vector<const CorrectionPattern*> chosen;
        for (const CorrectionPattern* one : catalogue.cascade(PatternKind::CommonError, target)) {
            if (everything || one->enabled)
                chosen.push_back(one);
        }
        return chosen;
    }
    const std::size_t colon = target.find(':');
    const std::string_view code = target.substr(0, colon);
    const std::size_t rank = std::stoul(std::string{target.substr(colon + 1)});
    for (const CorrectionPattern& one : catalogue.patterns()) {
        if (one.kind() == PatternKind::CommonError && one.code == code && one.rank == rank)
            return {&one};
    }
    return {};
}

/// A record written in the test, for what the shipped files do not do.
CorrectionPattern recordOf(std::string expression, std::string replacement, bool repeat = false) {
    CorrectionPattern pattern;
    pattern.code = "Test";
    pattern.rank = 1;
    pattern.name = "A record of the test";
    pattern.expression = std::move(expression);
    pattern.flags = PatternFlags{.dotAll = true, .multiline = true};
    pattern.fields = CommonErrorFields{
        .classes = {.human = true}, .replacement = std::move(replacement), .repeat = repeat};
    return pattern;
}

CorrectedTexts correctedBy(const std::vector<CorrectionPattern>& records,
                           const std::vector<std::string>& texts) {
    std::vector<const CorrectionPattern*> chosen;
    chosen.reserve(records.size());
    for (const CorrectionPattern& one : records)
        chosen.push_back(&one);
    return correctCommonErrors(IcuPatternEngine{}, chosen, texts, SubtitleFormat::SubRip);
}

} // namespace

TEST_CASE("the common errors are corrected as Gaupol corrects them", "[text][pattern][engine]") {
    const std::vector<subedit::test::TextCase> cases =
        subedit::test::textCasesOf("motifs/attendus/common-error.cas");
    REQUIRE(cases.size() > 100);
    REQUIRE(shippedPatterns().diagnostics().empty());

    // **One case is excepted, and the exception is written down, not silent.**
    // Decision D9 of the spec: Gaupol's own tag-aware parser pulls a tag it
    // finds inside a match back to the start of what the match removed;
    // `MarkupParser::transform` keeps it as far as the new text reaches — the
    // rule phase 10 gave it, and `recherche.cas` holds it to. The PR of #501
    // names it.
    const std::vector<std::string> exceptedByD9{
        "Zyyy:3 corrige — balise à l'intérieur de la correspondance",
    };

    for (const subedit::test::TextCase& one : cases) {
        // `<target> <corrige|intact> — <label>`, the target being a record or
        // a cascade.
        const std::size_t verb =
            std::min(one.name.find(" corrige — "), one.name.find(" intact — "));
        REQUIRE(verb != std::string::npos);
        const std::string target = one.name.substr(0, verb);

        const std::vector<const CorrectionPattern*> chosen = patternsOf(target);
        REQUIRE_FALSE(chosen.empty());

        const std::vector<std::string> given{one.input};
        const CorrectedTexts done =
            correctCommonErrors(IcuPatternEngine{}, chosen, given, SubtitleFormat::SubRip);

        INFO("cas ligne " << one.line << " : " << one.name);
        CHECK(done.failures.empty());
        REQUIRE(done.texts.size() == 1);
        // A case of common errors is never a removal: `supprimé` is the mentions'.
        CHECK(one.expected.has_value());
        if (std::ranges::find(exceptedByD9, one.name) != exceptedByD9.end())
            continue;
        CHECK(done.texts.front() == one.expected.value_or(""));
    }
}

TEST_CASE("a letter written as a base and an accent is one word character, as in Python",
          "[text][pattern][engine]") {
    // ADR 0036: ICU counts the combining accent as a word character, Python does
    // not, and the expression is rewritten to say what Python says. `\bi\b` finds
    // the pronoun `i` in Python, since the accent after it is not a word
    // character — and here.
    const std::vector<CorrectionPattern> records{recordOf(R"(\bi\b)", "I")};
    const CorrectedTexts done = correctedBy(records, {"i\xCC\x82le et i am"});

    CHECK(done.texts == std::vector<std::string>{"I\xCC\x82le et I am"});
}

TEST_CASE("a replacement is applied to the text the previous one left, and Repeat until it settles",
          "[text][pattern][engine]") {
    // `a a a a` → each pass removes one space per pair, and lookbehind sees the
    // text as already rewritten: a global replace-all would leave `aa aa`.
    const std::vector<CorrectionPattern> records{recordOf(R"((?<=a) (?=a))", "", true)};
    const CorrectedTexts done = correctedBy(records, {"a a a a"});

    CHECK(done.texts == std::vector<std::string>{"aaaa"});
    CHECK(done.failures.empty());
}

TEST_CASE("a pattern matching nothing is skipped one character at a time, and ends",
          "[text][pattern][engine]") {
    // `x*` matches the empty string everywhere, and each match is replaced: the
    // guard against finding the same place for ever is Gaupol's `Finder`.
    const std::vector<CorrectionPattern> records{recordOf("x*", "-")};
    const CorrectedTexts done = correctedBy(records, {"ab", "\xC3\xA9"});

    // Gaupol: `-a-b-` for ASCII, and a two-byte letter is one character.
    CHECK(done.texts == std::vector<std::string>{"-a-b-", "-\xC3\xA9-"});
}

TEST_CASE("only the common-error patterns are applied", "[text][pattern][engine]") {
    CorrectionPattern other = recordOf("a", "b");
    other.fields = subedit::core::HearingImpairedFields{.replacement = "b"};

    CHECK(correctedBy({other}, {"a"}).texts == std::vector<std::string>{"a"});
}

TEST_CASE("a pattern that cannot be applied is named, and the others are",
          "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> records{
        recordOf(R"((?P<no_underscore>a))", "b"), // ICU names have no underscore
        recordOf("(unclosed", "b"),               // ICU refuses it
        recordOf("(a)", R"(\2)"),                 // there is no group 2
        recordOf("(a)", R"(\q)"),                 // Python has no \q
        recordOf("a", "b")};
    const CorrectedTexts done = correctedBy(records, {"a"});

    // The last one works, and the others are named — once, for every text.
    CHECK(done.texts == std::vector<std::string>{"b"});
    REQUIRE(done.failures.size() == 4);
    CHECK(done.failures[0].kind == FailureKind::Untranslatable);
    CHECK(done.failures[1].kind == FailureKind::CompileError);
    CHECK(done.failures[2].kind == FailureKind::InvalidReplacement);
    CHECK(done.failures[3].kind == FailureKind::InvalidReplacement);
    for (const auto& failure : done.failures) {
        CHECK_FALSE(failure.text.has_value());
        CHECK(failure.name == "A record of the test");
        CHECK_FALSE(failure.detail.empty());
    }
}

TEST_CASE("a pattern that backtracks without end is given up on that text only",
          "[text][pattern][engine]") {
    const std::string catastrophic(40, 'a');
    const std::vector<CorrectionPattern> records{recordOf("(a+)+$", "x"), recordOf("b", "c")};
    const CorrectedTexts done = correctedBy(records, {catastrophic + "b", "b"});

    // The first text is given up by the first pattern, and the second pattern
    // still applies to it; the second text is not affected.
    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TimedOut);
    CHECK(done.failures.front().text == std::optional<std::size_t>{0});
    CHECK(done.texts == std::vector<std::string>{catastrophic + "c", "c"});
}

TEST_CASE("a pattern that keeps growing the text is stopped by its size",
          "[text][pattern][engine]") {
    // Doubling in a single pass, since `replaceAll` replaces every match it
    // finds before returning: `a` → `aa` → `aaaa` → … outgrows a subtitle in a
    // handful of passes, well short of `kMaxRepeatPasses`.
    const std::vector<CorrectionPattern> growing{recordOf("a", "aa", true)};
    const CorrectedTexts done = correctedBy(growing, {"a", "b"});

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TooLong);
    CHECK(done.failures.front().text == std::optional<std::size_t>{0});
    // What it left is what it was given, not a half-grown text.
    CHECK(done.texts == std::vector<std::string>{"a", "b"});
}

TEST_CASE("a pattern that never settles but grows one character a pass is stopped by its passes",
          "[text][pattern][engine]") {
    // `\A` anchors to the absolute start of the text, so each pass finds and
    // replaces exactly one match — the text grows by one character a pass,
    // nowhere near `kMaxTextBytes` after a hundred of them. This is the
    // failure `TooManyPasses` exists for, distinct from the one above.
    const std::vector<CorrectionPattern> anchored{recordOf(R"(\Aa)", "aa", true)};
    const CorrectedTexts done = correctedBy(anchored, {"a"});

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TooManyPasses);
    // Left as it was given, not partway grown.
    CHECK(done.texts == std::vector<std::string>{"a"});
}

TEST_CASE("a replacement that matches itself again ends when the text stops changing",
          "[text][pattern][engine]") {
    // Gaupol repeats while a pass finds something, and never returns here.
    const std::vector<CorrectionPattern> records{recordOf("a", "a", true)};
    const CorrectedTexts done = correctedBy(records, {"aaa"});

    CHECK(done.texts == std::vector<std::string>{"aaa"});
    CHECK(done.failures.empty());
}

TEST_CASE("a named group in the replacement is read by its name", "[text][pattern][engine]") {
    // None of Gaupol's shipped replacements name a group — every one of them
    // uses `\1`-`\99`; `\g<name>` is exercised only here, on a record written
    // for it, which is what makes `PatternMatcher::groupNumber` reachable.
    const std::vector<CorrectionPattern> records{recordOf("(?P<word>a)", R"([\g<word>])")};
    const CorrectedTexts done = correctedBy(records, {"a"});

    CHECK(done.failures.empty());
    CHECK(done.texts == std::vector<std::string>{"[a]"});
}

TEST_CASE("a text is given back in its place whatever the patterns did",
          "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> records{recordOf("a", "b")};
    const CorrectedTexts done = correctedBy(records, {"", "aaa", "xyz"});

    CHECK(done.texts == std::vector<std::string>{"", "bbb", "xyz"});
    CHECK(correctedBy({}, {"a"}).texts == std::vector<std::string>{"a"});
}
