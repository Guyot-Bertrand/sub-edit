// The command line in every language that ships — issues #668 and #720. The other language tests
// run a copy of the binary beside a fixture; these run the build tree's own, beside the catalogues
// the build compiles from `src/po/<language>.po` and installs, as an installed program runs.
//
// **The languages are those of `src/po/LINGUAS`**, handed to this file alone by its CMakeLists
// (`SUBEDIT_SHIPPED_LANGUAGES`): a language added there is tested here with no line to write. What
// is asked of every language holds for any of them — its words are looked up in its own compiled
// catalogue, through the project's reader — and French keeps a case of its own, which names the
// words.

#include <subedit/core/i18n/catalogue.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::EndsWith;
using subedit::core::Catalogue;
using subedit::core::substitute;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;
using subedit::e2e::invokeShipped;
using subedit::e2e::Locale;
using subedit::e2e::Scratch;
using subedit::e2e::writeSrt;

namespace {

/// The lines of `src/po/LINGUAS`, comma-separated by the build.
[[nodiscard]] std::vector<std::string> shippedLanguages() {
    std::vector<std::string> languages;
    const std::string_view list{SUBEDIT_SHIPPED_LANGUAGES};
    std::size_t start = 0;
    while (start < list.size()) {
        std::size_t end = list.find(',', start);
        if (end == std::string_view::npos)
            end = list.size();
        if (end > start)
            languages.emplace_back(list.substr(start, end - start));
        start = end + 1;
    }
    return languages;
}

/// A user of `language`: `LANGUAGE` as a desktop sets it, and the locale.
[[nodiscard]] Locale userOf(const std::string& language) {
    return Locale{.language = language, .lang = language + ".UTF-8"};
}

/// The words of a shipped catalogue, the English text when it has none — as the program says
/// them: the test asks the catalogue what a message should become.
class Words {
public:
    explicit Words(const std::string& language) {
        auto read = Catalogue::fromFile(std::filesystem::path{SUBEDIT_SHIPPED_LOCALE_DIR} /
                                        language / "LC_MESSAGES" / "subedit.mo");
        REQUIRE(read.has_value());
        m_catalogue = *std::move(read);
    }

    [[nodiscard]] std::string of(std::string_view text) const {
        return std::string{m_catalogue.find({}, text).value_or(text)};
    }

    [[nodiscard]] std::string
    of(std::string_view singular, std::string_view plural, std::uint64_t count) const {
        return std::string{
            m_catalogue.findPlural(singular, count).value_or(count == 1 ? singular : plural)};
    }

private:
    Catalogue m_catalogue;
};

/// A French user, for the case that names the French words.
const Locale kFrench{.language = "fr", .lang = "fr_FR.UTF-8"};

/// A usage error of ours: `shift` told where to write and not by how much.
const std::vector<std::string> kUsageError{"shift", "--output-dir", "out", "a.srt"};
constexpr std::string_view kUsageMessage =
    "shift needs --by, or --to-grid to work the amount out from the positions";

} // namespace

TEST_CASE("the help is in every language that ships, and the names typed are not",
          "[e2e][CLI-I18N-01]") {
    const std::string language = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << language) {
        const Words words{language};
        const CliRun english = invoke({"--help"});
        const CliRun run = invokeShipped(userOf(language), {"--help"});

        CHECK(run.exitCode == 0);
        CHECK(run.output != english.output);
        CHECK_THAT(run.output,
                   ContainsSubstring(words.of("Read, inspect and retime subtitle files.")));
        CHECK_THAT(run.output, ContainsSubstring(words.of("Subcommands") + ":"));
        CHECK_THAT(run.output, ContainsSubstring(words.of("Print this help message and exit")));
        // The names typed are the same in every language.
        CHECK_THAT(run.output, ContainsSubstring("-h,--help"));
        CHECK_THAT(run.output, ContainsSubstring("  shift "));
        if (words.of("Subcommands") != "Subcommands")
            CHECK_THAT(run.output, !ContainsSubstring("Subcommands:"));
    }
}

TEST_CASE("a usage error, a failure and a summary are in every language that ships",
          "[e2e][CLI-I18N-01]") {
    const std::string language = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << language) {
        const Words words{language};
        const CliRun usage = invokeShipped(userOf(language), kUsageError);
        CHECK(usage.errors == words.of(kUsageMessage) + "\n");
        CHECK(usage.errors != invoke(kUsageError).errors);
        CHECK(usage.exitCode == invoke(kUsageError).exitCode);

        // The locale alone names the language, with no LANGUAGE at all.
        CHECK(invokeShipped(Locale{.lang = language + ".UTF-8"}, kUsageError).errors ==
              usage.errors);

        const Scratch scratch;
        const std::string present = writeSrt(scratch, "in/a.srt", 2);
        const std::string absent = scratch.of("in/b.srt");
        const CliRun batch = invokeShipped(
            userOf(language),
            {"shift", "--by", "1", "--output-dir", scratch.of("out"), present, absent});
        CHECK(batch.exitCode == 3);

        // What was done, in the words of the language around the numbers it was given.
        const std::string done = substitute(
            words.of("%1 subtitle shifted by %2", "%1 subtitles shifted by %2", 2), {"2", "%2"});
        const std::size_t duration = done.find("%2");
        REQUIRE(duration != std::string::npos);
        CHECK_THAT(batch.errors, ContainsSubstring(done.substr(0, duration)));
        CHECK_THAT(batch.errors, ContainsSubstring(done.substr(duration + 2)));

        CHECK_THAT(
            batch.errors,
            ContainsSubstring(substitute(words.of("%1: %2"), {absent, words.of("does not exist")}) +
                              "\n"));
        const std::string summary =
            substitute(words.of("%1 of %2 file shifted", "%1 of %2 files shifted", 2), {"1", "2"});
        CHECK_THAT(
            batch.errors,
            EndsWith(substitute(words.of("%1, %2 failed", "%1, %2 failed", 1), {summary, "1"}) +
                     "\n"));
    }
}

TEST_CASE("with a language installed, English is what LC_ALL=C or LANGUAGE=en asks for",
          "[e2e][CLI-I18N-01][CLI-I18N-03]") {
    const CliRun english = invoke({"shift", "--help"});
    REQUIRE_THAT(english.output, ContainsSubstring("Options:"));

    const std::string language = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << language) {
        const std::string locale = language + ".UTF-8";
        for (const Locale& asked : {Locale{.language = language, .all = "C", .lang = locale},
                                    Locale{.language = "en:" + language, .lang = locale}}) {
            const CliRun run = invokeShipped(asked, {"shift", "--help"});
            CHECK(run.output == english.output);
            CHECK(invokeShipped(asked, kUsageError).errors == invoke(kUsageError).errors);
        }
    }
}

TEST_CASE("the output of --format json is the same bytes in every language that ships",
          "[e2e][CLI-I18N-02]") {
    const std::string language = GENERATE(from_range(shippedLanguages()));
    DYNAMIC_SECTION("language " << language) {
        const Words words{language};
        const Scratch scratch;
        const std::string present = writeSrt(scratch, "in/a.srt", 2);
        // A dry run, so that the second run finds nothing the first one wrote.
        const std::vector<std::string> arguments{"--format",
                                                 "json",
                                                 "shift",
                                                 "--by",
                                                 "1",
                                                 "--dry-run",
                                                 "--output-dir",
                                                 scratch.of("out"),
                                                 present,
                                                 scratch.of("in/absent.srt")};

        const CliRun english = invoke(arguments);
        const CliRun translated = invokeShipped(userOf(language), arguments);

        CHECK(translated.exitCode == english.exitCode);
        CHECK(translated.output == english.output);
        CHECK_THAT(english.output, ContainsSubstring(": does not exist"));
        // While the same failure, said to a person, is in the language.
        CHECK_THAT(
            translated.errors,
            ContainsSubstring(substitute(
                words.of("%1: %2"), {scratch.of("in/absent.srt"), words.of("does not exist")})));
    }
}

TEST_CASE("the help, a usage error and a batch say the French words the catalogue ships",
          "[e2e][CLI-I18N-01]") {
    const CliRun help = invokeShipped(kFrench, {"--help"});
    CHECK(help.exitCode == 0);
    CHECK_THAT(help.output,
               ContainsSubstring("Lire, inspecter et recaler des fichiers de sous-titres."));
    CHECK_THAT(help.output, ContainsSubstring("Sous-commandes:"));
    CHECK_THAT(help.output,
               ContainsSubstring("  shift                       Déplacer chaque position d'un "
                                 "fichier d'une quantité fixe"));
    CHECK_THAT(help.output,
               ContainsSubstring("-h,--help                   Afficher cette aide et quitter"));
    CHECK_THAT(help.output, !ContainsSubstring("Subcommands:"));

    const CliRun usage = invokeShipped(kFrench, kUsageError);
    CHECK(usage.errors ==
          "shift demande --by, ou --to-grid pour calculer la quantité à partir des positions\n");
    CHECK(invokeShipped(Locale{.lang = "fr_FR.UTF-8"}, kUsageError).errors == usage.errors);

    const Scratch scratch;
    const std::string present = writeSrt(scratch, "in/a.srt", 2);
    const CliRun batch = invokeShipped(
        kFrench,
        {"shift", "--by", "1", "--output-dir", scratch.of("out"), present, scratch.of("in/b.srt")});
    CHECK(batch.exitCode == 3);
    CHECK_THAT(batch.errors,
               ContainsSubstring(present + " : 2 sous-titres décalés de 1.000 s -> " +
                                 scratch.of("out/a.srt")));
    CHECK_THAT(batch.errors, ContainsSubstring(scratch.of("in/b.srt") + " : n'existe pas\n"));
    CHECK_THAT(batch.errors, EndsWith("1 sur 2 fichiers décalés, 1 en échec\n"));

    const CliRun json = invokeShipped(kFrench,
                                      {"--format",
                                       "json",
                                       "shift",
                                       "--by",
                                       "1",
                                       "--dry-run",
                                       "--output-dir",
                                       scratch.of("out"),
                                       scratch.of("in/absent.srt")});
    CHECK_THAT(json.errors, ContainsSubstring(scratch.of("in/absent.srt") + " : n'existe pas"));
}
