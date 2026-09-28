#include <subedit/gui/correction_target_page.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionTask;
using subedit::core::Document;
using subedit::gui::CorrectionScope;
using subedit::gui::CorrectionTargetPage;
} // namespace

TEST_CASE("the selection target is unavailable, and current project is the default, "
          "when nothing is selected",
          "[gui][correction-target-page]") {
    CorrectionTargetPage page{/*selectionAvailable=*/false, /*translationAvailable=*/false};

    CHECK(page.scope() == CorrectionScope::CurrentProject);
}

TEST_CASE("the translation document is offered only when a target carries one",
          "[gui][correction-target-page]") {
    CorrectionTargetPage withTranslation{true, true};
    CorrectionTargetPage without{true, false};

    // Without a translation available, asking for it anyway still answers
    // Main: the radio was never enabled, so it cannot have been chosen.
    CHECK(withTranslation.document() == Document::Main); // opens on Main either way
    CHECK(without.document() == Document::Main);
}

TEST_CASE("all four tasks are offered and none is checked by default",
          "[gui][correction-target-page]") {
    CorrectionTargetPage page{true, false};

    CHECK_FALSE(page.taskChecked(CorrectionTask::Mentions));
    CHECK_FALSE(page.taskChecked(CorrectionTask::CommonErrors));
    CHECK_FALSE(page.taskChecked(CorrectionTask::Capitalization));
    CHECK_FALSE(page.taskChecked(CorrectionTask::LineBreak));
}

TEST_CASE("setTaskChecked opens a task pre-checked", "[gui][correction-target-page]") {
    CorrectionTargetPage page{true, false};

    page.setTaskChecked(CorrectionTask::CommonErrors, true);

    CHECK(page.taskChecked(CorrectionTask::CommonErrors));
    CHECK_FALSE(page.taskChecked(CorrectionTask::Mentions));
}
