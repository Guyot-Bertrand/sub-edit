// The reading of Gaupol's pattern files — issue #498, ADR 0037.
//
// Two kinds of test. The shipped files are read from the disk, as the program
// installed reads them, and what they hold is confronted with what the oracle of
// #494 wrote from the same files — never with what this reader was written to
// give. Everything else goes through `InMemoryFileSystem`: a malformed file is
// written in the test, where it can be read, and no test reaches a user's real
// directory, because the reader is handed its directories and none is handed to
// it.

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using subedit::core::CapitalizationFields;
using subedit::core::CapitalizeAt;
using subedit::core::CommonErrorFields;
using subedit::core::CorrectionPattern;
using subedit::core::FileErrorKind;
using subedit::core::HearingImpairedFields;
using subedit::core::InMemoryFileSystem;
using subedit::core::LineBreakFields;
using subedit::core::PatternCatalogue;
using subedit::core::PatternDiagnostic;
using subedit::core::PatternKind;
using subedit::core::PatternProblem;
using subedit::core::readPatternCatalogue;
using subedit::core::RealFileSystem;

constexpr std::array kAllKinds{PatternKind::CommonError,
                               PatternKind::Capitalization,
                               PatternKind::HearingImpaired,
                               PatternKind::LineBreak};

const std::filesystem::path kShipped = "/patterns";
const std::filesystem::path kUser = "/user";

/// The catalogue of what a test wrote in `/patterns` and `/user`.
PatternCatalogue readFrom(const InMemoryFileSystem& files) {
    return readPatternCatalogue(files, kShipped, kUser);
}

/// The problems of a reading, without the file they came from.
std::vector<PatternProblem> problemsOf(const PatternCatalogue& catalogue) {
    std::vector<PatternProblem> problems;
    for (const PatternDiagnostic& one : catalogue.diagnostics())
        problems.push_back(one.problem);
    return problems;
}

std::vector<std::string>
namesOf(const PatternCatalogue& catalogue, PatternKind kind, std::string_view code) {
    std::vector<std::string> names;
    for (const CorrectionPattern* one : catalogue.cascade(kind, code))
        names.push_back(one->name);
    return names;
}

/// The one record called `name`.
const CorrectionPattern& recordNamed(const PatternCatalogue& catalogue, std::string_view name) {
    const auto found = std::ranges::find_if(
        catalogue.patterns(), [name](const CorrectionPattern& one) { return one.name == name; });
    REQUIRE(found != catalogue.patterns().end());
    return *found;
}

/// A file of common errors with the records `bodies` name, each given its
/// header, a name, and the two keys a common error cannot do without.
std::string commonErrors(const std::vector<std::string>& bodies) {
    std::string file = "# -*- conf -*-\n";
    for (const std::string& body : bodies)
        file += "\n[Common Error Pattern]\n" + body + "\n";
    return file;
}

} // namespace

TEST_CASE("the shipped pattern files read without a word to say", "[text][pattern]") {
    const RealFileSystem files;
    const PatternCatalogue catalogue = readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});

    CHECK(catalogue.diagnostics().empty());

    // The 81 records of Gaupol's files: 63 common errors, 3 capitalizations, 6
    // mentions and 9 line breaks — counted by #492 in the files themselves.
    std::array<std::size_t, 4> counts{};
    for (const CorrectionPattern& one : catalogue.patterns())
        ++counts[static_cast<std::size_t>(one.kind())];
    CHECK(catalogue.patterns().size() == 81);
    CHECK(counts[static_cast<std::size_t>(PatternKind::CommonError)] == 63);
    CHECK(counts[static_cast<std::size_t>(PatternKind::Capitalization)] == 3);
    CHECK(counts[static_cast<std::size_t>(PatternKind::HearingImpaired)] == 6);
    CHECK(counts[static_cast<std::size_t>(PatternKind::LineBreak)] == 9);
}

TEST_CASE("every code's cascade is the one the oracle wrote from Gaupol's own reading",
          "[text][pattern]") {
    const RealFileSystem files;
    const PatternCatalogue catalogue = readPatternCatalogue(files, SUBEDIT_PATTERNS_DIR, {});

    std::ifstream fixture{std::filesystem::path{SUBEDIT_TEST_DATA_DIR} /
                          "motifs/attendus/cascades.txt"};
    REQUIRE(fixture.is_open());

    std::size_t checked = 0;
    std::string line;
    while (std::getline(fixture, line)) {
        if (line.empty() || line.starts_with('#'))
            continue;
        // `<kind> <request> | <code>:<rank><+|-> …`
        const std::size_t bar = line.find(" | ");
        REQUIRE(bar != std::string::npos);
        std::istringstream head{line.substr(0, bar)};
        std::string kindName;
        std::string request;
        head >> kindName >> request;

        const auto* const kind = std::ranges::find_if(kAllKinds, [&kindName](PatternKind one) {
            return subedit::core::fileExtensionOf(one) == kindName;
        });
        REQUIRE(kind != kAllKinds.end());

        std::vector<std::string> expected;
        std::istringstream items{line.substr(bar + 3)};
        for (std::string item; items >> item;)
            expected.push_back(item);

        std::vector<std::string> actual;
        for (const CorrectionPattern* one : catalogue.cascade(*kind, request)) {
            actual.push_back(one->code + ":" + std::to_string(one->rank) +
                             (one->enabled ? "+" : "-"));
        }
        INFO(kindName << " " << request);
        CHECK(actual == expected);
        ++checked;
    }
    // Four kinds, seven codes.
    CHECK(checked == 28);
}

TEST_CASE("a record reads into the fields its kind has", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn-en.common-error",
                  commonErrors({"Name=Letter I\nDescription=Fix it\nClasses=Human;OCR;\n"
                                "Pattern=(?<=[a-z])I\nFlags=DOTALL;MULTILINE;IGNORECASE;\n"
                                "Replacement=l\nRepeat=True"}));
    files.addFile(kShipped / "Latn-en.capitalization",
                  "[Capitalization Pattern]\nName=Sentence\nPattern=\\.\nCapitalize=After\n"
                  "[Capitalization Pattern]\nName=Pronoun\nPattern=\\bi\\b\nCapitalize=Start\n");
    files.addFile(
        kShipped / "Latn.hearing-impaired",
        "[Hearing Impaired Pattern]\nName=Brackets\nPattern=\\[.*?\\]\nReplacement=\\0\n");
    files.addFile(kShipped / "Latn.line-break",
                  "[Line Break Pattern]\nName=Clause\nPattern=(\\.)( )\nGroup=2\nPenalty=-10\n");

    const PatternCatalogue catalogue = readFrom(files);
    REQUIRE(catalogue.diagnostics().empty());
    REQUIRE(catalogue.patterns().size() == 5);

    const CorrectionPattern& error = recordNamed(catalogue, "Letter I");
    CHECK(error.kind() == PatternKind::CommonError);
    CHECK(error.code == "Latn-en");
    CHECK(error.rank == 1);
    CHECK(error.name == "Letter I");
    CHECK(error.description == "Fix it");
    CHECK(error.expression == "(?<=[a-z])I");
    CHECK(error.flags.dotAll);
    CHECK(error.flags.multiline);
    CHECK(error.flags.ignoreCase);
    const auto& fields = std::get<CommonErrorFields>(error.fields);
    CHECK(fields.classes.human);
    CHECK(fields.classes.ocr);
    CHECK(fields.replacement == "l");
    CHECK(fields.repeat);

    CHECK(std::get<CapitalizationFields>(recordNamed(catalogue, "Sentence").fields).capitalize ==
          CapitalizeAt::After);
    const CorrectionPattern& pronoun = recordNamed(catalogue, "Pronoun");
    CHECK(std::get<CapitalizationFields>(pronoun.fields).capitalize == CapitalizeAt::Start);
    CHECK(pronoun.rank == 2);
    // `\0` guards a replacement from GKeyFile; nothing follows it, so it goes.
    CHECK(std::get<HearingImpairedFields>(recordNamed(catalogue, "Brackets").fields)
              .replacement.empty());
    const auto& line = std::get<LineBreakFields>(recordNamed(catalogue, "Clause").fields);
    CHECK(line.group == 2);
    CHECK(line.penalty == -10.0);
}

TEST_CASE("a pattern file is read the way Gaupol reads it", "[text][pattern]") {
    InMemoryFileSystem files;
    // A BOM, CRLF line ends, a comment line, an indented comment, a key with the
    // underscore old files marked translatable fields with, blanks at the end of
    // a line, a `#` that is a musical note and not a comment, `\040`, which is a
    // space and not a guard, and a `\0` that is one.
    files.addFile(kShipped / "Zyyy.common-error",
                  "\xEF\xBB\xBF# -*- conf -*-\r\n\r\n[Common Error Pattern]\r\n   # not read\r\n"
                  "_Name=Notes \r\nPattern=#+\r\nClasses=Human;\r\nReplacement=-\\040\r\n"
                  "Description=Uses the \\0 guard\r\n");

    const PatternCatalogue catalogue = readFrom(files);
    REQUIRE(catalogue.diagnostics().empty());
    REQUIRE(catalogue.patterns().size() == 1);
    const CorrectionPattern& one = catalogue.patterns().front();
    CHECK(one.name == "Notes");
    CHECK(one.expression == "#+");
    CHECK(std::get<CommonErrorFields>(one.fields).replacement == "-\\040");
    CHECK(one.description == "Uses the  guard");
}

TEST_CASE("a record that cannot be read is named and refused, and the others are read",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(
        kShipped / "Latn.common-error",
        commonErrors({"Name=Fine\nClasses=OCR;\nPattern=a",
                      "Classes=OCR;\nPattern=a",                            // no name
                      "Name=No expression\nClasses=OCR;",                   // no Pattern
                      "Name=Flag\nClasses=OCR;\nPattern=a\nFlags=VERBOSE;", // a flag we do not read
                      "Name=Class\nClasses=Robot;\nPattern=a",              // not Human or OCR
                      "Name=No classes\nPattern=a",
                      "Name=Repeat\nClasses=OCR;\nPattern=a\nRepeat=Maybe",
                      "Name=Policy\nClasses=OCR;\nPattern=a\nPolicy=Append",
                      "Name=Empty\nClasses=OCR;\nPattern=",
                      "Name=Last\nClasses=Human;\nPattern=b"}));

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(problemsOf(catalogue) == std::vector<PatternProblem>{PatternProblem::MissingField,
                                                               PatternProblem::MissingField,
                                                               PatternProblem::InvalidValue,
                                                               PatternProblem::InvalidValue,
                                                               PatternProblem::MissingField,
                                                               PatternProblem::InvalidValue,
                                                               PatternProblem::InvalidValue,
                                                               PatternProblem::InvalidValue});
    // Two survive, and the last keeps the rank it has in the file.
    REQUIRE(catalogue.patterns().size() == 2);
    CHECK(catalogue.patterns()[0].name == "Fine");
    CHECK(catalogue.patterns()[1].name == "Last");
    CHECK(catalogue.patterns()[1].rank == 10);

    // A diagnostic says which file, which line, and what.
    const PatternDiagnostic& first = catalogue.diagnostics().front();
    CHECK(first.file == kShipped / "Latn.common-error");
    CHECK(first.detail == "Name");
    CHECK(first.line == 8);
}

TEST_CASE("a key the kind has no use for is named, and the record stays", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=A\nClasses=OCR;\nPattern=a\nPenalty=3\nColour=red"}));

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(catalogue.patterns().size() == 1);
    REQUIRE(catalogue.diagnostics().size() == 2);
    CHECK(catalogue.diagnostics()[0].problem == PatternProblem::UnknownField);
    CHECK(catalogue.diagnostics()[0].detail == "Colour");
    CHECK(catalogue.diagnostics()[1].detail == "Penalty");
}

TEST_CASE("lines that fit nowhere are named", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.common-error",
                  "Name=Before any record\n[Common Error Pattern]\nName=A\nClasses=OCR;\n"
                  "Pattern=a\nnot a field\n");

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(catalogue.patterns().size() == 1);
    CHECK(problemsOf(catalogue) == std::vector<PatternProblem>{PatternProblem::FieldOutsideRecord,
                                                               PatternProblem::MalformedLine});
    CHECK(catalogue.diagnostics()[0].line == 1);
    CHECK(catalogue.diagnostics()[1].line == 6);
}

TEST_CASE("a file that cannot be read is named and the others are read", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=A\nClasses=OCR;\nPattern=a"}));
    files.addFile(kShipped / "Latn.line-break",
                  "[Line Break Pattern]\nName=B\nPattern=b\nGroup=1\nPenalty=1\n");
    // Files come in sorted order: `Latn.common-error` is the first read.
    files.failNextRead(FileErrorKind::PermissionDenied);

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(problemsOf(catalogue) == std::vector<PatternProblem>{PatternProblem::FileUnreadable});
    REQUIRE(catalogue.patterns().size() == 1);
    CHECK(catalogue.patterns().front().kind() == PatternKind::LineBreak);
}

TEST_CASE("a shipped directory that is missing is named, a user's is the ordinary case",
          "[text][pattern]") {
    const InMemoryFileSystem none;

    const PatternCatalogue catalogue = readPatternCatalogue(none, kShipped, kUser);
    REQUIRE(catalogue.diagnostics().size() == 1);
    CHECK(catalogue.diagnostics().front().problem == PatternProblem::DirectoryUnreadable);
    CHECK(catalogue.diagnostics().front().file == kShipped);

    InMemoryFileSystem shipped;
    shipped.addFile(kShipped / "Latn.common-error",
                    commonErrors({"Name=A\nClasses=OCR;\nPattern=a"}));
    CHECK(readPatternCatalogue(shipped, kShipped, kUser).diagnostics().empty());
    // No user directory named at all is no reading at all.
    CHECK(readPatternCatalogue(shipped, kShipped, {}).patterns().size() == 1);
}

TEST_CASE("the user's patterns follow the shipped ones of the same code", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=Shipped\nClasses=OCR;\nPattern=a"}));
    files.addFile(kUser / "Latn.common-error",
                  commonErrors({"Name=Mine\nClasses=Human;\nPattern=b"}));
    files.addFile(kUser / "Latn-xx.common-error",
                  commonErrors({"Name=Only mine\nClasses=Human;\nPattern=c"}));
    // Neither a `.conf` nor a README is a pattern file.
    files.addFile(kUser / "LISEZMOI.md", "# read me");

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(catalogue.diagnostics().empty());
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn") ==
          std::vector<std::string>{"Shipped", "Mine"});
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-xx") ==
          std::vector<std::string>{"Shipped", "Mine", "Only mine"});
    CHECK(namesOf(catalogue, PatternKind::LineBreak, "Latn").empty());
}

TEST_CASE("SkipIn leaves a record out of the codes it names", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(
        kShipped / "Latn.common-error",
        commonErrors(
            {"Name=Everywhere\nClasses=OCR;\nPattern=a",
             "Name=Not in French\nClasses=OCR;\nPattern=b\nSkipIn=Latn-fr;",
             "Name=Not in French or Finnish\nClasses=OCR;\nPattern=c\nSkipIn=Latn-fi;Latn-fr;"}));

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-en").size() == 3);
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-fr") ==
          std::vector<std::string>{"Everywhere"});
    // A country of the language is a request for its language too.
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-fi-FI") ==
          std::vector<std::string>{"Everywhere", "Not in French"});
}

TEST_CASE("a name found again follows the ones it repeats, or replaces them under Policy=Replace",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=A\nClasses=OCR;\nPattern=1",
                                "Name=B\nClasses=OCR;\nPattern=2",
                                "Name=A\nClasses=OCR;\nPattern=3"}));
    files.addFile(kShipped / "Latn-en.common-error",
                  commonErrors({"Name=A\nClasses=OCR;\nPattern=4\nPolicy=Replace",
                                "Name=C\nClasses=OCR;\nPattern=5"}));
    files.addFile(kShipped / "Latn-fr.common-error",
                  commonErrors({"Name=A\nClasses=OCR;\nPattern=6"}));

    const PatternCatalogue catalogue = readFrom(files);

    // The two `A` of `Latn` are filed together, at the first one's place, and the
    // French one joins them. In English the replacing `A` takes the place of the
    // two it replaces, once the holes they leave are closed — the position
    // Gaupol's filter gives it — and `C` follows.
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn") ==
          std::vector<std::string>{"A", "A", "B"});
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-fr") ==
          std::vector<std::string>{"A", "A", "A", "B"});
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn-en") ==
          std::vector<std::string>{"A", "B", "C"});
    // Which `A` it is: the one that replaced.
    CHECK(catalogue.cascade(PatternKind::CommonError, "Latn-en").front()->expression == "4");
}

TEST_CASE("the activation of the shipped .conf files gives what is on by default",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.hearing-impaired",
                  "[Hearing Impaired Pattern]\nName=Sound in brackets\nPattern=a\n"
                  "[Hearing Impaired Pattern]\nName=Say \"hi\" & go\nPattern=b\n"
                  "[Hearing Impaired Pattern]\nName=Untouched\nPattern=c\n"
                  "[Hearing Impaired Pattern]\nName=Sound in brackets\nPattern=d\n");
    files.addFile(kShipped / "Latn.hearing-impaired.conf",
                  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<patterns>\n"
                  "  <!-- <pattern name=\"Untouched\" enabled=\"false\"/> -->\n"
                  "  <pattern name=\"Sound in brackets\" enabled=\"false\"/>\n"
                  "  <pattern name='Say &quot;hi&quot; &amp; go' enabled='false'/>\n"
                  "  <pattern name=\"No such pattern\" enabled=\"false\"/>\n"
                  "  <pattern enabled=\"false\"/>\n"
                  "</patterns>\n");

    const PatternCatalogue catalogue = readFrom(files);

    // Both records that share a name follow the one tick box.
    CHECK_FALSE(catalogue.patterns()[0].enabled);
    CHECK_FALSE(catalogue.patterns()[1].enabled);
    CHECK(catalogue.patterns()[2].enabled);
    CHECK_FALSE(catalogue.patterns()[3].enabled);
    // A name that matches nothing is ignored, as Gaupol ignores it; an element
    // that names nothing is not.
    CHECK(problemsOf(catalogue) ==
          std::vector<PatternProblem>{PatternProblem::MalformedActivation});
    CHECK(catalogue.diagnostics().front().line == 7);
}

TEST_CASE("a value other than true switches a pattern off, and the cascade lists it all the same",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.hearing-impaired",
                  "[Hearing Impaired Pattern]\nName=A\nPattern=a\n");
    files.addFile(kShipped / "Latn.hearing-impaired.conf",
                  R"(<patterns><pattern name="A" enabled="yes"/></patterns>)");

    const PatternCatalogue catalogue = readFrom(files);

    REQUIRE(catalogue.cascade(PatternKind::HearingImpaired, "Latn").size() == 1);
    CHECK_FALSE(catalogue.cascade(PatternKind::HearingImpaired, "Latn").front()->enabled);
}

TEST_CASE("the user's activation is not read from her pattern directory", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.hearing-impaired",
                  "[Hearing Impaired Pattern]\nName=A\nPattern=a\n");
    files.addFile(kUser / "Latn.hearing-impaired.conf",
                  R"(<patterns><pattern name="A" enabled="false"/></patterns>)");

    const PatternCatalogue catalogue = readFrom(files);

    // The choice a user makes is a setting of the program, applied on top of
    // this reading — ADR 0037.
    REQUIRE(catalogue.patterns().size() == 1);
    CHECK(catalogue.patterns().front().enabled);
}

TEST_CASE("the code Zyyy, or none, asks for the patterns of every script and nothing else",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Zyyy.common-error",
                  commonErrors({"Name=Any\nClasses=OCR;\nPattern=a"}));
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=Latin\nClasses=OCR;\nPattern=b"}));

    const PatternCatalogue catalogue = readFrom(files);

    // Once each: `Zyyy` is the root of every cascade, not a second level of it.
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Zyyy") == std::vector<std::string>{"Any"});
    CHECK(namesOf(catalogue, PatternKind::CommonError, "") == std::vector<std::string>{"Any"});
    CHECK(namesOf(catalogue, PatternKind::CommonError, "Latn") ==
          std::vector<std::string>{"Any", "Latin"});
}

TEST_CASE("a value that is not the number a key wants refuses the record", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.line-break",
                  "[Line Break Pattern]\nName=No group\nPattern=a\nPenalty=1\n"
                  "[Line Break Pattern]\nName=Bad group\nPattern=a\nGroup=two\nPenalty=1\n"
                  "[Line Break Pattern]\nName=Bad penalty\nPattern=a\nGroup=1\nPenalty=1x\n"
                  "[Line Break Pattern]\nName=Fine\nPattern=a\nGroup=1\nPenalty=2.5\n");
    files.addFile(kShipped / "Latn.capitalization",
                  "[Capitalization Pattern]\nName=No mode\nPattern=a\n"
                  "[Capitalization Pattern]\nName=Bad mode\nPattern=a\nCapitalize=Middle\n");
    files.addFile(kShipped / "Latn.common-error",
                  commonErrors({"Name=Empty classes\nClasses=\nPattern=a"}));

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(problemsOf(catalogue) ==
          std::vector<PatternProblem>{PatternProblem::MissingField,   // no Capitalize
                                      PatternProblem::InvalidValue,   // Capitalize=Middle
                                      PatternProblem::InvalidValue,   // Classes=
                                      PatternProblem::MissingField,   // no Group
                                      PatternProblem::InvalidValue,   // Group=two
                                      PatternProblem::InvalidValue}); // Penalty=1x
    REQUIRE(catalogue.patterns().size() == 1);
    CHECK(std::get<LineBreakFields>(catalogue.patterns().front().fields).penalty == 2.5);
}

TEST_CASE("an activation attribute that is not written as name=\"value\" is not read",
          "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.hearing-impaired",
                  "[Hearing Impaired Pattern]\nName=A\nPattern=a\n"
                  "[Hearing Impaired Pattern]\nName=B\nPattern=b\n");
    // `data-name` is not `name`; a missing `=`, a missing quote and an unclosed
    // quote each leave `enabled` alone, and a tag with a real `name` after them
    // is read all the same.
    files.addFile(kShipped / "Latn.hearing-impaired.conf",
                  R"(<pattern data-name="A" name="B" enabled="false"/>
<pattern name enabled="false"/>
<pattern name=A enabled="false"/>
<pattern name="A enabled="false"/>
<pattern name="A enabled=false/>)");

    const PatternCatalogue catalogue = readFrom(files);

    // Only `B` was named by a tag that could be read: the one that names `A` in
    // an attribute of another name, or with a quote that never closes, did not.
    CHECK(catalogue.patterns()[0].enabled);
    CHECK_FALSE(catalogue.patterns()[1].enabled);
    CHECK(problemsOf(catalogue).size() == 3);
}

TEST_CASE("an activation file that cannot be read is named", "[text][pattern]") {
    InMemoryFileSystem files;
    files.addFile(kShipped / "Latn.hearing-impaired.conf", "<patterns/>");
    files.failNextRead(FileErrorKind::Io);

    const PatternCatalogue catalogue = readFrom(files);

    CHECK(problemsOf(catalogue) == std::vector<PatternProblem>{PatternProblem::FileUnreadable});
}
