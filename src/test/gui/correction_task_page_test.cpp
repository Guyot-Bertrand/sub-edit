// The four task pages of the correction assistant — issue #505, task 9.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_task_page.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::readPatternCatalogue;
using subedit::gui::CommonErrorsPage;
using subedit::gui::MentionsPage;

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
