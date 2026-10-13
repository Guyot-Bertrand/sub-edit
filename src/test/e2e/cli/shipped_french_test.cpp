// The command line in the French that ships — issue #668. The other language tests run a copy
// of the binary beside a fixture; these run the build tree's own, beside the catalogue the build
// compiles from `src/po/fr.po` and installs, as a French user's installed program runs.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::EndsWith;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;
using subedit::e2e::invokeShipped;
using subedit::e2e::Locale;
using subedit::e2e::Scratch;
using subedit::e2e::writeSrt;

namespace {

/// A French user: `LANGUAGE` as a desktop sets it, and the locale.
const Locale kFrench{.language = "fr", .lang = "fr_FR.UTF-8"};

/// A usage error of ours: `shift` told where to write and not by how much.
const std::vector<std::string> kUsageError{"shift", "--output-dir", "out", "a.srt"};

} // namespace

TEST_CASE("the help is in the French that ships, and the names typed are not",
          "[e2e][CLI-I18N-01]") {
    const CliRun run = invokeShipped(kFrench, {"--help"});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output,
               ContainsSubstring("Lire, inspecter et recaler des fichiers de sous-titres."));
    CHECK_THAT(run.output, ContainsSubstring("Sous-commandes:"));
    CHECK_THAT(run.output,
               ContainsSubstring("  shift                       Déplacer chaque position d'un "
                                 "fichier d'une quantité fixe"));
    CHECK_THAT(run.output,
               ContainsSubstring("-h,--help                   Afficher cette aide et quitter"));
    CHECK_THAT(run.output, !ContainsSubstring("Subcommands:"));
}

TEST_CASE("a usage error, a failure and a summary are in the French that ships",
          "[e2e][CLI-I18N-01]") {
    const CliRun usage = invokeShipped(kFrench, kUsageError);
    CHECK(usage.errors ==
          "shift demande --by, ou --to-grid pour calculer la quantité à partir des positions\n");
    CHECK(usage.exitCode == invoke(kUsageError).exitCode);

    // The locale alone names the language, with no LANGUAGE at all.
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
}

TEST_CASE("with French installed, English is what LC_ALL=C or LANGUAGE=en asks for",
          "[e2e][CLI-I18N-01][CLI-I18N-03]") {
    const CliRun english = invoke({"shift", "--help"});
    REQUIRE_THAT(english.output, ContainsSubstring("Options:"));

    for (const Locale& locale : {Locale{.language = "fr", .all = "C", .lang = "fr_FR.UTF-8"},
                                 Locale{.language = "en:fr", .lang = "fr_FR.UTF-8"}}) {
        const CliRun run = invokeShipped(locale, {"shift", "--help"});
        CHECK(run.output == english.output);
        CHECK(invokeShipped(locale, kUsageError).errors == invoke(kUsageError).errors);
    }
}

TEST_CASE("the output of --format json is the same bytes in the French that ships",
          "[e2e][CLI-I18N-02]") {
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
    const CliRun french = invokeShipped(kFrench, arguments);

    CHECK(french.exitCode == english.exitCode);
    CHECK(french.output == english.output);
    CHECK_THAT(english.output, ContainsSubstring(": does not exist"));
    // While the same failure, said to a person, is French.
    CHECK_THAT(french.errors, ContainsSubstring(scratch.of("in/absent.srt") + " : n'existe pas"));
}
