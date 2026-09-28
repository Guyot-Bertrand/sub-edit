// What breaking lines in ems costs on a full-length file, next to the
// characters figure of #502 — issue #503, decision D5 of the spec of
// phase 12.
//
// Same patterns, same document, same `breakLines`; the only difference from
// `core/text/line_breaking_bench.cpp` is the measure — `EmsLineMeasure` wraps
// `QFontMetricsF`, which is genuinely costly next to counting code points, so
// this one always measures cached.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_breaking.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/ems_line_measure.hpp>

#include <QFont>
#include <QFontInfo>
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

TEST_CASE("breaking the lines of a full-length file, in ems", "[bench][text]") {
    // Read alongside "in characters" — same document, same patterns, this
    // file's own reason for existing.
    const QFont font{QStringLiteral("DejaVu Sans"), 10};
    if (QFontInfo{font}.family() != QStringLiteral("DejaVu Sans"))
        SKIP("la police DejaVu Sans est absente (paquet fonts-dejavu-core)");

    const Project project = subedit::test::fullLengthProject();
    const Setup setup = setupFor(project);
    const std::vector<const subedit::core::CorrectionPattern*> patterns = activeFor(setup);
    const subedit::core::IcuPatternEngine engine;
    const subedit::gui::EmsLineMeasure ems{font};
    const subedit::core::CachedLineMeasure cached{ems};
    // Gaupol's own defaults for the ems unit — D5.
    constexpr double kMaxLength = 24.0;
    constexpr int kMaxLines = 3;

    BENCHMARK("découpage de 4000 sous-titres, en ems, avec cache de longueurs") {
        return subedit::core::breakLines(
            engine, patterns, setup.texts, cached, kMaxLength, kMaxLines);
    };
}
