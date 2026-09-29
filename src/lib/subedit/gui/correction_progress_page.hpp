#pragma once

#include <QFutureWatcher>
#include <QWizardPage>

#include <functional>
#include <memory>

/// **Declared rather than included** — the shape `subtitle_table_model.hpp`
/// adopted after `moc` once failed on `<concepts>` reached through
/// `Selection`. That failure is **not a standing rule**: `main_window.hpp`,
/// itself `Q_OBJECT`, includes `selection.hpp`, and today's `moc` parses
/// `correction_run.hpp` cleanly with this project's include paths (checked
/// for #505's final review). A declaration is kept because it is all this
/// header needs, and it keeps `moc` and every includer out of the core headers.
///
/// A forward declaration is enough even though this page stores the type: a
/// value member would need it complete, but `std::function<CorrectionProposal()>`
/// type-erases its target and never needs `CorrectionProposal` complete just to
/// be named, and naming `QFutureWatcher<CorrectionProposal>` as the argument of
/// a `unique_ptr` does not instantiate the template — only using the pointee
/// would.
namespace subedit::core {
struct CorrectionProposal;
} // namespace subedit::core

namespace subedit::gui {

/// Runs a correction computation on a background thread while the wizard
/// shows progress — D8. Abandoning is the wizard's own Cancel button: nothing
/// here interrupts the computation, but **the page is never destroyed while it
/// runs** — its destructor waits for it, so whatever the computation
/// references need only outlive the page, never the thread.
class CorrectionProgressPage final : public QWizardPage {
    Q_OBJECT

public:
    explicit CorrectionProgressPage(QWidget* parent = nullptr);
    ~CorrectionProgressPage() override;

    CorrectionProgressPage(const CorrectionProgressPage&) = delete;
    CorrectionProgressPage& operator=(const CorrectionProgressPage&) = delete;
    CorrectionProgressPage(CorrectionProgressPage&&) = delete;
    CorrectionProgressPage& operator=(CorrectionProgressPage&&) = delete;

    /// What to run once the page is shown — set before the wizard opens.
    void setComputation(std::function<core::CorrectionProposal()> compute);

    void initializePage() override;
    /// Back: the computation still running is no longer acted on — it neither
    /// stores its result nor advances the wizard once it finishes.
    void cleanupPage() override;
    [[nodiscard]] bool isComplete() const override;

    /// What the computation answered — empty until `isComplete()`.
    [[nodiscard]] const core::CorrectionProposal& result() const;

signals:
    /// Emitted synchronously, on the GUI thread, at the very start of
    /// `initializePage()` — the moment for a listener to read the wizard's
    /// other pages and call `setComputation` before the background thread
    /// starts.
    void aboutToCompute();

private:
    std::function<core::CorrectionProposal()> m_compute;
    std::unique_ptr<QFutureWatcher<core::CorrectionProposal>> m_watcher;
    std::unique_ptr<core::CorrectionProposal> m_result;
    bool m_done = false;
    /// Whether the page is the one shown — between `initializePage` and
    /// `cleanupPage`. A computation finishing outside that window is ignored.
    bool m_active = false;
};

} // namespace subedit::gui
