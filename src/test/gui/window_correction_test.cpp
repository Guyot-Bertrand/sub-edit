// `Tools ▸ Correct Texts…` wired into the window — issue #505, task 15b.
//
// One case per registry entry, driven the way `window_hearing_impaired_test.cpp`
// and `window_tabs_test.cpp` already drive a real `MainWindow`: a `FakePrompts`
// whose `fill` plays the user inside the wizard, `nextRun` standing in for
// Finish (`true`) or Cancel (`false`).
//
// **Every catalogue here is built in the test itself**, `InMemoryFileSystem` +
// `readPatternCatalogue`, the way `pattern_catalogue_test.cpp` and
// `correction_controller_test.cpp`'s own `smallCatalogue` already do — never
// the shipped files, so a test asserting a specific pattern's behaviour never
// drifts with them.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QSignalSpy>
#include <QStatusBar>
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::CorrectionTask;
using subedit::core::InMemoryFileSystem;
using subedit::core::noticeOfCorrection;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::PatternCatalogue;
using subedit::core::readPatternCatalogue;
using subedit::gui::CorrectionProgressPage;
using subedit::gui::CorrectionResultModel;
using subedit::gui::CorrectionWizard;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

const std::filesystem::path kShipped = "/patterns";
const std::filesystem::path kUser = "/user";

[[nodiscard]] InMemoryFileSystem withFile(const char* content) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path = "film.srt") {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, 4), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// A button found by its own label — the pattern list gives each checkbox
/// that text and that `objectName` alike (`PatternList::setCode`, Task 8);
/// the D4/D7 checkboxes (`Human`/`OCR`, `Sound in brackets`/`Sound in
/// parentheses`) and the target page's radio buttons carry no `objectName` at
/// all, only their text — so matching on text covers every case with one
/// lookup, whichever kind of `QAbstractButton` is asked for.
template<class Button>
[[nodiscard]] Button* buttonNamed(const QWidget& parent, const QString& text) {
    for (Button* button : parent.findChildren<Button*>()) {
        if (button->text() == text)
            return button;
    }
    return nullptr;
}

/// Shows the wizard — `currentId()` reads -1 until it has been shown at least
/// once, since `FakePrompts::run` never calls `exec()` the way the production
/// `QtPrompts::run` does (`correction_controller_test.cpp`'s own
/// `driveToConfirmation` notes the same thing) — then walks forward to the
/// progress page and waits for its computation, whichever tasks are checked.
///
/// **A loop on `nextId()`, not a fixed count of `next()`** — `CorrectionSettings{}`
/// checks Common Errors *and* Capitalization by default (Gaupol's own
/// defaults), so a case that only means to exercise one of the four tasks
/// still walks through whichever others are on; a fixed number of `next()`
/// calls would land on the wrong page and never complete.
void advanceToConfirmation(CorrectionWizard& wizard) {
    wizard.show();
    while (wizard.currentId() != CorrectionWizard::ProgressId)
        wizard.next();
    QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
    REQUIRE(spy.wait(2000));
    // The progress page auto-advances to the confirmation page once its
    // computation completes — nothing further to drive here.
}

/// One text, one common error to fix — `Bonjour  Marie`'s double space.
constexpr const char* kOneError = "1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n";

/// One `Zyyy` common-error record, "Double space", carrying both classes —
/// built the way `correction_controller_test.cpp`'s own `smallCatalogue`
/// already does.
[[nodiscard]] PatternCatalogue doubleSpaceCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Double space\nClasses=Human;OCR;\n"
                  "Pattern= {2,}\nReplacement=\\040\n");
    return readPatternCatalogue(files, kShipped, {});
}

/// Two `Zyyy` common-error records, one of each class alone — D4.
[[nodiscard]] PatternCatalogue humanAndOcrCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Human fix\nClasses=Human;\n"
                  "Pattern=Bonjour\nReplacement=Salut\n"
                  "\n[Common Error Pattern]\nName=OCR fix\nClasses=OCR;\n"
                  "Pattern=Marie\nReplacement=Marion\n");
    return readPatternCatalogue(files, kShipped, {});
}

/// One record that will not compile, and one that does — GUI-CORRECT-06.
[[nodiscard]] PatternCatalogue brokenAndGoodCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Broken\nClasses=Human;OCR;\n"
                  "Pattern=(\nReplacement=x\n"
                  "\n[Common Error Pattern]\nName=Double space\nClasses=Human;OCR;\n"
                  "Pattern= {2,}\nReplacement=\\040\n");
    return readPatternCatalogue(files, kShipped, {});
}

/// One record shipped, one of the user's own — GUI-CORRECT-07.
[[nodiscard]] PatternCatalogue shippedAndUserCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Shipped fix\nClasses=Human;OCR;\n"
                  "Pattern=Bonjour\nReplacement=Salut\n");
    files.addFile(kUser / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=User fix\nClasses=Human;OCR;\n"
                  "Pattern=Marie\nReplacement=Marion\n");
    return readPatternCatalogue(files, kShipped, kUser);
}

/// One `Zyyy` hearing-impaired record, matching `#…#` — GUI-HEARING-03.
[[nodiscard]] PatternCatalogue musicCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.hearing-impaired",
                  "# -*- conf -*-\n"
                  "\n[Hearing Impaired Pattern]\nName=Music\nPattern=#[^#]*#\nReplacement=\\0\n");
    return readPatternCatalogue(files, kShipped, {});
}

} // namespace

TEST_CASE("GUI-CORRECT-01: the assistant applies checked tasks to the chosen target and document",
          "[gui][GUI-CORRECT-01]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        advanceToConfirmation(wizard);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(doubleSpaceCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour Marie");
}

TEST_CASE("GUI-CORRECT-01: choosing All Open Projects reaches every open project, not just "
          "the one shown",
          "[gui][GUI-CORRECT-01]") {
    // The default target (current project) is exercised by the case above;
    // this one drives the other end of the same choice — `CorrectionScope::AllProjects`
    // — the only way to prove `wizard.targetPage().scope()` actually reaches
    // `correctionTargetsOf` and the multi-project apply loop inside `open()`,
    // rather than every case happening to agree with the default.
    InMemoryFileSystem files;
    files.addFile("premier.srt", "1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n");
    files.addFile("second.srt", "1\n00:00:01,000 --> 00:00:02,000\nSalut  Jean\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.setPatternCatalogue(doubleSpaceCatalogue());
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->count() == 2);
    REQUIRE(window.tabBar()->currentIndex() == 1); // "second.srt" is the one shown

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        // No setter for the scope on `CorrectionTargetPage` — the radio
        // buttons carry no `objectName`, only their own text, so this drives
        // the actual widget the way a click would, the same idiom already
        // used above for the pattern list's and the task pages' checkboxes.
        QRadioButton* allProjects =
            buttonNamed<QRadioButton>(wizard.targetPage(), QStringLiteral("All Open Projects"));
        REQUIRE(allProjects != nullptr);
        allProjects->setChecked(true);
        advanceToConfirmation(wizard);
    };
    window.correctTextsAction()->trigger();

    // Both tabs corrected — the one shown when the assistant opened, and the
    // other one, which `shownProject()` never named.
    window.tabBar()->setCurrentIndex(0);
    CHECK(textAt(window, 0) == "Bonjour Marie");
    window.tabBar()->setCurrentIndex(1);
    CHECK(textAt(window, 0) == "Salut Jean");
}

TEST_CASE(
    "GUI-CORRECT-02: a changed text shows its original, and is accepted, refused or retouched",
    "[gui][GUI-CORRECT-02]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        advanceToConfirmation(wizard);
        // The confirmation page is current now: the row's own original shows,
        // unmarked text and all — then refuse it.
        CorrectionResultModel* model = wizard.confirmationPage().resultModel();
        REQUIRE(model->rowCount({}) == 1);
        CHECK(model->data(model->index(0, CorrectionResultModel::Original), Qt::DisplayRole)
                  .toString()
                  .contains(QStringLiteral("Bonjour")));
        model->markAll(false);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(doubleSpaceCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour  Marie"); // refused: unchanged
}

TEST_CASE("GUI-CORRECT-03: applying makes one history entry and the status bar says the count",
          "[gui][GUI-CORRECT-03]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        advanceToConfirmation(wizard);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(doubleSpaceCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(window.statusBar()->currentMessage().toStdString() == noticeOfCorrection(1, 0));
    CHECK(window.undoAction()->isEnabled());
    window.undoAction()->trigger();
    CHECK(textAt(window, 0) == "Bonjour  Marie"); // one undo puts it all back
}

TEST_CASE("GUI-CORRECT-04: a pattern turned off in the assistant stays off next time it opens",
          "[gui][GUI-CORRECT-04]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(doubleSpaceCatalogue());
    window.show();

    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        QCheckBox* box =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("Double space"));
        REQUIRE(box != nullptr);
        box->setChecked(false);
        advanceToConfirmation(wizard);
    };
    window.correctTextsAction()->trigger();

    // Reopening builds a fresh wizard from the settings the first run left —
    // `CorrectionSettings.patternActivations`, D2/D8 — read straight off the
    // box it opens on, no navigation needed.
    bool boxUncheckedOnReopen = false;
    prompts.nextRun = false; // Cancel: nothing left to drive
    prompts.fill = [&boxUncheckedOnReopen](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        QCheckBox* box =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("Double space"));
        REQUIRE(box != nullptr);
        boxUncheckedOnReopen = !box->isChecked();
    };
    window.correctTextsAction()->trigger();

    CHECK(boxUncheckedOnReopen);
}

TEST_CASE("GUI-CORRECT-05: unchecking a class leaves only the other class's own changes",
          "[gui][GUI-CORRECT-05]") {
    InMemoryFileSystem files = withFile("1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie\n\n");
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        QCheckBox* ocrBox =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("OCR"));
        REQUIRE(ocrBox != nullptr);
        ocrBox->setChecked(false);
        advanceToConfirmation(wizard);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(humanAndOcrCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    // The Human-classed record landed, the OCR-classed one did not — D4.
    CHECK(textAt(window, 0) == "Salut Marie");
}

TEST_CASE("GUI-CORRECT-06: a pattern that will not compile is named, and the rest still apply",
          "[gui][GUI-CORRECT-06]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    std::string abandoned;
    prompts.fill = [&abandoned](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        advanceToConfirmation(wizard);
        const QLabel* label =
            wizard.confirmationPage().findChild<QLabel*>(QStringLiteral("abandonedPatterns"));
        REQUIRE(label != nullptr);
        abandoned = label->text().toStdString();
    };
    // `setPatternCatalogue` before `show()`, deliberately: the window under
    // test must never read the shipped patterns.
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(brokenAndGoodCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(abandoned.find("Broken") != std::string::npos);
    CHECK(textAt(window, 0) == "Bonjour Marie"); // the pattern that compiles still applied
}

TEST_CASE("GUI-CORRECT-07: the shipped and the user's own patterns both apply",
          "[gui][GUI-CORRECT-07]") {
    InMemoryFileSystem files = withFile("1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie\n\n");
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        advanceToConfirmation(wizard);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(shippedAndUserCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(textAt(window, 0) == "Salut Marion");
}

TEST_CASE("GUI-HEARING-03: the moteur-based mentions and the bracket scan fire from the same run",
          "[gui][GUI-HEARING-03]") {
    InMemoryFileSystem files =
        withFile("1\n00:00:01,000 --> 00:00:02,000\nBonjour #music# Marie\n\n"
                 "2\n00:00:03,000 --> 00:00:04,000\nSalut [bruit] Jean\n\n");
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.targetPage().setTaskChecked(CorrectionTask::Mentions, true);
        QCheckBox* box =
            buttonNamed<QCheckBox>(wizard.mentionsPage(), QStringLiteral("Sound in brackets"));
        REQUIRE(box != nullptr);
        box->setChecked(true);
        advanceToConfirmation(wizard);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.setPatternCatalogue(musicCatalogue());
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour Marie"); // the moteur-based `#…#` pattern
    CHECK(textAt(window, 1) == "Salut Jean");    // the balayage-based bracket removal
}
