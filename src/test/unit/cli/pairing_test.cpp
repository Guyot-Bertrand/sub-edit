// Laying a translation over a main file, on an in-memory file system.
//
// What is proved here is the layer between the core's attachment and a file: the
// failures a missing or identical translation gives, and that a paired run
// writes the translation and nothing else.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/text_rewriting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Pairing;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;
using subedit::core::TranslationMethod;

namespace {

const std::string kMain = "1\n"
                          "00:00:01,000 --> 00:00:03,000\n"
                          "Hello there.\n"
                          "\n"
                          "2\n"
                          "00:00:04,000 --> 00:00:06,000\n"
                          "Goodbye.\n"
                          "\n";

const std::string kTranslation = "1\n"
                                 "00:00:01,000 --> 00:00:03,000\n"
                                 "[Un bruit] Bonjour.\n"
                                 "\n"
                                 "2\n"
                                 "00:00:04,000 --> 00:00:06,000\n"
                                 "Au revoir.\n"
                                 "\n";

InMemoryFileSystem filesWithBoth() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kMain);
    files.addFile("film.fr.srt", kTranslation);
    return files;
}

} // namespace

TEST_CASE("pairing reads the translation and attaches it, saying how the lines matched",
          "[cli][pairing]") {
    const InMemoryFileSystem files = filesWithBoth();
    auto opened = subedit::core::openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    subedit::core::Session session{std::move(opened->project)};

    const auto paired = subedit::cli::pair(
        files,
        session,
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Position},
        {});

    REQUIRE(paired.has_value());
    CHECK(paired->outcome.attached == 2);
    CHECK(paired->outcome.born == 0);
    CHECK(session.project().subtitles()[0].translationText == "[Un bruit] Bonjour.");
}

TEST_CASE("a translation that cannot be read fails with the identifier of its reason",
          "[cli][pairing]") {
    const InMemoryFileSystem files = filesWithBoth();
    auto opened = subedit::core::openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    subedit::core::Session session{std::move(opened->project)};

    const auto missing = subedit::cli::pair(
        files,
        session,
        Pairing{.translation = "absent.srt", .method = TranslationMethod::Position},
        {});
    REQUIRE_FALSE(missing.has_value());
    CHECK(missing.error().kind == "not-found");

    const auto same =
        subedit::cli::pair(files,
                           session,
                           Pairing{.translation = "film.srt", .method = TranslationMethod::Number},
                           {});
    REQUIRE_FALSE(same.has_value());
    CHECK(same.error().kind == "same-file-as-main");
    CHECK(same.error().message == "the file is already open as the main document");
}

TEST_CASE("a paired run writes the translation, at its own path, and not the main file",
          "[cli][pairing]") {
    InMemoryFileSystem files = filesWithBoth();
    std::ostringstream errors;

    const ExitCode code = subedit::cli::removeHearingImpairedIn(
        files,
        {"film.srt"},
        std::nullopt,
        Destination::from("", "out", false, 1).value(),
        Reporter{errors, 1},
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Position});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("out/film.fr.srt").value_or(""), ContainsSubstring("Bonjour."));
    CHECK_THAT(files.contentOf("out/film.fr.srt").value_or(""), !ContainsSubstring("Un bruit"));
    CHECK_FALSE(files.contentOf("out/film.srt").has_value());
}

TEST_CASE("a paired run whose translation is missing fails and writes nothing", "[cli][pairing]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kMain);
    std::ostringstream errors;

    const ExitCode code = subedit::cli::recaseIn(
        files,
        {"film.srt"},
        std::nullopt,
        subedit::core::LetterCase::Upper,
        std::nullopt,
        Destination::from("", "out", false, 1).value(),
        Reporter{errors, 1},
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Position});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("film.fr.srt"));
    CHECK_FALSE(files.contentOf("out/film.fr.srt").has_value());
}

TEST_CASE(
    "inspect with a translation says how the lines matched, and names a translation it cannot read",
    "[cli][pairing]") {
    const InMemoryFileSystem files = filesWithBoth();
    std::ostringstream errors;
    std::ostringstream out;

    CHECK(subedit::cli::inspectFile(
        files,
        "film.srt",
        {},
        out,
        Reporter{errors, 1},
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Number}));
    CHECK_THAT(out.str(), ContainsSubstring("matched by number"));
    CHECK_THAT(out.str(), ContainsSubstring("translation: 2 lines attached"));

    std::ostringstream failures;
    std::ostringstream nothing;
    CHECK_FALSE(subedit::cli::inspectFile(
        files,
        "film.srt",
        {},
        nothing,
        Reporter{failures, 1},
        Pairing{.translation = "absent.srt", .method = TranslationMethod::Position}));
    CHECK_THAT(failures.str(), ContainsSubstring("absent.srt: "));
    CHECK(nothing.str().empty());
}

TEST_CASE("the record of inspect with a translation carries the alignment", "[cli][pairing]") {
    const InMemoryFileSystem files = filesWithBoth();
    std::ostringstream errors;
    std::ostringstream out;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("inspect");

    CHECK(subedit::cli::inspectFile(
        files,
        "film.srt",
        {},
        out,
        reporter,
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Position}));

    CHECK_THAT(
        records.str(),
        ContainsSubstring("\"translation\":{\"file\":\"film.fr.srt\",\"method\":\"position\","
                          "\"attached\":2,\"born\":0,\"untranslated\":0,\"out_of_order\":0}"));
}

TEST_CASE("an encoding asked for reads the translation as well as the main file",
          "[cli][pairing]") {
    InMemoryFileSystem files = filesWithBoth();
    std::ostringstream errors;

    const ExitCode code = subedit::cli::removeHearingImpairedIn(
        files,
        {"film.srt"},
        subedit::core::Encoding::utf8(subedit::core::ByteOrderMark::Absent),
        Destination::from("", "out", false, 1).value(),
        Reporter{errors, 1},
        Pairing{.translation = "film.fr.srt", .method = TranslationMethod::Number});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("out/film.fr.srt").value_or(""), ContainsSubstring("Au revoir."));
}
