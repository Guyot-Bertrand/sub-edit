// Every place that lists the formats by hand, held to the list the core walks.
//
// The code derives its lists from `kSubtitleFormats`: the `--to` values, the file dialog's
// filters, the names and the extensions. A `switch` that forgets a format fails to compile. What
// is written by hand escapes both: the manual's tables, and the extensions the detection probe
// walks. A format added to the core and to nothing else would be offered by the program, missing
// from its manual, and invisible to the measure of the private corpus — in silence (#836).
//
// The English manual only: `check-translation-structure.py` makes every translated tree carry the
// same table rows and the same code spans, and the names of formats are never translated.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/wording/formats.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace {

using subedit::core::extensionOf;
using subedit::core::kSubtitleFormats;
using subedit::core::nameOf;
using subedit::core::optionNameOf;
using subedit::core::RealFileSystem;
using subedit::core::SubtitleFormat;

[[nodiscard]] std::string contentOf(const std::filesystem::path& path) {
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("cannot read " + path.string());
        return {};
    }
    return *std::move(content);
}

[[nodiscard]] std::string manualPage(std::string_view page) {
    return contentOf(std::filesystem::path{SUBEDIT_MANUAL_DIR} / page);
}

/// "`text`", as the manual writes a name typed or shown.
[[nodiscard]] std::string spanned(std::string_view text) {
    std::string span{"`"};
    span += text;
    span += '`';
    return span;
}

} // namespace

TEST_CASE("the formats --to accepts are the core's, each on its row of convert's manual",
          "[format][manual]") {
    const std::string page = manualPage("subedit-cli/convert.md");
    for (const SubtitleFormat format : kSubtitleFormats) {
        // | `srt` | SubRip | `.srt` |
        std::string row{"| "};
        row += spanned(optionNameOf(format));
        row += " | ";
        row += nameOf(format);
        row += " | ";
        row += spanned(extensionOf(format));
        row += " |";
        INFO(row);
        CHECK(page.find(row) != std::string::npos);
    }
}

TEST_CASE("the window's manual has a filter entry for each format of the core",
          "[format][manual]") {
    const std::string page = manualPage("subedit-gui/files.md");
    for (const SubtitleFormat format : kSubtitleFormats) {
        // | `SubRip` | `.srt` |
        std::string row{"| "};
        row += spanned(nameOf(format));
        row += " | ";
        row += spanned(extensionOf(format));
        row += " |";
        INFO(row);
        CHECK(page.find(row) != std::string::npos);
    }
}

TEST_CASE("inspect's manual and the manual's first page name each format of the core",
          "[format][manual]") {
    const std::string inspect = manualPage("subedit-cli/inspect.md");
    const std::string index = manualPage("index.md");
    for (const SubtitleFormat format : kSubtitleFormats) {
        INFO(nameOf(format));
        CHECK(inspect.find(spanned(nameOf(format))) != std::string::npos);
        CHECK(index.find(nameOf(format)) != std::string::npos);
    }
}

TEST_CASE("the detection probe walks the extension of each format of the core", "[format]") {
    // The probe of the private corpus filters on extensions, and a filter that missed one would
    // make a format invisible to the only measure of real files — the defect the journal of
    // `detection-de-format.md` already paid for once, with `.txt`.
    const std::string script =
        contentOf(std::filesystem::path{SUBEDIT_SCRIPTS_DIR} / "score-format-detection.py");
    const std::size_t start = script.find("SUBTITLE_SUFFIXES = (");
    REQUIRE(start != std::string::npos);
    const std::string suffixes = script.substr(start, script.find(')', start) - start);
    for (const SubtitleFormat format : kSubtitleFormats) {
        std::string quoted{"\""};
        quoted += extensionOf(format);
        quoted += '"';
        INFO(quoted);
        CHECK(suffixes.find(quoted) != std::string::npos);
    }
}
