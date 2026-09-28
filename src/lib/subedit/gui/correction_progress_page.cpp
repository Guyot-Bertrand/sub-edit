#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_progress_page.hpp>

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWizard>
#include <QtConcurrentRun>

namespace subedit::gui {

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
        *m_result = m_watcher->result();
        m_done = true;
        emit completeChanged();
        if (wizard() != nullptr)
            wizard()->next(); // auto-advance to the confirmation page
    });
}

CorrectionProgressPage::~CorrectionProgressPage() = default;

void CorrectionProgressPage::setComputation(std::function<core::CorrectionProposal()> compute) {
    m_compute = std::move(compute);
}

void CorrectionProgressPage::initializePage() {
    m_done = false;
    *m_result = core::CorrectionProposal{};
    emit aboutToCompute(); // GUI thread: a listener may call setComputation here
    m_watcher->setFuture(QtConcurrent::run(m_compute));
}

bool CorrectionProgressPage::isComplete() const {
    return m_done;
}

const core::CorrectionProposal& CorrectionProgressPage::result() const {
    return *m_result;
}

} // namespace subedit::gui
