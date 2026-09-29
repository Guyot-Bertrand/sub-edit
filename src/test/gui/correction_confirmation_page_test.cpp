#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QStyledItemDelegate>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <vector>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::FailureKind;
using subedit::core::PatternFailure;
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionConfirmationPage;
using subedit::gui::CorrectionProgressPage;

[[nodiscard]] QPushButton* buttonNamed(const QWidget& parent, const QString& text) {
    for (QPushButton* button : parent.findChildren<QPushButton*>()) {
        if (button->text() == text)
            return button;
    }
    return nullptr;
}
} // namespace

TEST_CASE("the page builds its table from the progress page's result",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(
            ProposedCorrection{.project = nullptr,
                               .index = subedit::core::SubtitleIndex::fromValue(0),
                               .document = subedit::core::Document::Main,
                               .original = "a",
                               .proposed = std::string{"b"}});
        return result;
    });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    REQUIRE(page.resultModel() != nullptr);
    CHECK(page.resultModel()->rowCount() == 1);
}

TEST_CASE("a named pattern failure is shown when the run left one behind",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.failures.push_back(PatternFailure{.kind = FailureKind::CompileError,
                                                 .code = "Zyyy",
                                                 .rank = 1,
                                                 .name = "Broken",
                                                 .text = std::nullopt,
                                                 .detail = ""});
        return result;
    });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    CHECK(page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))
              ->text()
              .contains(QStringLiteral("Broken")));
}

TEST_CASE("the remove-blank-subtitles box opens on the default it is given",
          "[gui][correction-confirmation-page]") {
    CorrectionConfirmationPage page;
    page.setRemoveBlankSubtitlesDefault(false);

    CHECK_FALSE(page.removeBlankSubtitles());
}

TEST_CASE("every failure kind names its own reason", "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        for (const FailureKind kind : {FailureKind::Untranslatable,
                                       FailureKind::CompileError,
                                       FailureKind::InvalidReplacement,
                                       FailureKind::TimedOut,
                                       FailureKind::TooManyPasses,
                                       FailureKind::TooLong}) {
            result.failures.push_back(PatternFailure{.kind = kind,
                                                     .code = "Zyyy",
                                                     .rank = 1,
                                                     .name = "Broken",
                                                     .text = std::nullopt,
                                                     .detail = ""});
        }
        return result;
    });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    const QString text = page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))->text();
    CHECK(text.contains(QStringLiteral("cannot be translated")));
    CHECK(text.contains(QStringLiteral("will not compile")));
    CHECK(text.contains(QStringLiteral("has an invalid replacement")));
    CHECK(text.contains(QStringLiteral("timed out")));
    CHECK(text.contains(QStringLiteral("never settled")));
    CHECK(text.contains(QStringLiteral("grew the text too long")));
}

TEST_CASE("clicking Preview on a selected row asks to preview it",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(
            ProposedCorrection{.index = subedit::core::SubtitleIndex::fromValue(0),
                               .original = "a",
                               .proposed = std::string{"b"}});
        result.corrections.push_back(
            ProposedCorrection{.index = subedit::core::SubtitleIndex::fromValue(1),
                               .original = "c",
                               .proposed = std::string{"d"}});
        return result;
    });
    QSignalSpy ready{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(ready.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    auto* table = page.findChild<QTableView*>();
    REQUIRE(table != nullptr);
    table->selectionModel()->select(table->model()->index(1, 0),
                                    QItemSelectionModel::Select | QItemSelectionModel::Rows);

    const QSignalSpy requested{&page, &CorrectionConfirmationPage::previewRequested};
    QPushButton* preview = buttonNamed(page, QStringLiteral("Preview"));
    REQUIRE(preview != nullptr);
    preview->click();

    REQUIRE(requested.count() == 1);
    CHECK(requested.at(0).at(0).toInt() == 1);
}

TEST_CASE("re-entering the page reuses its one diff delegate",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] { return CorrectionProposal{}; });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();
    auto* table = page.findChild<QTableView*>();
    REQUIRE(table != nullptr);
    // `CorrectionDiffDelegate` carries no `Q_OBJECT`, so `findChildren` can
    // only count delegates in general — the view's own default among them.
    // What matters is that entering again adds none.
    const auto delegates = table->findChildren<QStyledItemDelegate*>().size();
    const QAbstractItemDelegate* original =
        table->itemDelegateForColumn(subedit::gui::CorrectionResultModel::Original);
    REQUIRE(original != nullptr);

    page.initializePage(); // Back, then Next again

    CHECK(table->findChildren<QStyledItemDelegate*>().size() == delegates);
    CHECK(table->itemDelegateForColumn(subedit::gui::CorrectionResultModel::Original) == original);
}

TEST_CASE("a reading problem with no line names its file alone",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] { return CorrectionProposal{}; });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.setReadDiagnostics(
        {subedit::core::PatternDiagnostic{.problem = subedit::core::PatternProblem::FileUnreadable,
                                          .file = "/patterns/Latn-en.capitalization",
                                          .line = 0,
                                          .detail = "permission denied"}});
    page.initializePage();

    const QString text = page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))->text();
    CHECK(text == QStringLiteral("Could not be read: Latn-en.capitalization "
                                 "(file cannot be read: permission denied)"));
}

TEST_CASE("every reading problem names its own reason", "[gui][correction-confirmation-page]") {
    using subedit::core::PatternDiagnostic;
    using subedit::core::PatternProblem;
    CorrectionProgressPage progress;
    progress.setComputation([] { return CorrectionProposal{}; });
    QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    std::vector<PatternDiagnostic> diagnostics;
    for (const PatternProblem problem : {PatternProblem::FileUnreadable,
                                         PatternProblem::MalformedLine,
                                         PatternProblem::FieldOutsideRecord,
                                         PatternProblem::UnknownField,
                                         PatternProblem::MissingField,
                                         PatternProblem::InvalidValue,
                                         PatternProblem::MalformedActivation}) {
        diagnostics.push_back(PatternDiagnostic{
            .problem = problem, .file = "/patterns/Zyyy.common-error", .line = 3, .detail = {}});
    }
    // A directory given with its trailing separator has no file name of its
    // own: the whole path names it instead.
    diagnostics.push_back(PatternDiagnostic{.problem = PatternProblem::DirectoryUnreadable,
                                            .file = "/patterns/",
                                            .line = 0,
                                            .detail = {}});
    page.setReadDiagnostics(diagnostics);
    page.initializePage();

    const QString text = page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))->text();
    CHECK(text.contains(QStringLiteral("Zyyy.common-error, line 3 (file cannot be read)")));
    CHECK(text.contains(QStringLiteral("(malformed line)")));
    CHECK(text.contains(QStringLiteral("(field outside any pattern)")));
    CHECK(text.contains(QStringLiteral("(unknown field)")));
    CHECK(text.contains(QStringLiteral("(missing field)")));
    CHECK(text.contains(QStringLiteral("(invalid value)")));
    CHECK(text.contains(QStringLiteral("(malformed activation)")));
    CHECK(text.contains(QStringLiteral("/patterns/ (directory cannot be read)")));
}
