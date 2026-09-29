// The script/language/country widget of a task page — issue #505, task 7.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code_selector.hpp>

#include <QComboBox>
#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::PatternCodeSelector;

/// The combo among the selector's three whose first item reads `firstItem` —
/// `(every language)`/`(every country)` name the last two on their own; the
/// script combo, which has no such placeholder, is found by elimination.
[[nodiscard]] QComboBox* comboWithFirstItem(const QWidget& parent, const QString& firstItem) {
    for (QComboBox* combo : parent.findChildren<QComboBox*>()) {
        if (combo->itemText(0) == firstItem)
            return combo;
    }
    return nullptr;
}

const std::filesystem::path kShipped = "/patterns";
const std::filesystem::path kUser = "/user";

/// A file of common errors with the records `bodies` name, each given its
/// header, a name, and the two keys a common error cannot do without — the
/// same recipe as `pattern_code_test.cpp`'s `commonErrors()`.
std::string commonErrors(const std::vector<std::string>& bodies) {
    std::string file = "# -*- conf -*-\n";
    for (const std::string& body : bodies)
        file += "\n[Common Error Pattern]\n" + body + "\n";
    return file;
}

/// Zyyy (bare), Latn-en-US, Latn-fr — enough to exercise every cascade.
PatternCatalogue smallCatalogue() {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  commonErrors({"Name=Universal\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn-en-US.common-error",
                  commonErrors({"Name=US English\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn-fr.common-error",
                  commonErrors({"Name=French\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    return readPatternCatalogue(files, kShipped, kUser);
}
} // namespace

TEST_CASE("it opens on the first script the catalogue carries, unfiltered below it",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    const PatternCodeSelector selector{catalogue, PatternKind::CommonError};

    CHECK(selector.code() == "Zyyy");
}

TEST_CASE("setCode selects each combo and cascades the ones under it",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};

    selector.setCode("Latn-en-US");

    CHECK(selector.code() == "Latn-en-US");
}

TEST_CASE("moving to a script with no languages resets to the bare code",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};
    selector.setCode("Latn-en-US");

    selector.setCode("Zyyy");

    CHECK(selector.code() == "Zyyy");
}

TEST_CASE("setCode emits codeChanged exactly once, after every combo has settled",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};
    std::vector<std::string> codesSeenDuringEmission;
    QObject::connect(&selector, &PatternCodeSelector::codeChanged, [&] {
        codesSeenDuringEmission.push_back(selector.code());
    });

    selector.setCode("Latn-en-US");

    CHECK(codesSeenDuringEmission == std::vector<std::string>{"Latn-en-US"});
}

TEST_CASE("picking a script directly on its own combo cascades the languages under it",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    const PatternCodeSelector selector{catalogue, PatternKind::CommonError};
    const QSignalSpy spy{&selector, &PatternCodeSelector::codeChanged};

    QComboBox* script = comboWithFirstItem(selector, QStringLiteral("Zyyy"));
    REQUIRE(script != nullptr);
    const int latn = script->findText(QStringLiteral("Latn"));
    REQUIRE(latn >= 0);

    script->setCurrentIndex(latn);

    CHECK(selector.code() == "Latn");
    CHECK(spy.count() == 1);
}

TEST_CASE("picking a language directly on its own combo cascades the countries under it",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};
    selector.setCode("Latn");
    const QSignalSpy spy{&selector, &PatternCodeSelector::codeChanged};

    QComboBox* language = comboWithFirstItem(selector, QStringLiteral("(every language)"));
    REQUIRE(language != nullptr);
    const int en = language->findText(QStringLiteral("en"));
    REQUIRE(en >= 0);

    language->setCurrentIndex(en);

    CHECK(selector.code() == "Latn-en");
    CHECK(spy.count() == 1);
}
