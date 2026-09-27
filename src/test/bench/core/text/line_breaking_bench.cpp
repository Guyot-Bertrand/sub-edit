// What breaking lines costs on a full-length file, cached and not — issue
// #502, decision D5 of the spec of phase 12.
//
// Two benchmarks, same input and same patterns, one difference: whether
// `CharacterLineMeasure` is wrapped in `CachedLineMeasure`. **Measured, and
// the cache shows no benefit here** — counting code points is already cheap
// enough that a hash lookup costs about what recomputing does. D5 asked for
// the two numbers, not a win; the cache is what `gui`'s em measure will need,
// a font query behind `QFontMetricsF` being nowhere as cheap, and this file
// is where that comparison will be worth reading again.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_breaking.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <full_length_project.hpp>
#include <string>
#include <vector>

namespace {

using subedit::core::Project;

struct Setup {
    subedit::core::PatternCatalogue catalogue;
    std::vector<std::string> texts;
};

Setup setupFor(const Project& project) {
    const subedit::core::RealFileSystem files;
    Setup setup{.catalogue = subedit::core::readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {}),
                .texts = {}};
    for (const subedit::core::Subtitle& one : project.subtitles())
        setup.texts.push_back(one.mainText);
    return setup;
}

std::vector<const subedit::core::CorrectionPattern*> activeFor(const Setup& setup) {
    std::vector<const subedit::core::CorrectionPattern*> chosen;
    for (const subedit::core::CorrectionPattern* one :
         setup.catalogue.cascade(subedit::core::PatternKind::LineBreak, "Latn-en")) {
        if (one->enabled)
            chosen.push_back(one);
    }
    return chosen;
}

} // namespace

TEST_CASE("breaking the lines of a full-length file, in characters", "[bench][text]") {
    const Project project = subedit::test::fullLengthProject();
    const Setup setup = setupFor(project);
    const std::vector<const subedit::core::CorrectionPattern*> patterns = activeFor(setup);
    const subedit::core::IcuPatternEngine engine;
    constexpr double kMaxLength = 40.0;
    constexpr int kMaxLines = 2;

    BENCHMARK("découpage de 4000 sous-titres, sans cache de longueurs") {
        const subedit::core::CharacterLineMeasure measure;
        return subedit::core::breakLines(
            engine, patterns, setup.texts, measure, kMaxLength, kMaxLines);
    };

    BENCHMARK("découpage de 4000 sous-titres, avec cache de longueurs") {
        const subedit::core::CharacterLineMeasure characters;
        const subedit::core::CachedLineMeasure cached{characters};
        return subedit::core::breakLines(
            engine, patterns, setup.texts, cached, kMaxLength, kMaxLines);
    };
}
