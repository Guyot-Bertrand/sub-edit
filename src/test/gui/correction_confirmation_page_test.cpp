#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QLabel>
#include <QSignalSpy>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::FailureKind;
using subedit::core::PatternFailure;
using subedit::core::PatternKind;
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionConfirmationPage;
using subedit::gui::CorrectionProgressPage;
} // namespace

TEST_CASE("the page builds its table from the progress page's result",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(
            ProposedCorrection{.project = nullptr,
                               .index = subedit::core::SubtitleIndex::fromValue(0),
                               .document = subedit::core::Document::Main,
                               .original = "a",
                               .proposed = std::string{"b"}});
        return result;
    });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    REQUIRE(page.resultModel() != nullptr);
    CHECK(page.resultModel()->rowCount() == 1);
}

TEST_CASE("a named pattern failure is shown when the run left one behind",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.failures.push_back(PatternFailure{.kind = FailureKind::CompileError,
                                                 .code = "Zyyy",
                                                 .rank = 1,
                                                 .name = "Broken",
                                                 .text = std::nullopt,
                                                 .detail = ""});
        return result;
    });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    CHECK(page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))
              ->text()
              .contains(QStringLiteral("Broken")));
}

TEST_CASE("the remove-blank-subtitles box opens on the default it is given",
          "[gui][correction-confirmation-page]") {
    CorrectionConfirmationPage page;
    page.setRemoveBlankSubtitlesDefault(false);

    CHECK_FALSE(page.removeBlankSubtitles());
}
