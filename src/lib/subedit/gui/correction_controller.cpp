#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_controller.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/ems_line_measure.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>

#include <QDialog>
#include <QObject>

#include <algorithm>
#include <memory>
#include <utility>

namespace subedit::gui {

namespace {

/// Merges `from` into `into` by (kind, code, name), replacing an existing
/// entry or adding a new one — never clearing a stale entry for a code this
/// wizard run never showed (a known limitation, see Task 9's own note on
/// `PatternList`).
void mergeActivations(std::vector<core::PatternActivation>& into,
                      const std::vector<core::PatternActivation>& from) {
    for (const core::PatternActivation& activation : from) {
        const auto found = std::ranges::find_if(into, [&](const core::PatternActivation& existing) {
            return existing.kind == activation.kind && existing.code == activation.code &&
                   existing.name == activation.name;
        });
        if (found != into.end())
            *found = activation;
        else
            into.push_back(activation);
    }
}

/// Reads every page of `wizard` into `previous`'s shape — the settings to
/// compute with while the wizard is open, and to persist once it finishes.
[[nodiscard]] core::CorrectionSettings settingsOf(const CorrectionWizard& wizard,
                                                  const core::CorrectionSettings& previous) {
    core::CorrectionSettings settings = previous;
    settings.mentions = {.enabled = wizard.targetPage().taskChecked(core::CorrectionTask::Mentions),
                         .code = wizard.mentionsPage().code()};
    settings.commonErrors = {
        .enabled = wizard.targetPage().taskChecked(core::CorrectionTask::CommonErrors),
        .code = wizard.commonErrorsPage().code()};
    settings.capitalization = {
        .enabled = wizard.targetPage().taskChecked(core::CorrectionTask::Capitalization),
        .code = wizard.capitalizationPage().code()};
    settings.lineBreak = {.enabled =
                              wizard.targetPage().taskChecked(core::CorrectionTask::LineBreak),
                          .code = wizard.lineBreakPage().code()};
    settings.human = wizard.commonErrorsPage().human();
    settings.ocr = wizard.commonErrorsPage().ocr();
    settings.soundInBrackets = wizard.mentionsPage().soundInBrackets();
    settings.soundInParentheses = wizard.mentionsPage().soundInParentheses();
    settings.lineBreakMaxLength = wizard.lineBreakPage().maxLength();
    settings.lineBreakMaxLines = wizard.lineBreakPage().maxLines();
    settings.removeBlankSubtitles = wizard.confirmationPage().removeBlankSubtitles();
    mergeActivations(settings.patternActivations, wizard.mentionsPage().activations());
    mergeActivations(settings.patternActivations, wizard.commonErrorsPage().activations());
    mergeActivations(settings.patternActivations, wizard.capitalizationPage().activations());
    mergeActivations(settings.patternActivations, wizard.lineBreakPage().activations());
    return settings;
}

[[nodiscard]] ProjectPage* pageOwning(std::span<const std::unique_ptr<ProjectPage>> pages,
                                      const core::Project& project) {
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (&page->session->project() == &project)
            return page.get();
    }
    return nullptr;
}

} // namespace

CorrectionController::CorrectionController(Prompts& prompts, View& view)
    : m_prompts(&prompts), m_view(&view) {}

void CorrectionController::open() {
    const std::span<const std::unique_ptr<ProjectPage>> pages = m_view->pages();
    const std::size_t shown = m_view->shownProject();

    const bool selectionAvailable = selectionOf(*pages[shown]->tableSelection).count() != 0;
    bool translationAvailable = false;
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (page->session->project().translationFile().has_value()) {
            translationAvailable = true;
            break;
        }
    }

    CorrectionWizard wizard{m_view->patternCatalogue(),
                            m_settings,
                            selectionAvailable,
                            translationAvailable,
                            m_view->dialogParent()};

    core::IcuPatternEngine engine;
    QObject::connect(
        &wizard.progressPage(),
        &CorrectionProgressPage::aboutToCompute,
        &wizard,
        [this, &wizard, &engine, pages, shown] {
            const core::CorrectionSettings current = settingsOf(wizard, m_settings);
            const CorrectionScope scope = wizard.targetPage().scope();
            const core::Document document = wizard.targetPage().document();
            std::vector<core::CorrectionTarget> targets =
                correctionTargetsOf(scope, document, pages, shown);

            const std::shared_ptr<const core::LineMeasure> measure =
                wizard.lineBreakPage().useEms()
                    ? std::static_pointer_cast<const core::LineMeasure>(
                          std::make_shared<EmsLineMeasure>(m_view->applicationFont()))
                    : std::static_pointer_cast<const core::LineMeasure>(
                          std::make_shared<core::CharacterLineMeasure>());

            wizard.progressPage().setComputation(
                [this, &engine, current, targets = std::move(targets), measure] {
                    return core::proposeCorrections(
                        engine, m_view->patternCatalogue(), current, *measure, targets);
                });
        });

    QObject::connect(&wizard.confirmationPage(),
                     &CorrectionConfirmationPage::previewRequested,
                     &wizard,
                     [this, &wizard](int row) {
                         const core::ProposedCorrection& correction =
                             wizard.confirmationPage().resultModel()->correctionAt(row);
                         m_view->preview(*correction.project, correction.index);
                     });

    if (!m_prompts->run(wizard))
        return; // cancelled: settings and every project stay as they were

    m_settings = settingsOf(wizard, m_settings);

    const std::vector<core::ProposedCorrection> accepted =
        wizard.confirmationPage().resultModel()->acceptedCorrections();
    std::vector<core::AppliedCorrection> composed =
        core::applyCorrections(accepted, wizard.confirmationPage().removeBlankSubtitles());
    const core::CorrectionTally tally =
        core::tallyOf(composed); // before the commands are moved below

    for (core::AppliedCorrection& one : composed) {
        ProjectPage* owner = pageOwning(pages, *one.project);
        if (owner == nullptr)
            continue; // its project closed while the wizard was open
        owner->model->applied(owner->session->apply(std::move(one.command)));
    }

    m_view->announce(core::noticeOfCorrection(tally.corrected, tally.removed));
}

} // namespace subedit::gui
