// The four task pages of the correction assistant — issue #505, task 9.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/pattern_code_selector.hpp>

#include <QCheckBox>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <vector>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::CommonErrorsPage;
using subedit::gui::MentionsPage;
using subedit::gui::PatternCodeSelector;

const std::filesystem::path kShipped = "/patterns";

/// One `Zyyy` common-error record and one `Zyyy` hearing-impaired record,
/// built the way Task 1/5/8 already build small catalogues.
PatternCatalogue smallCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Letter I\nClasses=Human;OCR;\nPattern=a\n");
    files.addFile(kShipped / "Zyyy.hearing-impaired",
                  "# -*- conf -*-\n"
                  "\n[Hearing Impaired Pattern]\nName=Brackets\nPattern=\\[.*?\\]\n");
    return readPatternCatalogue(files, kShipped, {});
}

/// One common-error record under each of two codes that share nothing —
/// `Latn-en` and `Cyrl-ru` — so a code switch replaces every box.
PatternCatalogue twoCodes() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn-en.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=English rule\nClasses=Human;OCR;\nPattern=a\n");
    files.addFile(kShipped / "Cyrl-ru.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Russian rule\nClasses=Human;OCR;\nPattern=b\n");
    return readPatternCatalogue(files, kShipped, {});
}

[[nodiscard]] QCheckBox* boxNamed(const QWidget& page, const char* name) {
    return page.findChild<QCheckBox*>(QString::fromUtf8(name));
}
} // namespace

TEST_CASE("a common-errors page opens on its own code and classes", "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = smallCatalogue();
    CommonErrorsPage page{catalogue};

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    settings.human = true;
    settings.ocr = false;
    page.applySettings(settings);

    CHECK(page.code() == "Zyyy");
    CHECK(page.human());
    CHECK_FALSE(page.ocr());
}

TEST_CASE("a mentions page opens on the two D7 sound checkboxes, decoupled from patterns",
          "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = smallCatalogue();
    MentionsPage page{catalogue};

    CorrectionSettings settings;
    settings.soundInBrackets = true;
    page.applySettings(settings);

    CHECK(page.soundInBrackets());
    CHECK_FALSE(page.soundInParentheses());
}

TEST_CASE("a box unchecked under one code is found unchecked after switching away and back",
          "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = twoCodes();
    CommonErrorsPage page{catalogue};
    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Latn-en"};
    page.applySettings(settings);
    auto* selector = page.findChild<PatternCodeSelector*>();
    REQUIRE(selector != nullptr);

    QCheckBox* english = boxNamed(page, "English rule");
    REQUIRE(english != nullptr);
    english->setChecked(false);

    selector->setCode("Cyrl-ru");
    REQUIRE(boxNamed(page, "Russian rule") != nullptr);
    selector->setCode("Latn-en");

    english = boxNamed(page, "English rule");
    REQUIRE(english != nullptr);
    CHECK_FALSE(english->isChecked());
}

TEST_CASE("a box unchecked under one code still reaches the settings when the page ends on another",
          "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = twoCodes();
    CommonErrorsPage page{catalogue};
    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Latn-en"};
    page.applySettings(settings);
    auto* selector = page.findChild<PatternCodeSelector*>();
    REQUIRE(selector != nullptr);
    QCheckBox* english = boxNamed(page, "English rule");
    REQUIRE(english != nullptr);
    english->setChecked(false);

    selector->setCode("Cyrl-ru");

    // An override of another kind was already there: this page leaves it be.
    const subedit::core::PatternActivation otherKind{
        .kind = PatternKind::Capitalization, .code = "Zyyy", .name = "Other", .enabled = false};
    std::vector<subedit::core::PatternActivation> activations{otherKind};
    page.mergeActivationsInto(activations);

    CHECK(activations ==
          std::vector<subedit::core::PatternActivation>{otherKind,
                                                        {.kind = PatternKind::CommonError,
                                                         .code = "Latn-en",
                                                         .name = "English rule",
                                                         .enabled = false}});
}

TEST_CASE("re-checking a box an old run unchecked clears that run's override",
          "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = twoCodes();
    CommonErrorsPage page{catalogue};
    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Latn-en"};
    settings.patternActivations.push_back({.kind = PatternKind::CommonError,
                                           .code = "Latn-en",
                                           .name = "English rule",
                                           .enabled = false});
    page.applySettings(settings);
    QCheckBox* english = boxNamed(page, "English rule");
    REQUIRE(english != nullptr);
    REQUIRE_FALSE(english->isChecked());

    english->setChecked(true);
    std::vector<subedit::core::PatternActivation> activations = settings.patternActivations;
    page.mergeActivationsInto(activations);

    CHECK(activations.empty());
}
