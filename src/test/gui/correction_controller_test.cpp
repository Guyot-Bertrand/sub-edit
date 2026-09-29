// `CorrectionController` — issue #505, task 15: opens the wizard, resolves
// the target, applies what is accepted through the ordinary `Session::apply`
// road, and announces the tally.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_controller.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/pattern_code_selector.hpp>
#include <subedit/gui/project_page.hpp>

#include <QCheckBox>
#include <QDialog>
#include <QFont>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalSpy>
#include <QTableView>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::AttachedTranslation;
using subedit::core::attachTranslation;
using subedit::core::CorrectionSettings;
using subedit::core::CorrectionTask;
using subedit::core::InMemoryFileSystem;
using subedit::core::noticeOfCorrection;
using subedit::core::openProject;
using subedit::core::PatternActivation;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::Project;
using subedit::core::readPatternCatalogue;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::TranslationMethod;
using subedit::gui::CorrectionController;
using subedit::gui::CorrectionProgressPage;
using subedit::gui::CorrectionWizard;
using subedit::gui::PatternCodeSelector;
using subedit::gui::ProjectPage;
using subedit::test::FakePrompts;

const std::filesystem::path kShipped = "/patterns";

/// One `Zyyy` common-error record, "Double space" — enabled by default,
/// which is the one task exercised below. Built the way Task 1/5/8 already
/// build small catalogues (`correction_task_page_test.cpp`'s own
/// `smallCatalogue`).
///
/// **The replacement is `\040`, never a literal trailing space** — every line
/// of a pattern file is stripped before it is split on `=`
/// (`pattern_catalogue.cpp`'s own `stripped`), so a value ending the line in a
/// space loses it; `\040` is exactly the escape the shipped files use for the
/// same reason (`replacement_template.hpp`'s own doc comment).
[[nodiscard]] PatternCatalogue smallCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Double space\nClasses=Human;OCR;\n"
                  "Pattern= {2,}\nReplacement=\\040\n");
    return readPatternCatalogue(files, kShipped, {});
}

[[nodiscard]] std::unique_ptr<ProjectPage> pageOn(const char* content) {
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    return ProjectPage::make(std::move(opened->project));
}

/// The same, with a translation already attached — enough that
/// `translationFile()` answers, nothing more: a made-up line, matched or not,
/// still leaves the project with a translation file of its own.
[[nodiscard]] std::unique_ptr<ProjectPage> pageOnWithTranslation(const char* content) {
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    const std::vector<Subtitle> lines{Subtitle{.start = Timestamp::fromMilliseconds(1000),
                                               .end = Timestamp::fromMilliseconds(2000),
                                               .mainText = "Hello Marie"}};
    const AttachedTranslation attached =
        attachTranslation(opened->project, lines, SourceFile{}, TranslationMethod::Position);
    attached.command->apply(opened->project);
    return ProjectPage::make(std::move(opened->project));
}

/// The button of `Button` (`QPushButton`, `QCheckBox`, `QRadioButton`…) that
/// carries `text` — the same idiom `window_correction_test.cpp` already uses
/// to drive a wizard's widgets without an `objectName` of their own.
template<typename Button>
[[nodiscard]] Button* buttonNamed(const QWidget& parent, const QString& text) {
    for (Button* button : parent.findChildren<Button*>()) {
        if (button->text() == text)
            return button;
    }
    return nullptr;
}

/// The window, as the controller sees it.
class Desk final : public CorrectionController::View {
public:
    QWidget parent;
    PatternCatalogue catalogue;
    std::vector<std::unique_ptr<ProjectPage>> pages_;
    std::size_t shown = 0;
    std::vector<std::string> announced;
    std::vector<std::pair<const Project*, SubtitleIndex>> previewed;

    explicit Desk(PatternCatalogue catalogueIn) : catalogue(std::move(catalogueIn)) {}

    [[nodiscard]] QWidget* dialogParent() override { return &parent; }

    [[nodiscard]] std::span<const std::unique_ptr<ProjectPage>> pages() const override {
        return pages_;
    }

    [[nodiscard]] std::size_t shownProject() const override { return shown; }

    [[nodiscard]] const PatternCatalogue& patternCatalogue() const override { return catalogue; }

    [[nodiscard]] QFont applicationFont() const override { return QFont{}; }

    void announce(const std::string& message) override { announced.push_back(message); }

    void preview(const Project& project, SubtitleIndex index) override {
        previewed.emplace_back(&project, index);
    }
};

/// Drives a `CorrectionWizard` from the target page straight through to the
/// confirmation page, checking the common-errors task and waiting for its
/// computation.
///
/// **No `applySettings` override for the pattern list**: `smallCatalogue()`
/// ships its one record `Enabled` by default, `CorrectionSettings{}` starts
/// `commonErrors.enabled = true`, and `CorrectionWizard`'s own constructor
/// already calls `applySettings` once, from `CorrectionController::open()`,
/// with `desk`'s starting settings — so the box the pattern list shows for
/// "Double space" is already checked by the time this runs. (This is the
/// brief's own flagged gap: `wizard.settingsUsedForTest()` does not exist,
/// and this call is simply unnecessary once the wizard's own construction is
/// read.)
void driveToConfirmation(QDialog& dialog) {
    auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
    // `currentId()` reads -1 until the wizard has been shown at least once —
    // `FakePrompts::run` never calls `exec()`, unlike the production
    // `QtPrompts::run`, so this stands in for it (`correction_wizard_test.cpp`
    // shows the same wizard before driving it, for the same reason).
    wizard.show();
    // `CorrectionSettings{}` checks Common Errors *and* Capitalization by
    // default (Gaupol's own defaults) — Capitalization is turned off here so
    // that the only page between the target and progress is Common Errors.
    wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
    wizard.targetPage().setTaskChecked(CorrectionTask::Capitalization, false);
    wizard.next(); // target -> common errors
    wizard.next(); // common errors -> progress

    QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
    REQUIRE(spy.wait(2000));
    // The progress page auto-advances to the confirmation page once its
    // computation completes (`CorrectionProgressPage`'s own constructor
    // connection) — nothing further to drive here.
}

} // namespace

TEST_CASE("Finish applies the accepted corrections and announces the tally",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = &driveToConfirmation;
    CorrectionController controller{prompts, desk};

    controller.open();

    CHECK(desk.pages_[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
          "Bonjour Marie");
    CHECK(desk.pages_[0]->session->canUndo());
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == noticeOfCorrection(1, 0));
}

TEST_CASE("cancelling the wizard leaves every project and the settings untouched",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = false; // Cancel
    CorrectionController controller{prompts, desk};
    const CorrectionSettings before = controller.settings();

    controller.open();

    CHECK(desk.pages_[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
          "Bonjour  Marie");
    CHECK_FALSE(desk.pages_[0]->session->canUndo());
    CHECK(desk.announced.empty());
    CHECK(controller.settings() == before);
}

TEST_CASE("reopening with the same pattern toggled again replaces its own "
          "activation rather than duplicating it",
          "[gui][correction-controller]") {
    auto uncheckDoubleSpace = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.show();
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        wizard.targetPage().setTaskChecked(CorrectionTask::Capitalization, false);
        wizard.next(); // target -> common errors
        auto* box =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("Double space"));
        REQUIRE(box != nullptr);
        box->setChecked(false);
        wizard.next(); // common errors -> progress
        QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
        REQUIRE(spy.wait(2000));
    };
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = uncheckDoubleSpace;
    CorrectionController controller{prompts, desk};

    controller.open(); // first run: adds the override
    REQUIRE(controller.settings().patternActivations.size() == 1);

    controller.open(); // second run: finds it again, and replaces it in place

    CHECK(controller.settings().patternActivations.size() == 1);
    CHECK_FALSE(controller.settings().patternActivations[0].enabled);
}

TEST_CASE("the translation document is offered once any open project carries one",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    desk.pages_.push_back(pageOnWithTranslation("1\n00:00:01,000 --> 00:00:02,000\nAutre\n\n"));
    desk.shown = 0; // the shown project itself carries no translation

    bool translationEnabled = false;
    FakePrompts prompts;
    prompts.nextRun = false; // Cancel: read on the target page is all this needs
    prompts.fill = [&translationEnabled](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        auto* translation =
            buttonNamed<QRadioButton>(wizard.targetPage(), QStringLiteral("Translation"));
        REQUIRE(translation != nullptr);
        translationEnabled = translation->isEnabled();
    };
    CorrectionController controller{prompts, desk};

    controller.open();

    CHECK(translationEnabled);
}

TEST_CASE("Preview on the confirmation page reaches the view with the right "
          "project and index",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        driveToConfirmation(dialog);
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        auto* table = wizard.confirmationPage().findChild<QTableView*>();
        REQUIRE(table != nullptr);
        table->selectionModel()->select(table->model()->index(0, 0),
                                        QItemSelectionModel::Select | QItemSelectionModel::Rows);
        auto* preview =
            buttonNamed<QPushButton>(wizard.confirmationPage(), QStringLiteral("Preview"));
        REQUIRE(preview != nullptr);
        preview->click();
    };
    CorrectionController controller{prompts, desk};

    controller.open();

    REQUIRE(desk.previewed.size() == 1);
    CHECK(desk.previewed[0].first == &desk.pages_[0]->session->project());
    CHECK(desk.previewed[0].second == SubtitleIndex::fromValue(0));
}

namespace {

/// Checks Common Errors alone, opens its page, sets "Double space" to
/// `checked`, and runs to the confirmation page.
[[nodiscard]] auto settingDoubleSpace(bool checked) {
    return [checked](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.show();
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        wizard.targetPage().setTaskChecked(CorrectionTask::Capitalization, false);
        wizard.next(); // target -> common errors
        auto* box =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("Double space"));
        REQUIRE(box != nullptr);
        box->setChecked(checked);
        wizard.next(); // common errors -> progress
        QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
        REQUIRE(spy.wait(2000));
    };
}

/// One common-error record under each of two codes sharing nothing.
[[nodiscard]] PatternCatalogue twoCodesCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn-en.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=English rule\nClasses=Human;OCR;\nPattern=qqq\n");
    files.addFile(kShipped / "Cyrl-ru.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Russian rule\nClasses=Human;OCR;\nPattern=qqq\n");
    return readPatternCatalogue(files, kShipped, {});
}

} // namespace

TEST_CASE("re-enabling a pattern an earlier run disabled removes its override",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = settingDoubleSpace(false);
    CorrectionController controller{prompts, desk};

    controller.open(); // first run: disables it
    REQUIRE(controller.settings().patternActivations ==
            std::vector<PatternActivation>{{.kind = PatternKind::CommonError,
                                            .code = "Zyyy",
                                            .name = "Double space",
                                            .enabled = false}});

    prompts.fill = settingDoubleSpace(true);
    controller.open(); // second run: back to the shipped default

    // Not merely "no new override": the old one is gone, so the pattern is
    // enabled again the next time anything reads the settings.
    CHECK(controller.settings().patternActivations.empty());
}

TEST_CASE("a pattern unchecked under one code survives a switch to another and back",
          "[gui][correction-controller]") {
    Desk desk{twoCodesCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie\n\n"));
    bool stillUnchecked = false;
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [&stillUnchecked](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.show();
        wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
        wizard.targetPage().setTaskChecked(CorrectionTask::Capitalization, false);
        wizard.next(); // target -> common errors
        auto* selector = wizard.commonErrorsPage().findChild<PatternCodeSelector*>();
        REQUIRE(selector != nullptr);

        selector->setCode("Latn-en");
        auto* english =
            buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("English rule"));
        REQUIRE(english != nullptr);
        english->setChecked(false);

        selector->setCode("Cyrl-ru");
        selector->setCode("Latn-en");
        english = buttonNamed<QCheckBox>(wizard.commonErrorsPage(), QStringLiteral("English rule"));
        REQUIRE(english != nullptr);
        stillUnchecked = !english->isChecked();

        selector->setCode("Cyrl-ru"); // finish on the other code
        wizard.next();                // common errors -> progress
        QSignalSpy spy{&wizard.progressPage(), &CorrectionProgressPage::completeChanged};
        REQUIRE(spy.wait(2000));
    };
    CorrectionController controller{prompts, desk};

    controller.open();

    CHECK(stillUnchecked);
    CHECK(controller.settings().commonErrors.code == "Cyrl-ru");
    CHECK(controller.settings().patternActivations ==
          std::vector<PatternActivation>{{.kind = PatternKind::CommonError,
                                          .code = "Latn-en",
                                          .name = "English rule",
                                          .enabled = false}});
}

TEST_CASE("GUI-CORRECT-06: a pattern file line that cannot be read is named on the "
          "confirmation page",
          "[gui][correction-controller][GUI-CORRECT-06]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Double space\nClasses=Human;OCR;\n"
                  "Pattern= {2,}\nReplacement=\\040\n"
                  "this line is neither a header, a comment nor a key and value\n");
    PatternCatalogue catalogue = readPatternCatalogue(files, kShipped, {});
    REQUIRE(catalogue.diagnostics().size() == 1);
    REQUIRE(catalogue.diagnostics()[0].problem == subedit::core::PatternProblem::MalformedLine);

    Desk desk{std::move(catalogue)};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    std::string abandoned;
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [&abandoned](QDialog& dialog) {
        driveToConfirmation(dialog);
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        const auto* label =
            wizard.confirmationPage().findChild<QLabel*>(QStringLiteral("abandonedPatterns"));
        REQUIRE(label != nullptr);
        CHECK(label->isVisibleTo(&wizard));
        abandoned = label->text().toStdString();
    };
    CorrectionController controller{prompts, desk};

    controller.open();

    CHECK(abandoned.find("Zyyy.common-error, line 8") != std::string::npos);
    CHECK(abandoned.find("malformed line") != std::string::npos);
    // ... and the pattern that did read still applied.
    CHECK(desk.pages_[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
          "Bonjour Marie");
}

TEST_CASE("cancelling while the computation runs returns cleanly and changes nothing",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages_.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = false; // Cancel, the moment the progress page has started
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        wizard.show();
        wizard.targetPage().setTaskChecked(CorrectionTask::Capitalization, false);
        wizard.next(); // target -> common errors
        wizard.next(); // common errors -> progress: the computation starts
    };
    CorrectionController controller{prompts, desk};

    controller.open(); // the wizard, and the engine after it, are torn down here

    CHECK(desk.pages_[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
          "Bonjour  Marie");
    CHECK(desk.announced.empty());
}
