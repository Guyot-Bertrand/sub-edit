#include <subedit/core/edit/session.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_controller.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/ems_line_measure.hpp>
#include <subedit/gui/join_split_page.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>

#include <QDialog>
#include <QObject>

#include <memory>
#include <utility>

namespace subedit::gui {

namespace {

/// Reads every page of `wizard` into `previous`'s shape — the settings to
/// compute with while the wizard is open, and to persist once it finishes.
[[nodiscard]] core::CorrectionSettings settingsOf(const CorrectionWizard& wizard,
                                                  const core::CorrectionSettings& previous) {
    core::CorrectionSettings settings = previous;
    settings.mentions = {.enabled = wizard.targetPage().taskChecked(core::CorrectionTask::Mentions),
                         .code = wizard.mentionsPage().code()};
    settings.joinSplitEnabled =
        wizard.targetPage().taskChecked(core::CorrectionTask::JoinSplitWords);
    settings.joinWords = wizard.joinSplitPage().join();
    settings.splitWords = wizard.joinSplitPage().split();
    settings.spellLanguage = wizard.joinSplitPage().language();
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
    settings.lineBreakInEms = wizard.lineBreakPage().useEms();
    settings.lineBreakSkipOnLength = wizard.lineBreakPage().skipOnLength();
    settings.lineBreakSkipMaxLength = wizard.lineBreakPage().skipMaxLength();
    settings.lineBreakSkipOnLines = wizard.lineBreakPage().skipOnLines();
    settings.lineBreakSkipMaxLines = wizard.lineBreakPage().skipMaxLines();
    settings.removeBlankSubtitles = wizard.confirmationPage().removeBlankSubtitles();
    wizard.mentionsPage().mergeActivationsInto(settings.patternActivations);
    wizard.commonErrorsPage().mergeActivationsInto(settings.patternActivations);
    wizard.capitalizationPage().mergeActivationsInto(settings.patternActivations);
    wizard.lineBreakPage().mergeActivationsInto(settings.patternActivations);
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

/// The checker of `language`, with the replacements its file holds — or null
/// if the dictionary vanished between the page asking and now.
[[nodiscard]] std::shared_ptr<const core::SpellChecker>
openedSpellChecker(const core::SpellProvider& provider,
                   const std::string& language,
                   const std::filesystem::path& configDirectory) {
    const core::RealFileSystem files;
    auto opened = core::openSpellChecker(
        provider, language, files, core::spellReplacementFile(configDirectory, language));
    if (!opened.has_value())
        return nullptr;
    return std::make_shared<const core::SpellChecker>(std::move(*opened));
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

    // **Declared before the wizard, deliberately**: locals are destroyed in
    // reverse order, and the wizard's progress page waits in its destructor
    // for a computation still running on this engine — the engine must still
    // be there while it does.
    core::IcuPatternEngine engine;
    CorrectionWizard wizard{m_view->patternCatalogue(),
                            m_view->spellProvider(),
                            m_settings,
                            selectionAvailable,
                            translationAvailable,
                            m_view->dialogParent()};

    wizard.confirmationPage().setReadDiagnostics(m_view->patternCatalogue().diagnostics());

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

            // One measure, with its cache, for this computation alone: it is
            // asked from the background thread and from nowhere else (D5).
            const std::shared_ptr<const core::LineMeasure> measure =
                assistantLineMeasure(current.lineBreakInEms, m_view->applicationFont());

            // Opened here, on this thread, and only when the task runs and the
            // page found a dictionary: no dictionary is not a failure, the
            // task just does not run. The computation owns it from here.
            std::shared_ptr<const core::SpellChecker> spell;
            if (current.joinSplitEnabled && wizard.joinSplitPage().available())
                spell = openedSpellChecker(*m_view->spellProvider(),
                                           current.spellLanguage,
                                           m_view->spellConfigDirectory());

            wizard.progressPage().setComputation(
                [this, &engine, current, targets = std::move(targets), measure, spell] {
                    return core::proposeCorrections(engine,
                                                    m_view->patternCatalogue(),
                                                    current,
                                                    *measure,
                                                    targets,
                                                    spell.get());
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
