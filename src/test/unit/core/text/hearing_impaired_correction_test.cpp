// The hearing-impaired mentions the pattern engine plays — issue #500,
// decision D7 of the spec of phase 12.
//
// **The cases are Gaupol's, not ours.** `hearing-impaired.cas` was written by
// the oracle of #494 from what Gaupol does with its own files, before this
// engine existed; the test plays each case through the engine and the shipped
// patterns and asks for the same text. A disagreement is this code's, never
// the case's — except where the scan of ADR 0017 was chosen over Gaupol's own
// regular expression on purpose, which the PR of the issue lists case by case.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
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

using subedit::core::correctHearingImpaired;
using subedit::core::CorrectionPattern;
using subedit::core::FailureKind;
using subedit::core::HearingImpairedCorrection;
using subedit::core::HearingImpairedFields;
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
/// reading `common_errors_test.cpp` gives, for `PatternKind::HearingImpaired`.
std::vector<const CorrectionPattern*> patternsOf(std::string_view target) {
    const PatternCatalogue& catalogue = shippedPatterns();
    if (target.starts_with("cascade ")) {
        target.remove_prefix(std::string_view{"cascade "}.size());
        const bool everything = target.ends_with(" tous");
        if (everything)
            target.remove_suffix(std::string_view{" tous"}.size());
        std::vector<const CorrectionPattern*> chosen;
        for (const CorrectionPattern* one :
             catalogue.cascade(PatternKind::HearingImpaired, target)) {
            if (everything || one->enabled)
                chosen.push_back(one);
        }
        return chosen;
    }
    const std::size_t colon = target.find(':');
    const std::string_view code = target.substr(0, colon);
    const std::size_t rank = std::stoul(std::string{target.substr(colon + 1)});
    for (const CorrectionPattern& one : catalogue.patterns()) {
        if (one.kind() == PatternKind::HearingImpaired && one.code == code && one.rank == rank)
            return {&one};
    }
    return {};
}

CorrectionPattern recordOf(std::string expression, std::string replacement) {
    CorrectionPattern pattern;
    pattern.code = "Test";
    pattern.rank = 1;
    pattern.name = "A record of the test";
    pattern.expression = std::move(expression);
    pattern.flags = PatternFlags{.dotAll = true, .multiline = true};
    pattern.fields = HearingImpairedFields{.replacement = std::move(replacement)};
    return pattern;
}

HearingImpairedCorrection correctedBy(const std::vector<CorrectionPattern>& records,
                                      const std::vector<std::string>& texts) {
    std::vector<const CorrectionPattern*> chosen;
    chosen.reserve(records.size());
    for (const CorrectionPattern& one : records)
        chosen.push_back(&one);
    return correctHearingImpaired(IcuPatternEngine{}, chosen, texts, SubtitleFormat::SubRip);
}

} // namespace

TEST_CASE("hearing-impaired mentions are corrected as Gaupol corrects them",
          "[text][pattern][engine]") {
    const std::vector<TextCase> cases = textCasesOf("motifs/attendus/hearing-impaired.cas");
    REQUIRE(cases.size() > 15);
    REQUIRE(shippedPatterns().diagnostics().empty());

    // **Two cases are excepted, and the exception is written down, not
    // silent.** `hearing-impaired.entrees` names them "the two cases that
    // distinguish Gaupol's search from a `re.sub`" — a line left holding
    // nothing but a stray quote, next to a mention that only the scan of
    // ADR 0017 removes. Gaupol's own `_remove_leftover_hi` then empties that
    // line too (`^\W*$`), a global rule with no notion of where the mention
    // was; the scan keeps to the local seam ADR 0017 chose instead, and
    // leaves the quote's line alone. The PR of #500 lists them.
    const std::vector<std::string> exceptedByAdr0017{
        "Latn:2 corrige — une ligne de ponctuation seule, emportée",
        "Latn:1 corrige — la même, après un crochet",
    };

    // **One more exception, this one decision D9's.** `MarkupParser::transform`
    // keeps a tag it finds inside a match as far as the new text reaches,
    // where Gaupol's own tag-aware parser pulls it back to where the match
    // began — the rule phase 10 gave `transform`, and `recherche.cas` holds it
    // to. The PR of #501 names it.
    const std::vector<std::string> exceptedByD9{
        "Latn:5 corrige — le nom du locuteur est balisé",
    };

    for (const TextCase& one : cases) {
        const std::size_t verb =
            std::min(one.name.find(" corrige — "), one.name.find(" intact — "));
        REQUIRE(verb != std::string::npos);
        const std::string target = one.name.substr(0, verb);

        const std::vector<const CorrectionPattern*> chosen = patternsOf(target);
        REQUIRE_FALSE(chosen.empty());

        const std::vector<std::string> given{one.input};
        const HearingImpairedCorrection done =
            correctHearingImpaired(IcuPatternEngine{}, chosen, given, SubtitleFormat::SubRip);

        INFO("cas ligne " << one.line << " : " << one.name);
        CHECK(done.failures.empty());
        REQUIRE(done.texts.size() == 1);
        if (std::ranges::find(exceptedByAdr0017, one.name) != exceptedByAdr0017.end())
            continue;
        if (std::ranges::find(exceptedByD9, one.name) != exceptedByD9.end())
            continue;
        CHECK(done.texts.front() == one.expected);
    }
}

TEST_CASE("only the hearing-impaired patterns are applied", "[text][pattern][engine]") {
    CorrectionPattern other = recordOf("a", "b");
    other.fields = subedit::core::CommonErrorFields{.classes = {}, .replacement = "b"};

    CHECK(correctedBy({other}, {"a"}).texts == std::vector<std::optional<std::string>>{"a"});
}

TEST_CASE("Sound in brackets and Sound in parentheses are not compiled by the cascade",
          "[text][pattern][engine]") {
    // Giving them here would run their expression a second time, on text the
    // scan already resolved. `[laughs]` never reaches the engine, so the record
    // aimed at it below has nothing to compile away.
    std::vector<CorrectionPattern> records{recordOf(R"(\[.*?\])", "SHOULD NOT RUN")};
    records.front().name = "Sound in brackets";

    CHECK(correctedBy(records, {"[laughs] Hi"}).texts ==
          std::vector<std::optional<std::string>>{"Hi"});
}

TEST_CASE("the cascade runs once per pattern, and the clean-up only if it changed something",
          "[text][pattern][engine]") {
    // No pattern matches: the double space survives, and so does the scan's
    // own untouched output.
    CHECK(correctedBy({recordOf("xyz", "")}, {"Hello  there"}).texts ==
          std::vector<std::optional<std::string>>{"Hello  there"});
}

TEST_CASE("a hearing-impaired pattern that cannot be applied is named, and the others are",
          "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> records{recordOf("(unclosed", "b"), recordOf("a", "b")};
    const HearingImpairedCorrection done = correctedBy(records, {"a"});

    CHECK(done.texts == std::vector<std::optional<std::string>>{"b"});
    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::CompileError);
}

TEST_CASE("a named group in a hearing-impaired replacement is read by its name",
          "[text][pattern][engine]") {
    // None of the shipped records name a group — every one uses `\1` — so
    // `\g<name>` is exercised only here, the same corner `common_errors_test`
    // reaches for `correctCommonErrors`.
    const std::vector<CorrectionPattern> records{recordOf("(?P<word>a)", R"([\g<word>])")};

    CHECK(correctedBy(records, {"a"}).texts == std::vector<std::optional<std::string>>{"[a]"});
}

TEST_CASE("an invalid replacement is named, and the others still apply",
          "[text][pattern][engine]") {
    const std::vector<CorrectionPattern> records{recordOf("(a)", R"(\2)"), // there is no group 2
                                                 recordOf("a", "b")};
    const HearingImpairedCorrection done = correctedBy(records, {"a"});

    CHECK(done.texts == std::vector<std::optional<std::string>>{"b"});
    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::InvalidReplacement);
}

TEST_CASE("a hearing-impaired match with nothing in it is skipped one character at a time",
          "[text][pattern][engine]") {
    // `x*` matches the empty string everywhere, and each match is replaced:
    // the guard against finding the same place for ever is `Finder`'s. `_`
    // and not `-`: a leading dash is what the clean-up's own dialogue-dash
    // rules look for, which this case has nothing to do with.
    const std::vector<CorrectionPattern> records{recordOf("x*", "_")};

    CHECK(correctedBy(records, {"ab"}).texts == std::vector<std::optional<std::string>>{"_a_b_"});
}

TEST_CASE("a hearing-impaired pattern that backtracks without end is given up on that text",
          "[text][pattern][engine]") {
    // `$` anchors the end: appending `b` is what forces the backtracking, as
    // in `common_errors_test.cpp` — forty `a` alone matches on the first
    // attempt and gives up nothing to time out on.
    const std::string catastrophic(40, 'a');
    const std::vector<CorrectionPattern> records{recordOf("(a+)+$", "x")};
    const HearingImpairedCorrection done = correctedBy(records, {catastrophic + "b"});

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TimedOut);
    CHECK(done.texts == std::vector<std::optional<std::string>>{catastrophic + "b"});
}

TEST_CASE("a hearing-impaired pattern that keeps growing the text is stopped by its size",
          "[text][pattern][engine]") {
    // A single pass never repeats, so growing without bound takes many
    // matches rather than many passes: two hundred single-letter matches,
    // each replaced by a hundred characters, outgrow a subtitle well inside
    // the one pass `replaceOnce` makes.
    const std::string many(200, 'a');
    const std::vector<CorrectionPattern> records{recordOf("a", std::string(100, 'x'))};
    const HearingImpairedCorrection done = correctedBy(records, {many});

    REQUIRE(done.failures.size() == 1);
    CHECK(done.failures.front().kind == FailureKind::TooLong);
    // What it left is what it was given, not a half-grown text.
    CHECK(done.texts == std::vector<std::optional<std::string>>{many});
}
