// The command line in the user's language — issue #663, spec 15 D3 and D4.
//
// What a person reads follows the language: the help, the narration, the errors.
// What a script reads does not: `--format json`, the exit codes, the names typed.
// The runs in a language start the translated copy of the binary, which finds the
// fixture catalogue `data/i18n/cli.po`, compiled as Russian, beside it — the way an
// installed binary finds its own (`cli_run.hpp`, `invokeIn`).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <utility>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::EndsWith;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;
using subedit::e2e::invokeIn;
using subedit::e2e::invokeInEveryLanguage;
using subedit::e2e::Locale;
using subedit::e2e::russian;
using subedit::e2e::Scratch;
using subedit::e2e::writeSrt;
using subedit::e2e::writeUnreadable;

namespace {

/// A usage error of ours: `shift` told where to write and not by how much.
const std::vector<std::string> kUsageError{"shift", "--output-dir", "out", "a.srt"};

/// What the fixture says for that error.
constexpr const char* kUsageErrorInRussian =
    "shift требует --by или --to-grid, чтобы вычислить величину по позициям\n";

} // namespace

TEST_CASE("the help of a subcommand follows the language, and the names typed do not",
          "[e2e][CLI-I18N-01]") {
    const CliRun run = invokeIn(russian(), {"shift", "--help"});

    CHECK(run.exitCode == 0);
    // Its summary and its descriptions, and the words CLI11 puts around them.
    CHECK_THAT(run.output, ContainsSubstring("Сдвинуть все позиции файла на постоянную величину"));
    CHECK_THAT(run.output,
               ContainsSubstring("Использование: subedit-cli shift [OPTIONS] files..."));
    CHECK_THAT(run.output, ContainsSubstring("Позиционные:"));
    CHECK_THAT(run.output, ContainsSubstring("ОБЯЗАТЕЛЬНО"));
    CHECK_THAT(run.output, ContainsSubstring("Параметры:"));
    CHECK_THAT(run.output,
               ContainsSubstring("-h,--help                   Показать эту справку и выйти"));
    CHECK_THAT(run.output,
               ContainsSubstring("--by TEXT                   Величина сдвига: 2.999, -7.001 или "
                                 "00:00:07.001"));
    // The options keep their names, and what the catalogue lacks stays English.
    CHECK_THAT(run.output,
               ContainsSubstring("--output-dir TEXT           Directory to write into"));
    CHECK_THAT(run.output, !ContainsSubstring("Options:"));
}

TEST_CASE("the help of the tool lists its subcommands under a translated heading",
          "[e2e][CLI-I18N-01]") {
    const CliRun run = invokeIn(russian(), {"--help"});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("Читать, проверять и сдвигать файлы субтитров."));
    CHECK_THAT(run.output, ContainsSubstring("Подкоманды:"));
    CHECK_THAT(run.output,
               ContainsSubstring("  shift                       Сдвинуть все позиции файла на "
                                 "постоянную величину"));
}

TEST_CASE("a usage error follows the language, and its exit code does not", "[e2e][CLI-I18N-01]") {
    const CliRun english = invoke(kUsageError);
    const CliRun run = invokeIn(russian(), kUsageError);

    CHECK(english.errors ==
          "shift needs --by, or --to-grid to work the amount out from the positions\n");
    CHECK(run.errors == kUsageErrorInRussian);
    CHECK(run.exitCode == english.exitCode);
    CHECK(run.output.empty());
}

TEST_CASE("a failure and the narration of a batch follow the language, in all three forms",
          "[e2e][CLI-I18N-01]") {
    const Scratch scratch;
    const std::string present = writeSrt(scratch, "in/a.srt", 2);

    // One file written and one absent: the line of each, then the summary.
    const CliRun pair = invokeIn(
        russian(),
        {"shift", "--by", "1", "--output-dir", scratch.of("out"), present, scratch.of("in/b.srt")});
    CHECK(pair.exitCode == 3);
    CHECK_THAT(pair.errors,
               ContainsSubstring(present + ": 2 субтитра сдвинуто на 1.000 s → " +
                                 scratch.of("out/a.srt")));
    CHECK_THAT(pair.errors, ContainsSubstring(scratch.of("in/b.srt") + " — не существует\n"));
    CHECK_THAT(pair.errors, EndsWith("сдвинуто 1 из 2 файлов (форма 2–4), 1 с ошибкой\n"));

    // Twenty-one files and five: the first form and the third. No form is an
    // English word with an « s » put after it.
    for (const auto& [count, summary] :
         std::vector<std::pair<int, std::string>>{{21, "сдвинут 0 из 21 файла, 21 с ошибкой\n"},
                                                  {5, "сдвинуто 0 из 5 файлов, 5 с ошибками\n"}}) {
        std::vector<std::string> arguments{"shift", "--by", "1", "--output-dir", scratch.of("out")};
        for (int at = 0; at < count; ++at)
            arguments.push_back(scratch.of("absent-" + std::to_string(at) + ".srt"));
        const CliRun run = invokeIn(russian(), arguments);
        CHECK(run.exitCode == 2);
        CHECK_THAT(run.errors, EndsWith(summary));
    }
}

TEST_CASE("the language is read from LANGUAGE, LC_ALL, LC_MESSAGES and LANG, in that order",
          "[e2e][CLI-I18N-01]") {
    // Each of the three locale variables, alone, names the language.
    for (const Locale& locale : {Locale{.all = "ru_RU.UTF-8"},
                                 Locale{.messages = "ru_RU.UTF-8"},
                                 Locale{.lang = "ru_RU.UTF-8"}}) {
        CHECK(invokeIn(locale, kUsageError).errors == kUsageErrorInRussian);
    }

    // LANGUAGE comes first, and its list is walked past what is not installed.
    CHECK(invokeIn(Locale{.language = "de:ru", .lang = "de_DE.UTF-8"}, kUsageError).errors ==
          kUsageErrorInRussian);
    // LC_ALL before LC_MESSAGES before LANG: the first one set decides.
    CHECK(invokeIn(Locale{.messages = "ru_RU.UTF-8", .lang = "de_DE.UTF-8"}, kUsageError).errors ==
          kUsageErrorInRussian);
    const std::string english = invoke(kUsageError).errors;
    CHECK(invokeIn(Locale{.all = "de_DE.UTF-8", .lang = "ru_RU.UTF-8"}, kUsageError).errors ==
          english);
    // An entry naming English stops the list: what comes after it never applies.
    CHECK(invokeIn(Locale{.language = "en:ru", .lang = "ru_RU.UTF-8"}, kUsageError).errors ==
          english);
}

TEST_CASE("a language that is not installed is English, and nothing is said about it",
          "[e2e][CLI-I18N-01]") {
    const CliRun english = invoke({"shift", "--help"});

    for (const Locale& locale :
         {Locale{.all = "xx_YY.UTF-8"}, Locale{.language = "xx:yy", .lang = "zz_ZZ"}, Locale{}}) {
        const CliRun run = invokeIn(locale, {"shift", "--help"});
        CHECK(run.exitCode == 0);
        CHECK(run.output == english.output);
        CHECK(run.errors.empty());
    }
}

TEST_CASE("LC_ALL=C is English, whatever LANGUAGE asks for", "[e2e][CLI-I18N-03]") {
    const Locale c{.language = "ru", .all = "C", .lang = "ru_RU.UTF-8"};

    const CliRun help = invokeIn(c, {"shift", "--help"});
    CHECK(help.output == invoke({"shift", "--help"}).output);

    const CliRun error = invokeIn(c, kUsageError);
    CHECK(error.errors == invoke(kUsageError).errors);

    // LANGUAGE alone, with no locale at all, is the C locale too.
    CHECK(invokeIn(Locale{.language = "ru"}, kUsageError).errors == invoke(kUsageError).errors);
}

TEST_CASE("CLI11's own diagnostics stay English, as the manual says", "[e2e]") {
    const CliRun english = invoke({"shift", "--by"});
    const CliRun run = invokeIn(russian(), {"shift", "--by"});

    CHECK(run.errors == english.errors);
    CHECK(run.exitCode == english.exitCode);
}

TEST_CASE("the output of --format json is the same bytes in every language, failures included",
          "[e2e][CLI-I18N-02]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");
    const std::vector<std::string> arguments{"--format",
                                             "json",
                                             "shift",
                                             "--by",
                                             "1",
                                             "--output-dir",
                                             scratch.of("out"),
                                             good,
                                             scratch.of("in/absent.srt"),
                                             broken};

    // The records are compared by the runner; the words of the failures are English.
    const CliRun english = invokeInEveryLanguage(arguments);
    CHECK_THAT(english.output,
               ContainsSubstring(R"("message":")" + scratch.of("in/absent.srt") +
                                 R"(: does not exist"})"));
    // While the same failures, said to a person, are not.
    const CliRun run = invokeIn(russian(), arguments);
    CHECK_THAT(run.errors, ContainsSubstring(scratch.of("in/absent.srt") + " — не существует"));
    CHECK_THAT(run.errors, ContainsSubstring(broken + " — не в известном программе формате"));
}
