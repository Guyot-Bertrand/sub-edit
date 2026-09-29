// Decomposing pattern codes into script, language, country — issue #505, task 5.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::countriesOf;
using subedit::gui::joinCode;
using subedit::gui::languagesOf;
using subedit::gui::PatternCodeParts;
using subedit::gui::scriptsOf;
using subedit::gui::splitCode;

const std::filesystem::path kShipped = "/patterns";
const std::filesystem::path kUser = "/user";

/// A file of common errors with the records `bodies` name, each given its
/// header, a name, and the two keys a common error cannot do without.
std::string commonErrors(const std::vector<std::string>& bodies) {
    std::string file = "# -*- conf -*-\n";
    for (const std::string& body : bodies)
        file += "\n[Common Error Pattern]\n" + body + "\n";
    return file;
}

} // namespace

TEST_CASE("splitting and joining a code round-trip at every depth", "[gui][pattern-code]") {
    CHECK(splitCode("Zyyy") == PatternCodeParts{.script = "Zyyy"});
    CHECK(splitCode("Latn") == PatternCodeParts{.script = "Latn"});
    CHECK(splitCode("Latn-en") == PatternCodeParts{.script = "Latn", .language = "en"});
    CHECK(splitCode("Latn-en-US") ==
          PatternCodeParts{.script = "Latn", .language = "en", .country = "US"});

    CHECK(joinCode(PatternCodeParts{.script = "Latn", .language = "en", .country = "US"}) ==
          "Latn-en-US");
    CHECK(joinCode(PatternCodeParts{.script = "Latn", .language = "en"}) == "Latn-en");
    CHECK(joinCode(PatternCodeParts{.script = "Latn"}) == "Latn");
    CHECK(joinCode(PatternCodeParts{}) == "Zyyy");
}

TEST_CASE("the three combos read what the catalogue actually carries, cascading",
          "[gui][pattern-code]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  commonErrors({"Name=Universal\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=Latin Script\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn-en.common-error",
                  commonErrors({"Name=English\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn-en-US.common-error",
                  commonErrors({"Name=US English\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));
    files.addFile(kShipped / "Latn-fr.common-error",
                  commonErrors({"Name=French\nDescription=Test\nClasses=Human;\n"
                                "Pattern=test\nReplacement=ok"}));

    const auto catalogue = readPatternCatalogue(files, kShipped, kUser);

    CHECK(scriptsOf(catalogue, PatternKind::CommonError) ==
          std::vector<std::string>{"Zyyy", "Latn"});
    CHECK(languagesOf(catalogue, PatternKind::CommonError, "Latn") ==
          std::vector<std::string>{"en", "fr"});
    CHECK(countriesOf(catalogue, PatternKind::CommonError, "Latn", "en") ==
          std::vector<std::string>{"US"});
    // A script with only a bare code offers no language at all.
    CHECK(languagesOf(catalogue, PatternKind::CommonError, "Zyyy").empty());
}
