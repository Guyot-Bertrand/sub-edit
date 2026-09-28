#pragma once

#include <subedit/core/config/correction_settings.hpp>

#include <QWizard>

/// **Declared rather than included, deliberately** — the same reason and the
/// same shape as `correction_target_page.hpp`/`correction_progress_page.hpp`:
/// `moc` parses this header, and it chokes on the C++20 library headers
/// `correction_run.hpp` drags in through `Selection`/`SubtitleIndex`
/// (`<iterator>` pulls `<concepts>`). Declaring what the constructor needs
/// keeps `moc` out of all that; the definitions come in the implementation
/// file.
namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

class CapitalizationPage;
class CommonErrorsPage;
class CorrectionConfirmationPage;
class CorrectionProgressPage;
class CorrectionTargetPage;
class LineBreakPage;
class MentionsPage;

/// `Tools ▸ Correct Texts…` — D8, Gaupol's own order of pages, a task's page
/// shown only when it is checked on the target page.
class CorrectionWizard final : public QWizard {
    Q_OBJECT

public:
    enum PageId {
        TargetId,
        MentionsId,
        CommonErrorsId,
        CapitalizationId,
        LineBreakId,
        ProgressId,
        ConfirmationId,
    };

    CorrectionWizard(const core::PatternCatalogue& catalogue,
                     const core::CorrectionSettings& settings,
                     bool selectionAvailable,
                     bool translationAvailable,
                     QWidget* parent = nullptr);

    [[nodiscard]] int nextId() const override;

    [[nodiscard]] CorrectionTargetPage& targetPage() const { return *m_target; }

    [[nodiscard]] MentionsPage& mentionsPage() const { return *m_mentions; }

    [[nodiscard]] CommonErrorsPage& commonErrorsPage() const { return *m_commonErrors; }

    [[nodiscard]] CapitalizationPage& capitalizationPage() const { return *m_capitalization; }

    [[nodiscard]] LineBreakPage& lineBreakPage() const { return *m_lineBreak; }

    [[nodiscard]] CorrectionProgressPage& progressPage() const { return *m_progress; }

    [[nodiscard]] CorrectionConfirmationPage& confirmationPage() const { return *m_confirmation; }

private:
    CorrectionTargetPage* m_target;
    MentionsPage* m_mentions;
    CommonErrorsPage* m_commonErrors;
    CapitalizationPage* m_capitalization;
    LineBreakPage* m_lineBreak;
    CorrectionProgressPage* m_progress;
    CorrectionConfirmationPage* m_confirmation;
};

} // namespace subedit::gui
