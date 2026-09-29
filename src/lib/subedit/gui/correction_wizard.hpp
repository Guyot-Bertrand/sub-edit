#pragma once

#include <subedit/core/config/correction_settings.hpp>

#include <QWizard>

/// **Declared rather than included** — the shape `subtitle_table_model.hpp`
/// adopted after `moc` once failed on `<concepts>` reached through
/// `Selection`. That failure is **not a standing rule**: `main_window.hpp`,
/// itself `Q_OBJECT`, includes `selection.hpp`, and today's `moc` parses
/// `correction_run.hpp` cleanly with this project's include paths (checked
/// for #505's final review). A declaration is kept because it is all this
/// header needs, and it keeps `moc` and every includer out of the core headers.
namespace subedit::core {
class PatternCatalogue;
class SpellProvider;
} // namespace subedit::core

namespace subedit::gui {

class CapitalizationPage;
class CommonErrorsPage;
class CorrectionConfirmationPage;
class CorrectionProgressPage;
class CorrectionTargetPage;
class JoinSplitPage;
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
        JoinSplitId,
        CommonErrorsId,
        CapitalizationId,
        LineBreakId,
        ProgressId,
        ConfirmationId,
    };

    /// `spellProvider` may be null: no spell-checking, and the join-and-split
    /// page says there is no dictionary.
    CorrectionWizard(const core::PatternCatalogue& catalogue,
                     const core::SpellProvider* spellProvider,
                     const core::CorrectionSettings& settings,
                     bool selectionAvailable,
                     bool translationAvailable,
                     QWidget* parent = nullptr);

    [[nodiscard]] int nextId() const override;

    [[nodiscard]] CorrectionTargetPage& targetPage() const { return *m_target; }

    [[nodiscard]] MentionsPage& mentionsPage() const { return *m_mentions; }

    [[nodiscard]] JoinSplitPage& joinSplitPage() const { return *m_joinSplit; }

    [[nodiscard]] CommonErrorsPage& commonErrorsPage() const { return *m_commonErrors; }

    [[nodiscard]] CapitalizationPage& capitalizationPage() const { return *m_capitalization; }

    [[nodiscard]] LineBreakPage& lineBreakPage() const { return *m_lineBreak; }

    [[nodiscard]] CorrectionProgressPage& progressPage() const { return *m_progress; }

    [[nodiscard]] CorrectionConfirmationPage& confirmationPage() const { return *m_confirmation; }

private:
    CorrectionTargetPage* m_target;
    MentionsPage* m_mentions;
    JoinSplitPage* m_joinSplit;
    CommonErrorsPage* m_commonErrors;
    CapitalizationPage* m_capitalization;
    LineBreakPage* m_lineBreak;
    CorrectionProgressPage* m_progress;
    CorrectionConfirmationPage* m_confirmation;
};

} // namespace subedit::gui
