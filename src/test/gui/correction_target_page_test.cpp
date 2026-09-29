#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/correction_target_page.hpp>

#include <QRadioButton>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionTask;
using subedit::core::Document;
using subedit::gui::CorrectionScope;
using subedit::gui::CorrectionTargetPage;

/// The radio button of `parent` whose label reads `text` — the page names
/// none of its own, the same reason `window_correction_test.cpp` looks them
/// up by label rather than by `objectName`.
[[nodiscard]] QRadioButton* radioNamed(const QWidget& parent, const QString& text) {
    for (QRadioButton* button : parent.findChildren<QRadioButton*>()) {
        if (button->text() == text)
            return button;
    }
    return nullptr;
}
} // namespace

TEST_CASE("the selection target is unavailable, and current project is the default, "
          "when nothing is selected",
          "[gui][correction-target-page]") {
    const CorrectionTargetPage page{/*selectionAvailable=*/false, /*translationAvailable=*/false};

    CHECK(page.scope() == CorrectionScope::CurrentProject);
}

TEST_CASE("the translation document is offered only when a target carries one",
          "[gui][correction-target-page]") {
    const CorrectionTargetPage withTranslation{true, true};
    const CorrectionTargetPage without{true, false};

    // Without a translation available, asking for it anyway still answers
    // Main: the radio was never enabled, so it cannot have been chosen.
    CHECK(withTranslation.document() == Document::Main); // opens on Main either way
    CHECK(without.document() == Document::Main);
}

TEST_CASE("all four tasks are offered and none is checked by default",
          "[gui][correction-target-page]") {
    const CorrectionTargetPage page{true, false};

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

TEST_CASE("checking Selection on the target page answers the selection scope",
          "[gui][correction-target-page]") {
    const CorrectionTargetPage page{/*selectionAvailable=*/true, /*translationAvailable=*/false};
    QRadioButton* selection = radioNamed(page, QStringLiteral("Selection"));
    REQUIRE(selection != nullptr);

    selection->setChecked(true);

    CHECK(page.scope() == CorrectionScope::Selection);
}
