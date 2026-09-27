// What the correction of common errors costs on a full-length file — phase 12.
//
// The patterns are the ones Gaupol activates by default for a language, applied
// to the text of the four thousand subtitles of `full_length_project.hpp`:
// the engine, the translation of the syntax, the replacements and `Repeat`, and
// nothing of the window. ADR 0036 measured ICU at about a tenth of a second per
// film on a private corpus; this figure is what the journal compares from now
// on, so that a change to the translation, the template or the loop that costs
// something is seen when it is made.
//
// **The patterns are compiled inside the measured region**, as they are each
// time the assistant runs: a benchmark that compiled them outside would measure
// the loop and hide what a user waits for.
//
// Two languages, because the document is French and its expectations are
// English: `Latn-fr` is the run a user of this document does, `Latn-en` is the
// one the ADR measured, and they do not apply the same records.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <full_length_project.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::Project;

/// The patterns Gaupol activates for `code`, and the texts of the project.
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

std::vector<const subedit::core::CorrectionPattern*> activeFor(const Setup& setup,
                                                               std::string_view code) {
    std::vector<const subedit::core::CorrectionPattern*> chosen;
    for (const subedit::core::CorrectionPattern* one :
         setup.catalogue.cascade(subedit::core::PatternKind::CommonError, code)) {
        if (one->enabled)
            chosen.push_back(one);
    }
    return chosen;
}

} // namespace

TEST_CASE("correcting the common errors of a full-length file", "[bench][text]") {
    const Project project = subedit::test::fullLengthProject();
    const Setup setup = setupFor(project);
    const std::vector<const subedit::core::CorrectionPattern*> french = activeFor(setup, "Latn-fr");
    const std::vector<const subedit::core::CorrectionPattern*> english =
        activeFor(setup, "Latn-en");
    const subedit::core::IcuPatternEngine engine;

    BENCHMARK("erreurs courantes du français sur 4000 sous-titres") {
        return subedit::core::correctCommonErrors(engine, french, setup.texts);
    };

    BENCHMARK("erreurs courantes de l'anglais sur 4000 sous-titres") {
        return subedit::core::correctCommonErrors(engine, english, setup.texts);
    };
}
