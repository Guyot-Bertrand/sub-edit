#pragma once

#include <QFutureWatcher>
#include <QWizardPage>

#include <functional>
#include <memory>

/// **Declared rather than included, deliberately** — the same reason and the
/// same shape as `subtitle_table_model.hpp`: `moc` parses this header, and it
/// chokes on the C++20 library headers `correction_run.hpp` drags in through
/// `Selection`/`SubtitleIndex` (`<iterator>` pulls `<concepts>`). Declaring
/// what the members need keeps `moc` out of all that; the definitions come in
/// the implementation file.
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
/// here interrupts the computation, it is simply never read.
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
};

} // namespace subedit::gui
