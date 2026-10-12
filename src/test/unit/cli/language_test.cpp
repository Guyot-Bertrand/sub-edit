// The command line under a second language — issue #663: what a person reads follows
// the catalogue, what a script reads does not. Read through the fixture `data/i18n/cli.po`,
// compiled as Russian, which has three plural forms.

#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/i18n/catalogue.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/duration.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Failure;
using subedit::cli::Reporter;
using subedit::cli::shiftAll;
using subedit::cli::summaryOf;
using subedit::cli::Verb;
using subedit::core::Catalogue;
using subedit::core::Duration;
using subedit::core::InMemoryFileSystem;
using subedit::core::installCatalogue;

namespace {

/// The fixture catalogue installed for as long as this lives, and English put back
/// after, whatever happened in the test.
class InRussian {
public:
    InRussian() {
        auto catalogue = Catalogue::fromFile(std::string{SUBEDIT_COMPILED_I18N_DIR} + "/cli.mo");
        REQUIRE(catalogue.has_value());
        installCatalogue(std::make_shared<const Catalogue>(std::move(*catalogue)));
    }

    InRussian(const InRussian&) = delete;
    InRussian& operator=(const InRussian&) = delete;
    InRussian(InRussian&&) = delete;
    InRussian& operator=(InRussian&&) = delete;

    ~InRussian() { installCatalogue(nullptr); }
};

} // namespace

TEST_CASE("a failure is worded in the language for a person and in English for the record",
          "[cli][i18n][CLI-I18N-01][CLI-I18N-02]") {
    const InRussian russian;
    InMemoryFileSystem files;
    std::ostringstream errors;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 1}.withRecords(records).forCommand("shift");

    const ExitCode code = shiftAll(files,
                                   {"absent.srt"},
                                   std::nullopt,
                                   Duration::fromMilliseconds(1000),
                                   Destination::from("", "out", false, 1).value(),
                                   reporter);

    CHECK(code == ExitCode::AllFailed);
    CHECK(errors.str() == "absent.srt — не существует\n");
    CHECK(records.str() == R"({"schema":1,"command":"shift","file":"absent.srt","ok":false,)"
                           R"("error":{"kind":"not-found","message":"absent.srt: does not exist"}})"
                           "\n");
}

TEST_CASE("a failure keeps its two wordings when it is put after what it is about",
          "[cli][i18n][CLI-I18N-02]") {
    const InRussian russian;
    const Failure failure{"not-found", [] { return subedit::core::translate("does not exist"); }};

    const Failure about = failure.about("b.srt");

    CHECK(about.kind == "not-found");
    CHECK(about.message == "b.srt — не существует");
    CHECK(about.english == "b.srt: does not exist");
}

TEST_CASE("the summary of a batch takes the form its numbers call for, of three",
          "[cli][i18n][CLI-I18N-01]") {
    const InRussian russian;

    // 21 takes the first form, 3 the second, 5 the third; so do the failures.
    CHECK(summaryOf(Verb::Shifted, 1, 21) == "сдвинут 1 из 21 файла, 20 с ошибками");
    CHECK(summaryOf(Verb::Shifted, 3, 3) == "сдвинуто 3 из 3 файлов (форма 2–4)");
    CHECK(summaryOf(Verb::Shifted, 5, 5) == "сдвинуто 5 из 5 файлов");
    CHECK(summaryOf(Verb::Shifted, 1, 2) == "сдвинуто 1 из 2 файлов (форма 2–4), 1 с ошибкой");
    CHECK(summaryOf(Verb::Shifted, 0, 4) ==
          "сдвинуто 0 из 4 файлов (форма 2–4), 4 с ошибками (форма 2–4)");
}

TEST_CASE("what the catalogue does not hold stays in English", "[cli][i18n][CLI-I18N-01]") {
    const InRussian russian;

    CHECK(summaryOf(Verb::Converted, 2, 2) == "2 of 2 files converted");
}
