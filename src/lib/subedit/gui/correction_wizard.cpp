#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>

#include <algorithm>
#include <array>

namespace subedit::gui {

namespace {
constexpr std::array<int, 4> kTaskPages{CorrectionWizard::MentionsId,
                                        CorrectionWizard::CommonErrorsId,
                                        CorrectionWizard::CapitalizationId,
                                        CorrectionWizard::LineBreakId};
constexpr std::array<core::CorrectionTask, 4> kTasks{core::CorrectionTask::Mentions,
                                                     core::CorrectionTask::CommonErrors,
                                                     core::CorrectionTask::Capitalization,
                                                     core::CorrectionTask::LineBreak};
} // namespace

CorrectionWizard::CorrectionWizard(const core::PatternCatalogue& catalogue,
                                   const core::CorrectionSettings& settings,
                                   bool selectionAvailable,
                                   bool translationAvailable,
                                   QWidget* parent)
    : QWizard(parent),
      m_target(new CorrectionTargetPage{selectionAvailable, translationAvailable}),
      m_mentions(new MentionsPage{catalogue}),
      m_commonErrors(new CommonErrorsPage{catalogue}),
      m_capitalization(new CapitalizationPage{catalogue}),
      m_lineBreak(new LineBreakPage{catalogue}),
      m_progress(new CorrectionProgressPage{}),
      m_confirmation(new CorrectionConfirmationPage{}) {
    setWindowTitle(QStringLiteral("Correct Texts"));
    setPage(TargetId, m_target);
    setPage(MentionsId, m_mentions);
    setPage(CommonErrorsId, m_commonErrors);
    setPage(CapitalizationId, m_capitalization);
    setPage(LineBreakId, m_lineBreak);
    setPage(ProgressId, m_progress);
    setPage(ConfirmationId, m_confirmation);
    setStartId(TargetId);

    // `CorrectionSettings` has no array of `TaskSettings` to index by task —
    // this picks the right field for each of the four tasks, Gaupol's order.
    const auto taskSettingsOf =
        [&settings](core::CorrectionTask task) -> const core::TaskSettings& {
        switch (task) {
        case core::CorrectionTask::Mentions:
            return settings.mentions;
        case core::CorrectionTask::CommonErrors:
            return settings.commonErrors;
        case core::CorrectionTask::Capitalization:
            return settings.capitalization;
        case core::CorrectionTask::LineBreak:
            return settings.lineBreak;
        }
        return settings.mentions; // unreachable
    };
    for (const core::CorrectionTask task : kTasks)
        m_target->setTaskChecked(task, taskSettingsOf(task).enabled);
    m_mentions->applySettings(settings);
    m_commonErrors->applySettings(settings);
    m_capitalization->applySettings(settings);
    m_lineBreak->applySettings(settings);
    m_confirmation->setProgressPage(m_progress);
    m_confirmation->setRemoveBlankSubtitlesDefault(settings.removeBlankSubtitles);
}

int CorrectionWizard::nextId() const {
    if (currentId() == ConfirmationId)
        return -1;
    if (currentId() == ProgressId)
        return ConfirmationId;

    // TargetId or one of the four task pages: walk forward from here to the
    // next checked task, Gaupol's own order. Indices, not iterator
    // arithmetic, so there is nothing to form one before `begin()`.
    std::size_t start = 0;
    if (currentId() != TargetId) {
        const auto* const it = std::ranges::find(kTaskPages, currentId());
        start = static_cast<std::size_t>(it - kTaskPages.begin()) + 1;
    }
    for (std::size_t index = start; index < kTaskPages.size(); ++index) {
        if (m_target->taskChecked(kTasks[index]))
            return kTaskPages[index];
    }
    return ProgressId;
}

} // namespace subedit::gui
