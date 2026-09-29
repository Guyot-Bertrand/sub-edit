#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_wizard.hpp>

#include <QSignalSpy>
#include <QTest>
#include <QThreadPool>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::ProposedCorrection;
using subedit::core::readPatternCatalogue;
using subedit::gui::CorrectionProgressPage;
using subedit::gui::CorrectionWizard;
} // namespace

TEST_CASE("the page is not complete until its background computation finishes",
          "[gui][correction-progress-page]") {
    CorrectionProgressPage page;
    page.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(
            ProposedCorrection{.project = nullptr,
                               .index = subedit::core::SubtitleIndex::fromValue(0),
                               .document = subedit::core::Document::Main,
                               .original = "original",
                               .proposed = "proposed"});
        return result;
    });
    CHECK_FALSE(page.isComplete());

    QSignalSpy spy{&page, &CorrectionProgressPage::completeChanged};
    page.initializePage();
    REQUIRE(spy.wait(2000));

    CHECK(page.isComplete());
    CHECK(page.result().corrections.size() == 1);
}

// Issue #505's final review: the page must never be destroyed while its
// computation runs (Cancel), and must not act on one abandoned by Back.

namespace {

using namespace std::chrono_literals;

/// A computation that says when it started, sleeps `delay`, and says when it
/// ended — slow enough that a test can act while it still runs.
struct SlowComputation {
    std::atomic<bool> started{false};
    std::atomic<bool> ended{false};
    std::chrono::milliseconds delay = 300ms;

    [[nodiscard]] std::function<CorrectionProposal()> function() {
        return [this] {
            started = true;
            std::this_thread::sleep_for(delay);
            ended = true;
            return CorrectionProposal{};
        };
    }

    void awaitStart() const {
        while (!started)
            std::this_thread::yield();
    }
};

[[nodiscard]] PatternCatalogue emptyCatalogue() {
    const InMemoryFileSystem files;
    return readPatternCatalogue(files, "/patterns", {});
}

} // namespace

TEST_CASE("destroying the page mid-computation waits for the computation to end",
          "[gui][correction-progress-page]") {
    SlowComputation slow;
    auto page = std::make_unique<CorrectionProgressPage>();
    page->setComputation(slow.function());
    page->initializePage();
    slow.awaitStart();
    REQUIRE_FALSE(slow.ended);

    const auto before = std::chrono::steady_clock::now();
    page.reset(); // what Cancel does to the wizard, and so to this page
    const auto waited = std::chrono::steady_clock::now() - before;

    // The computation had ended by the time the destructor returned: nothing
    // it references could have been destroyed under it.
    CHECK(slow.ended);
    CHECK(waited >= 100ms);
    QThreadPool::globalInstance()->waitForDone(); // `slow` outlives the thread, pass or fail
}

TEST_CASE("destroying a page whose computation never started returns at once",
          "[gui][correction-progress-page]") {
    SlowComputation slow;
    const auto before = std::chrono::steady_clock::now();
    {
        CorrectionProgressPage page;
        page.setComputation(slow.function());
    }
    CHECK(std::chrono::steady_clock::now() - before < 100ms);
    CHECK_FALSE(slow.started);
}

TEST_CASE("Back during the computation does not jump forward when it ends",
          "[gui][correction-progress-page]") {
    const PatternCatalogue catalogue = emptyCatalogue();
    CorrectionSettings settings;
    settings.capitalization.enabled = false; // Common Errors alone, then progress
    CorrectionWizard wizard{catalogue, nullptr, settings, true, false};
    SlowComputation slow;
    wizard.progressPage().setComputation(slow.function());
    wizard.show();
    wizard.next(); // target -> common errors
    wizard.next(); // common errors -> progress: the computation starts
    REQUIRE(wizard.currentId() == CorrectionWizard::ProgressId);

    wizard.back();
    REQUIRE(wizard.currentId() == CorrectionWizard::CommonErrorsId);
    slow.awaitStart();
    while (!slow.ended)
        QTest::qWait(20);
    QTest::qWait(100); // let the watcher's `finished` be delivered

    CHECK(wizard.currentId() == CorrectionWizard::CommonErrorsId);
    CHECK_FALSE(wizard.progressPage().isComplete());

    // Next again: a fresh run, which does advance once it ends.
    QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
    wizard.next();
    REQUIRE(spy.wait(2000));
    CHECK(wizard.currentId() == CorrectionWizard::ConfirmationId);
}

TEST_CASE("destroying the page after its computation threw drops the exception",
          "[gui][correction-progress-page]") {
    std::atomic<bool> threw{false};
    {
        CorrectionProgressPage page;
        page.setComputation([&threw]() -> CorrectionProposal {
            threw = true;
            throw std::runtime_error{"the computation failed"};
        });
        page.initializePage();
        while (!threw)
            std::this_thread::yield();
        // No event loop runs here, so `finished` is never handled: the only
        // road to the stored exception is the destructor's own wait, which
        // must not let it escape.
    }
    CHECK(threw);
}
