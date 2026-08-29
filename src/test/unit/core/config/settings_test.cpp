// La configuration persistée — issue #240, décision de l'ADR 0022.
//
// **Les six cas du tableau de l'ADR ont chacun leur cas de test**, et ils sont
// le cœur du ticket : ce qui est décidé n'est pas ce que la configuration
// retient, mais ce qu'elle fait quand le fichier ne dit pas ce qu'on attend.
// Une configuration est un confort ; sa défaillance doit coûter le confort et
// rien d'autre.
//
// Tout passe par `InMemoryFileSystem` : aucun de ces cas ne touche un
// emplacement réel, et c'est la couture de l'ADR qui le garantit — la
// configuration reçoit son chemin plutôt que d'en chercher un.
//
// **Aucun tag d'exigence ici**, et pour la raison de #154 :
// `check-requirements.sh` lit les binaires d'interface et de bout en bout, donc
// un `GUI-CONFIG-0N` écrit dans un test de noyau lui serait invisible —
// décoration plutôt que traçabilité. Les trois promesses sont citées là où la
// fenêtre est éprouvée, dans `window_settings_test.cpp`.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::readSettings;
using subedit::core::renderSettings;
using subedit::core::Settings;
using subedit::core::SettingsRead;
using subedit::core::WindowGeometry;
using subedit::core::writeSettings;

constexpr const char* kPath = "/config/subedit/settings.conf";

[[nodiscard]] SettingsRead readOf(const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(kPath, content);
    return readSettings(files, kPath);
}

/// Une configuration dont rien n'est au défaut, pour les cas qui ont besoin
/// d'un aller-retour.
[[nodiscard]] Settings chosen() {
    return Settings{.geometry = WindowGeometry{.x = 40, .y = 60, .width = 1440, .height = 900},
                    .maximised = true,
                    .columnWidths = {50, 120, 120, 120}};
}

} // namespace

// ## Fichier absent : tous les défauts, et ce n'est pas une erreur

TEST_CASE("un fichier absent donne tous les défauts, sans rien signaler", "[config]") {
    const InMemoryFileSystem files;

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
    // Le premier lancement n'a rien à signaler : il n'y a pas de réglage perdu.
    CHECK_FALSE(read.unreadable.has_value());
}

// ## Fichier illisible : tous les défauts, et un diagnostic

TEST_CASE("un fichier illisible donne tous les défauts, et le dit", "[config]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.maximised = true\n");
    files.failNextRead(FileErrorKind::PermissionDenied);

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    REQUIRE(read.unreadable.has_value());
    // `value_or` plutôt qu'un déréférencement : l'analyse statique ne lit pas
    // le `REQUIRE` ci-dessus comme une garde, et le défaut d'un `FileError` est
    // `NotFound`, que la comparaison refuse.
    CHECK(read.unreadable.value_or(FileError{}).kind == FileErrorKind::PermissionDenied);
}

// ## Clé inconnue : ignorée à la lecture, absente à la réécriture

TEST_CASE("une clé inconnue est ignorée sans un mot", "[config]") {
    // Un fichier écrit par une version qui en connaissait plus n'est pas un
    // fichier fautif : c'est le mode d'échec que la tolérance choisit.
    const SettingsRead read = readOf("window.maximised = true\n"
                                     "window.opacity = 0.5\n"
                                     "table.font = DejaVu Sans\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("une clé inconnue n'est pas réécrite", "[config]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.opacity = 0.5\n");

    const SettingsRead read = readSettings(files, kPath);
    REQUIRE(writeSettings(files, kPath, read.settings).has_value());

    CHECK(files.contentOf(kPath).value_or("").find("opacity") == std::string::npos);
}

// ## Valeur illisible : le défaut est gardé, un diagnostic est produit

TEST_CASE("une valeur illisible laisse le défaut en place, et le dit", "[config]") {
    const SettingsRead read = readOf("window.geometry = plus tard\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics.front().key == "window.geometry");
    CHECK(read.diagnostics.front().value == "plus tard");
}

TEST_CASE("une valeur illisible n'emporte pas les autres options", "[config]") {
    // Le diagnostic accompagne le défaut plutôt que de remplacer le fichier :
    // ce qui se lisait se lit encore.
    const SettingsRead read = readOf("window.geometry = 0,0,0,0\n"
                                     "window.maximised = true\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.size() == 1);
}

TEST_CASE("les largeurs de colonnes se refusent en nombre autre que quatre", "[config]") {
    // Trois largeurs pour quatre colonnes réglables : deviner laquelle manque
    // serait deviner ce que l'utilisateur voulait.
    CHECK(readOf("table.columns = 50,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120,700\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120\n").diagnostics.empty());
}

TEST_CASE("les deux valeurs booléennes se lisent, et elles seules", "[config]") {
    // `false` autant que `true` : c'est le défaut, donc personne ne l'écrit
    // spontanément — et une valeur qu'aucun test ne lit est une valeur dont on
    // ne sait pas si elle se lit.
    CHECK(readOf("window.maximised = true\n").settings.maximised);
    CHECK_FALSE(readOf("window.maximised = false\n").settings.maximised);
    CHECK(readOf("window.maximised = false\n").diagnostics.empty());
}

TEST_CASE("une largeur de colonne nulle ou négative est illisible", "[config]") {
    // Une colonne large de zéro est une colonne qu'on ne retrouverait pas, et
    // une largeur négative n'existe pas : ni l'une ni l'autre n'est une
    // largeur que l'utilisateur a posée.
    CHECK(readOf("table.columns = 50,0,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,-10,120,120\n").diagnostics.size() == 1);
}

TEST_CASE("une ligne sans signe égal est ignorée", "[config]") {
    // Ce n'est pas une option, donc ce n'est pas une option illisible : rien
    // n'est signalé, et le reste du fichier se lit.
    const SettingsRead read = readOf("ceci n'est pas une option\n"
                                     "window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("ce qui traîne après un nombre le rend illisible", "[config]") {
    // « 12 pixels » n'est pas un douze suivi de bruit : c'est une valeur qu'on
    // n'a pas su lire, et l'accepter serait accepter n'importe quoi.
    CHECK(readOf("window.maximised = oui\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120 px\n").diagnostics.size() == 1);
}

// ## Option absente : le défaut, sans que ce soit un cas particulier

TEST_CASE("une option absente vaut son défaut", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.columnWidths.empty());
    CHECK(read.diagnostics.empty());
}

// ## Option à son défaut : réécrite commentée

TEST_CASE("une option à son défaut est réécrite commentée", "[config]") {
    const std::string written = renderSettings(Settings{});

    CHECK_THAT(written, ContainsSubstring("#window.geometry = "));
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
}

TEST_CASE("une option réglée est réécrite nue", "[config]") {
    const std::string written = renderSettings(chosen());

    CHECK_THAT(written, ContainsSubstring("\nwindow.geometry = 40,60,1440,900\n"));
    CHECK_THAT(written, ContainsSubstring("\nwindow.maximised = true\n"));
    CHECK_THAT(written, ContainsSubstring("\ntable.columns = 50,120,120,120\n"));
}

// **Ce que la réécriture commentée achète**, et la raison pour laquelle elle
// n'est pas une coquetterie : une option jamais touchée n'est pas figée à la
// valeur du jour où elle a été écrite. Relire ce qu'on vient d'écrire redonne
// donc les défauts, et non des valeurs gelées.
TEST_CASE("une option à son défaut se relit au défaut après réécriture", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, Settings{}).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
}

TEST_CASE("ce qui a été réglé se relit tel quel", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, chosen()).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == chosen());
    CHECK(read.diagnostics.empty());
}

// ## Ce que l'écriture demande au système de fichiers

TEST_CASE("l'écriture fait le répertoire que personne n'a fait", "[config]") {
    // Au premier lancement, `~/.config/subedit` n'existe pas.
    InMemoryFileSystem files;

    CHECK(writeSettings(files, kPath, chosen()).has_value());
    CHECK(files.contentOf(kPath).has_value());
}

TEST_CASE("une écriture refusée est rendue, et n'arrête rien", "[config]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    CHECK_FALSE(writeSettings(files, kPath, chosen()).has_value());
}

TEST_CASE("le fichier écrit s'explique de lui-même", "[config]") {
    // Il est fait pour être ouvert dans un éditeur : sans ces lignes, un
    // lecteur qui tombe sur des options commentées croit à des restes.
    const std::string written = renderSettings(Settings{});

    CHECK(written.starts_with("# subedit settings."));
    CHECK_THAT(written, ContainsSubstring("commented out"));
}
