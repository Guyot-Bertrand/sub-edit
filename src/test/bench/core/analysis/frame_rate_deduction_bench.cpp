// What deducing the frame rate costs on a full-length file.
//
// The figure matters because of where the deduction runs: ADR 0021 puts it on
// the **read path**, so that the status bar, the conversion dialog and
// `inspect` can show it without anyone asking. Its budget is a fifth of what
// reading costs — `reading a full-length file` is the entry to compare it to,
// and the journal of measurements holds both.
//
// The candidates are eight and the pass is one, so the cost is linear in the
// number of starts and independent of what they contain: a file with a clean
// grid and a file of noise take the same time. The document is therefore the
// shared fixture, unchanged, and no second measurement on a silent file would
// say anything the first did not.
//
// Collecting the starts is measured with the deduction rather than before it.
// That is deliberate: it is what a caller has to do, and hiding it would report
// a cost nobody actually pays. Since issue #223 no caller writes that gathering
// itself — the overload on `Project` does it — so the benchmark calls what they
// call, and the figure stays the one they pay.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <full_length_project.hpp>

namespace {

using subedit::core::deduceFrameRate;
using subedit::core::Project;

} // namespace

TEST_CASE("deducing the frame rate of a full-length file", "[bench][analysis]") {
    const Project project = subedit::test::fullLengthProject();

    BENCHMARK("déduction de fréquence sur 4000 sous-titres") {
        return deduceFrameRate(project);
    };
}
