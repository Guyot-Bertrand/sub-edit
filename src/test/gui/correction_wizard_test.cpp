#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_wizard.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::CorrectionTask;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::readPatternCatalogue;
using subedit::gui::CorrectionWizard;

PatternCatalogue emptyCatalogue() {
    const InMemoryFileSystem files;
    return readPatternCatalogue(files, "/patterns", {});
}

/// `CorrectionSettings{}` is not "nothing checked" — `commonErrors` and
/// `capitalization` default to enabled (Gaupol's own defaults). Both cases
/// below need every task starting unchecked, so they build their own.
CorrectionSettings noTaskChecked() {
    CorrectionSettings settings;
    settings.mentions.enabled = false;
    settings.commonErrors.enabled = false;
    settings.capitalization.enabled = false;
    settings.lineBreak.enabled = false;
    return settings;
}
} // namespace

TEST_CASE("advancing past the target page goes straight to progress when no task is checked",
          "[gui][correction-wizard]") {
    const PatternCatalogue catalogue = emptyCatalogue();
    CorrectionWizard wizard{catalogue,
                            /*spellProvider=*/nullptr,
                            noTaskChecked(),
                            /*selectionAvailable=*/true,
                            /*translationAvailable=*/false};
    wizard.show();
    REQUIRE(wizard.currentId() == CorrectionWizard::TargetId);

    wizard.next();

    CHECK(wizard.currentId() == CorrectionWizard::ProgressId);
}

TEST_CASE("advancing past the target page visits only the checked tasks, in order",
          "[gui][correction-wizard]") {
    const PatternCatalogue catalogue = emptyCatalogue();
    CorrectionWizard wizard{catalogue, nullptr, noTaskChecked(), true, false};
    wizard.show();
    wizard.targetPage().setTaskChecked(CorrectionTask::LineBreak, true);
    wizard.targetPage().setTaskChecked(CorrectionTask::Mentions, true);

    wizard.next();
    CHECK(wizard.currentId() ==
          CorrectionWizard::MentionsId); // Gaupol's order, not the check order

    wizard.next();
    CHECK(wizard.currentId() == CorrectionWizard::LineBreakId);

    wizard.next();
    CHECK(wizard.currentId() == CorrectionWizard::ProgressId);
}
