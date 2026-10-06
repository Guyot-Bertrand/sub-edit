// The three rewrites of the texts, on an in-memory file system.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/text_rewriting.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::dialogueDashesIn;
using subedit::cli::ExitCode;
using subedit::cli::italicsIn;
using subedit::cli::Range;
using subedit::cli::recaseIn;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;
using subedit::core::LetterCase;

namespace {

const std::string kFile = "1\n00:00:01,000 --> 00:00:02,000\nbonjour marie\n\n"
                          "2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n";

const std::string kTmPlayer = "00:00:01:Hello there\n00:00:03:Bye\n";

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
    std::string records;
};

template<typename Rewrite>
Run run(const std::string& content, const std::string& name, Rewrite rewrite, bool dry = false) {
    InMemoryFileSystem files;
    files.addFile(name, content);
    std::ostringstream errors;
    std::ostringstream records;
    const Reporter reporter = Reporter{errors, 1}.withRecords(records).forCommand("test");
    const Destination destination = Destination::from("", dry ? "" : "out", false, 1, dry).value();

    const ExitCode code = rewrite(files, name, destination, reporter);
    return {.code = code,
            .written = files.contentOf("out/" + name).value_or(""),
            .errors = errors.str(),
            .records = records.str()};
}

} // namespace

TEST_CASE("a recase writes the texts in the case asked for and counts them",
          "[cli][text-rewriting][CLI-CASE-01]") {
    const Run done = run(kFile, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
        return recaseIn(files, {name}, std::nullopt, LetterCase::Upper, std::nullopt, dest, rep);
    });

    CHECK(done.code == ExitCode::Success);
    CHECK_THAT(done.written, ContainsSubstring("BONJOUR MARIE"));
    CHECK_THAT(done.written, ContainsSubstring("BONJOUR\nMARIE"));
    CHECK_THAT(done.errors, ContainsSubstring("2 subtitles recased"));
    CHECK_THAT(done.records, ContainsSubstring("\"counts\":{\"changed\":2}"));
}

TEST_CASE("a recase over a range changes only that range", "[cli][text-rewriting][CLI-CASE-01]") {
    const Run done = run(kFile, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
        return recaseIn(files,
                        {name},
                        std::nullopt,
                        LetterCase::Upper,
                        Range{.first = 2, .last = 2},
                        dest,
                        rep);
    });

    CHECK_THAT(done.written, ContainsSubstring("bonjour marie"));
    CHECK_THAT(done.written, ContainsSubstring("BONJOUR\nMARIE"));
    CHECK_THAT(done.errors, ContainsSubstring("1 subtitle recased"));
}

TEST_CASE("a recase that changes no text is nothing to change",
          "[cli][text-rewriting][CLI-CASE-01]") {
    const Run done = run(kFile, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
        return recaseIn(files, {name}, std::nullopt, LetterCase::Title, std::nullopt, dest, rep);
    });

    // The first text changes, the second is already in title case; run it again.
    CHECK_THAT(done.written, ContainsSubstring("Bonjour Marie"));

    const Run again =
        run(done.written, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
            return recaseIn(
                files, {name}, std::nullopt, LetterCase::Title, std::nullopt, dest, rep);
        });
    CHECK_THAT(again.errors, ContainsSubstring("nothing to change"));
    CHECK_THAT(again.records, ContainsSubstring("\"counts\":{\"changed\":0},\"changes\":[]"));
}

TEST_CASE("italics goes one way or the other, as told", "[cli][text-rewriting][CLI-ITALIC-01]") {
    const Run on = run(kFile, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
        return italicsIn(files, {name}, std::nullopt, true, std::nullopt, dest, rep);
    });
    CHECK_THAT(on.written, ContainsSubstring("<i>bonjour marie</i>"));
    CHECK_THAT(on.errors, ContainsSubstring("2 subtitles put in italics"));

    const Run off =
        run(on.written, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
            return italicsIn(files, {name}, std::nullopt, false, std::nullopt, dest, rep);
        });
    CHECK(off.written == kFile);
    CHECK_THAT(off.errors, ContainsSubstring("2 subtitles taken out of italics"));
}

TEST_CASE("a format that writes no style is refused, with the reason and a kind",
          "[cli][text-rewriting][CLI-ITALIC-02]") {
    for (const bool italic : {true, false}) {
        const Run done =
            run(kTmPlayer, "a.txt", [italic](auto& files, const auto& name, auto& dest, auto& rep) {
                return italicsIn(files, {name}, std::nullopt, italic, std::nullopt, dest, rep);
            });

        CHECK(done.code == ExitCode::AllFailed);
        CHECK_THAT(done.errors,
                   ContainsSubstring("TMPlayer writes no style: there are no italics to "));
        CHECK_THAT(done.records, ContainsSubstring("\"kind\":\"no-style\""));
        CHECK(done.written.empty());
    }
}

TEST_CASE("dashes go on and come off, as told", "[cli][text-rewriting][CLI-DASH-01]") {
    const Run add = run(kFile, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
        return dialogueDashesIn(files, {name}, std::nullopt, true, std::nullopt, dest, rep);
    });
    CHECK_THAT(add.written, ContainsSubstring("- bonjour marie"));
    CHECK_THAT(add.written, ContainsSubstring("- Bonjour\n- Marie"));
    CHECK_THAT(add.errors, ContainsSubstring("2 subtitles dashed"));

    const Run remove =
        run(add.written, "a.srt", [](auto& files, const auto& name, auto& dest, auto& rep) {
            return dialogueDashesIn(files, {name}, std::nullopt, false, std::nullopt, dest, rep);
        });
    CHECK(remove.written == kFile);
    CHECK_THAT(remove.errors, ContainsSubstring("2 subtitles undashed"));
}

TEST_CASE("a dry run lists the changes and writes nothing",
          "[cli][text-rewriting][CLI-DRYRUN-04]") {
    const Run done = run(
        kFile,
        "a.srt",
        [](auto& files, const auto& name, auto& dest, auto& rep) {
            return dialogueDashesIn(files, {name}, std::nullopt, true, std::nullopt, dest, rep);
        },
        true);

    CHECK(done.written.empty());
    CHECK_THAT(done.errors, ContainsSubstring("(dry run, nothing written)"));
    CHECK_THAT(done.records, ContainsSubstring("\"changes\":[{\"subtitle\":1,"));
}

TEST_CASE("the words of the text subcommands name one way, and two are refused",
          "[cli][text-rewriting][CLI-ITALIC-01][CLI-DASH-01]") {
    CHECK(subedit::cli::letterCaseNamed("title") == subedit::core::LetterCase::Title);
    CHECK(subedit::cli::letterCaseNamed("sentence") == subedit::core::LetterCase::Sentence);
    CHECK(subedit::cli::letterCaseNamed("upper") == subedit::core::LetterCase::Upper);
    CHECK(subedit::cli::letterCaseNamed("lower") == subedit::core::LetterCase::Lower);

    CHECK(subedit::cli::italicsDirectionOf(true, false).value());
    CHECK_FALSE(subedit::cli::italicsDirectionOf(false, true).value());
    CHECK_FALSE(subedit::cli::italicsDirectionOf(true, true).has_value());
    CHECK_FALSE(subedit::cli::italicsDirectionOf(false, false).has_value());

    CHECK(subedit::cli::dashesDirectionOf(true, false).value());
    CHECK_FALSE(subedit::cli::dashesDirectionOf(false, true).value());
    CHECK_FALSE(subedit::cli::dashesDirectionOf(true, true).has_value());
    CHECK(subedit::cli::dashesDirectionOf(false, false).error().find("needs --add") !=
          std::string::npos);
}
