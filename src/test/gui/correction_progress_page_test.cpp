#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_progress_page.hpp>

#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionProgressPage;
} // namespace

TEST_CASE("the page is not complete until its background computation finishes",
          "[gui][correction-progress-page]") {
    CorrectionProgressPage page;
    page.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(
            ProposedCorrection{.project = nullptr,
                               .index = subedit::core::SubtitleIndex::fromValue(0),
                               .document = subedit::core::Document::Main,
                               .original = "original",
                               .proposed = "proposed"});
        return result;
    });
    CHECK_FALSE(page.isComplete());

    QSignalSpy spy{&page, &CorrectionProgressPage::completeChanged};
    page.initializePage();
    REQUIRE(spy.wait(2000));

    CHECK(page.isComplete());
    CHECK(page.result().corrections.size() == 1);
}
