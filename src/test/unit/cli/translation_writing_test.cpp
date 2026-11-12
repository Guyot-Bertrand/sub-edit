// The translation of a pair written at the positions of its main file, on an
// in-memory file system.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/translation_writing.hpp>
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
using subedit::cli::writeTranslationAt;
using subedit::core::InMemoryFileSystem;

TEST_CASE("the translation is written at the positions of the main file, and the alignment is "
          "the sentence",
          "[cli][translation-writing][CLI-PAIR-01]") {
    InMemoryFileSystem files;
    files.addFile("main.srt", "1\n00:00:01,000 --> 00:00:02,000\none\n\n");
    // Late by half a second: by position the line still finds the subtitle.
    files.addFile("fr.srt", "1\n00:00:01,500 --> 00:00:02,500\nun\n\n");
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code =
        writeTranslationAt(files,
                           "main.srt",
                           std::nullopt,
                           Destination::from("out/fr.srt", "", false, 1).value(),
                           Reporter{errors, 1}.withRecords(records).forCommand("pair"),
                           Pairing{.translation = "fr.srt"});

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/fr.srt").value_or("") == "1\n00:00:01,000 --> 00:00:02,000\nun\n\n");
    CHECK_THAT(errors.str(),
               ContainsSubstring("[1/1] source: main.srt\n"
                                 "[subedit] [pair] [1/1] translation: fr.srt\n"
                                 "[subedit] [pair] [1/1] result: translation: 1 line attached\n"
                                 "[subedit] [pair] [1/1] dest: out/fr.srt\n"));
    CHECK_THAT(records.str(), ContainsSubstring("\"counts\":{\"subtitles\":1}"));
    CHECK_THAT(records.str(), ContainsSubstring("\"attached\":1"));
}

TEST_CASE("a translation that cannot be read is named in the failure, the main file being the "
          "source",
          "[cli][translation-writing][CLI-PAIR-01]") {
    InMemoryFileSystem files;
    files.addFile("main.srt", "1\n00:00:01,000 --> 00:00:02,000\none\n\n");
    std::ostringstream errors;

    const ExitCode code = writeTranslationAt(files,
                                             "main.srt",
                                             std::nullopt,
                                             Destination::from("out/fr.srt", "", false, 1).value(),
                                             Reporter{errors, 0}.forCommand("pair"),
                                             Pairing{.translation = "fr.srt"});

    CHECK(code == ExitCode::AllFailed);
    CHECK(errors.str() == "[subedit] [pair] [1/1] source: main.srt\n"
                          "[subedit] [pair] [1/1] error: fr.srt: does not exist\n");
}
