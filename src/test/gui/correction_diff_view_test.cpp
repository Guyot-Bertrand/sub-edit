#include <subedit/core/text/text_diff.hpp>
#include <subedit/gui/correction_diff_view.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::DiffSpan;
using subedit::gui::correctionDiffHtml;
} // namespace

TEST_CASE("an unchanged span is written plainly", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "Bonjour", .changed = false}};
    CHECK(correctionDiffHtml(spans).toStdString() == "Bonjour");
}

TEST_CASE("a changed span is wrapped in bold", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "Bonjour ", .changed = false},
                                      DiffSpan{.text = "!", .changed = true}};
    CHECK(correctionDiffHtml(spans).toStdString() == "Bonjour <b>!</b>");
}

TEST_CASE("text that looks like markup is escaped before it is wrapped", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "<i>", .changed = true}};
    CHECK(correctionDiffHtml(spans).toStdString() == "<b>&lt;i&gt;</b>");
}
