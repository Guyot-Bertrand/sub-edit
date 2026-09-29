#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_progress_page.hpp>

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWizard>
#include <QtConcurrentRun>

namespace subedit::gui {

namespace {

/// Waits for whatever `watcher` is running, if anything.
///
/// `waitForFinished` rethrows what the computation threw. Both callers wait
/// on a run whose result nobody reads any more — one abandoned by Back, or
/// the page being destroyed — so the exception is dropped, exactly as it was
/// before these waits existed; a destructor must not throw anyway.
void settle(QFutureWatcher<core::CorrectionProposal>& watcher) noexcept {
    try {
        watcher.waitForFinished();
    } catch (...) { // NOLINT(bugprone-empty-catch): dropped on purpose, see above
    }
}

} // namespace

CorrectionProgressPage::CorrectionProgressPage(QWidget* parent)
    : QWizardPage(parent),
      m_watcher(std::make_unique<QFutureWatcher<core::CorrectionProposal>>()),
      m_result(std::make_unique<core::CorrectionProposal>()) {
    setTitle(QStringLiteral("Progress"));

    auto* bar = new QProgressBar{this};
    bar->setRange(0, 0); // indeterminate: proposeCorrections reports no progress of its own

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(new QLabel{QStringLiteral("Computing the proposed corrections…"), this});
    layout->addWidget(bar);

    connect(m_watcher.get(), &QFutureWatcher<core::CorrectionProposal>::finished, this, [this] {
        // Left by Back while it ran: its answer is no longer asked for. And a
        // `finished` still queued from a run abandoned that way can arrive
        // after Next started a fresh one — the watcher's *current* future is
        // the only one whose end counts.
        if (!m_active || !m_watcher->isFinished())
            return;
        *m_result = m_watcher->result();
        m_done = true;
        emit completeChanged();
        if (wizard() != nullptr)
            wizard()->next(); // auto-advance to the confirmation page
    });
}

CorrectionProgressPage::~CorrectionProgressPage() {
    // The computation references what its caller owns — the engine, the
    // catalogue, the projects of its targets — and the caller's own rule is
    // that those outlive this page, not the background thread. Waiting here
    // is what makes that rule enough: a Cancel pressed mid-computation
    // blocks for the rest of it, bounded by ICU's own time limit, instead of
    // leaving the thread working on objects destroyed under it. Never-started
    // is fine too: a watcher with no future set waits on nothing.
    settle(*m_watcher);
}

void CorrectionProgressPage::setComputation(std::function<core::CorrectionProposal()> compute) {
    m_compute = std::move(compute);
}

void CorrectionProgressPage::initializePage() {
    // Back then Next again before the previous run finished: let it finish
    // first. `setFuture` below would forget it, and the destructor only waits
    // on the future the watcher holds — two runs in flight would leave one
    // unwaited for.
    settle(*m_watcher);
    m_done = false;
    m_active = true;
    *m_result = core::CorrectionProposal{};
    emit aboutToCompute(); // GUI thread: a listener may call setComputation here
    // No computation set is an empty proposal, never a `bad_function_call`
    // stored in the future for the destructor's wait to rethrow.
    m_watcher->setFuture(QtConcurrent::run(
        [compute = m_compute] { return compute ? compute() : core::CorrectionProposal{}; }));
}

void CorrectionProgressPage::cleanupPage() {
    // Back, while the computation may still run: it is left to finish on its
    // own — Back stays immediate — but its `finished` no longer stores a
    // result or advances the wizard from wherever the user now is.
    m_active = false;
    m_done = false;
    QWizardPage::cleanupPage();
}

bool CorrectionProgressPage::isComplete() const {
    return m_done;
}

const core::CorrectionProposal& CorrectionProgressPage::result() const {
    return *m_result;
}

} // namespace subedit::gui
