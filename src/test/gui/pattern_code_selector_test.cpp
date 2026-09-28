// The script/language/country widget of a task page — issue #505, task 7.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code_selector.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::PatternCodeSelector;

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
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};

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
