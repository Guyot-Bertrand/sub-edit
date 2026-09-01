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

#include <filesystem>
#include <string>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::InsertPlacement;
using subedit::core::readSettings;
using subedit::core::renderSettings;
using subedit::core::Settings;
using subedit::core::SettingsRead;
using subedit::core::Theme;
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
                    .columnWidths = {50, 120, 120, 120},
                    .tableShare = 63,
                    .lastDirectory = std::filesystem::path{"/films/quai"},
                    .theme = Theme::Dark,
                    .insertPlacement = InsertPlacement::Above};
}

} // namespace

// ## Fichier absent : tous les défauts, et ce n'est pas une erreur

TEST_CASE("a missing file gives every default, reporting nothing", "[config]") {
    const InMemoryFileSystem files;

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
    // Le premier lancement n'a rien à signaler : il n'y a pas de réglage perdu.
    CHECK_FALSE(read.unreadable.has_value());
}

// ## Fichier illisible : tous les défauts, et un diagnostic

TEST_CASE("an unreadable file gives every default, and says so", "[config]") {
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

TEST_CASE("an unknown key is ignored without a word", "[config]") {
    // Un fichier écrit par une version qui en connaissait plus n'est pas un
    // fichier fautif : c'est le mode d'échec que la tolérance choisit.
    const SettingsRead read = readOf("window.maximised = true\n"
                                     "window.opacity = 0.5\n"
                                     "table.font = DejaVu Sans\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("an unknown key is not written back", "[config]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.opacity = 0.5\n");

    const SettingsRead read = readSettings(files, kPath);
    REQUIRE(writeSettings(files, kPath, read.settings).has_value());

    CHECK(files.contentOf(kPath).value_or("").find("opacity") == std::string::npos);
}

// ## Valeur illisible : le défaut est gardé, un diagnostic est produit

TEST_CASE("an unreadable value leaves the default in place, and says so", "[config]") {
    const SettingsRead read = readOf("window.geometry = plus tard\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics.front().key == "window.geometry");
    CHECK(read.diagnostics.front().value == "plus tard");
}

TEST_CASE("an unreadable value does not carry off the other options", "[config]") {
    // Le diagnostic accompagne le défaut plutôt que de remplacer le fichier :
    // ce qui se lisait se lit encore.
    const SettingsRead read = readOf("window.geometry = 0,0,0,0\n"
                                     "window.maximised = true\n");

    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.size() == 1);
}

TEST_CASE("column widths are refused in any number but four", "[config]") {
    // Trois largeurs pour quatre colonnes réglables : deviner laquelle manque
    // serait deviner ce que l'utilisateur voulait.
    CHECK(readOf("table.columns = 50,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120,700\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120\n").diagnostics.empty());
}

TEST_CASE("the two boolean values read, and they alone", "[config]") {
    // `false` autant que `true` : c'est le défaut, donc personne ne l'écrit
    // spontanément — et une valeur qu'aucun test ne lit est une valeur dont on
    // ne sait pas si elle se lit.
    CHECK(readOf("window.maximised = true\n").settings.maximised);
    CHECK_FALSE(readOf("window.maximised = false\n").settings.maximised);
    CHECK(readOf("window.maximised = false\n").diagnostics.empty());
}

TEST_CASE("a null or negative column width is unreadable", "[config]") {
    // Une colonne large de zéro est une colonne qu'on ne retrouverait pas, et
    // une largeur négative n'existe pas : ni l'une ni l'autre n'est une
    // largeur que l'utilisateur a posée.
    CHECK(readOf("table.columns = 50,0,120,120\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,-10,120,120\n").diagnostics.size() == 1);
}

TEST_CASE("a line without an equals sign is ignored", "[config]") {
    // Ce n'est pas une option, donc ce n'est pas une option illisible : rien
    // n'est signalé, et le reste du fichier se lit.
    const SettingsRead read = readOf("ceci n'est pas une option\n"
                                     "window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK(read.diagnostics.empty());
}

TEST_CASE("anything trailing after a number makes it unreadable", "[config]") {
    // « 12 pixels » n'est pas un douze suivi de bruit : c'est une valeur qu'on
    // n'a pas su lire, et l'accepter serait accepter n'importe quoi.
    CHECK(readOf("window.maximised = oui\n").diagnostics.size() == 1);
    CHECK(readOf("table.columns = 50,120,120,120 px\n").diagnostics.size() == 1);
}

// ## Les trois options venues avec #254 et #241

TEST_CASE("the three themes read, and nothing else", "[config]") {
    CHECK(readOf("general.theme = system\n").settings.theme == Theme::System);
    CHECK(readOf("general.theme = light\n").settings.theme == Theme::Light);
    CHECK(readOf("general.theme = dark\n").settings.theme == Theme::Dark);

    // Un thème qu'on ne connaît pas laisse celui qui ne fait rien.
    const SettingsRead unknown = readOf("general.theme = solarized\n");
    CHECK(unknown.settings.theme == Theme::System);
    CHECK(unknown.diagnostics.size() == 1);
}

TEST_CASE("the three themes are written back as they read", "[config]") {
    // L'aller-retour des trois, et pas seulement de celui qu'on choisit dans
    // les autres cas : une valeur qui s'écrit et ne se relit pas serait une
    // préférence perdue au redémarrage suivant.
    for (const Theme theme : {Theme::System, Theme::Light, Theme::Dark}) {
        InMemoryFileSystem files;
        REQUIRE(writeSettings(files, kPath, Settings{.theme = theme}).has_value());

        CHECK(readSettings(files, kPath).settings.theme == theme);
    }
}

// ## Le côté d'une insertion, venu avec #242

TEST_CASE("the two sides of an insertion read, and nothing else", "[config]") {
    CHECK(readOf("edit.insert-placement = above\n").settings.insertPlacement ==
          InsertPlacement::Above);
    CHECK(readOf("edit.insert-placement = below\n").settings.insertPlacement ==
          InsertPlacement::Below);

    // Un côté qu'on ne connaît pas laisse celui de Gaupol, qui est le nôtre.
    const SettingsRead unknown = readOf("edit.insert-placement = sideways\n");
    CHECK(unknown.settings.insertPlacement == InsertPlacement::Below);
    CHECK(unknown.diagnostics.size() == 1);
}

TEST_CASE("the two sides are written back as they read", "[config]") {
    for (const InsertPlacement placement : {InsertPlacement::Above, InsertPlacement::Below}) {
        InMemoryFileSystem files;
        REQUIRE(writeSettings(files, kPath, Settings{.insertPlacement = placement}).has_value());

        CHECK(readSettings(files, kPath).settings.insertPlacement == placement);
    }
}

TEST_CASE("the share given to the table refuses both its extremes", "[config]") {
    // Ni zéro ni cent : une table haute de rien, ou une bande vidéo haute de
    // rien, est une fenêtre qu'on ne saurait plus rouvrir autrement qu'en
    // effaçant son fichier de configuration.
    CHECK(readOf("window.table-share = 0\n").diagnostics.size() == 1);
    CHECK(readOf("window.table-share = 100\n").diagnostics.size() == 1);
    CHECK(readOf("window.table-share = 1\n").settings.tableShare == 1);
    CHECK(readOf("window.table-share = 99\n").settings.tableShare == 99);
    CHECK(readOf("window.table-share = deux tiers\n").diagnostics.size() == 1);
}

TEST_CASE("a relative directory is unreadable, an absolute one is not", "[config]") {
    // Un chemin relatif est relatif à un répertoire courant que personne ne
    // connaît : ce n'est pas un chemin à compléter au petit bonheur.
    CHECK(readOf("file.directory = ../films\n").diagnostics.size() == 1);
    CHECK_FALSE(readOf("file.directory = ../films\n").settings.lastDirectory.has_value());

    CHECK(readOf("file.directory = /films/quai\n").settings.lastDirectory ==
          std::filesystem::path{"/films/quai"});
}

// ## Option absente : le défaut, sans que ce soit un cas particulier

TEST_CASE("a missing option is worth its default", "[config]") {
    const SettingsRead read = readOf("window.maximised = true\n");

    CHECK(read.settings.maximised);
    CHECK_FALSE(read.settings.geometry.has_value());
    CHECK(read.settings.columnWidths.empty());
    CHECK(read.diagnostics.empty());
}

// ## Option à son défaut : réécrite commentée

TEST_CASE("an option at its default is written back commented out", "[config]") {
    const std::string written = renderSettings(Settings{});

    CHECK_THAT(written, ContainsSubstring("#window.geometry = "));
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
    CHECK_THAT(written, ContainsSubstring("#window.table-share = "));
    CHECK_THAT(written, ContainsSubstring("#file.directory = "));
    CHECK_THAT(written, ContainsSubstring("#general.theme = system"));
    CHECK_THAT(written, ContainsSubstring("#edit.insert-placement = below"));
}

TEST_CASE("an option that was set is written back bare", "[config]") {
    const std::string written = renderSettings(chosen());

    CHECK_THAT(written, ContainsSubstring("\nwindow.geometry = 40,60,1440,900\n"));
    CHECK_THAT(written, ContainsSubstring("\nwindow.maximised = true\n"));
    CHECK_THAT(written, ContainsSubstring("\ntable.columns = 50,120,120,120\n"));
    CHECK_THAT(written, ContainsSubstring("\nwindow.table-share = 63\n"));
    CHECK_THAT(written, ContainsSubstring("\nfile.directory = /films/quai\n"));
    CHECK_THAT(written, ContainsSubstring("\ngeneral.theme = dark\n"));
    CHECK_THAT(written, ContainsSubstring("\nedit.insert-placement = above\n"));
}

// **Ce que la réécriture commentée achète**, et la raison pour laquelle elle
// n'est pas une coquetterie : une option jamais touchée n'est pas figée à la
// valeur du jour où elle a été écrite. Relire ce qu'on vient d'écrire redonne
// donc les défauts, et non des valeurs gelées.
TEST_CASE("an option at its default reads back as the default after a rewrite", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, Settings{}).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == Settings{});
    CHECK(read.diagnostics.empty());
}

TEST_CASE("what was set reads back unchanged", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(writeSettings(files, kPath, chosen()).has_value());

    const SettingsRead read = readSettings(files, kPath);

    CHECK(read.settings == chosen());
    CHECK(read.diagnostics.empty());
}

// ## Ce que l'écriture demande au système de fichiers

TEST_CASE("writing makes the directory nobody made", "[config]") {
    // Au premier lancement, `~/.config/subedit` n'existe pas.
    InMemoryFileSystem files;

    CHECK(writeSettings(files, kPath, chosen()).has_value());
    CHECK(files.contentOf(kPath).has_value());
}

TEST_CASE("a refused write is returned, and stops nothing", "[config]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    CHECK_FALSE(writeSettings(files, kPath, chosen()).has_value());
}

TEST_CASE("the written file explains itself", "[config]") {
    // Il est fait pour être ouvert dans un éditeur : sans ces lignes, un
    // lecteur qui tombe sur des options commentées croit à des restes.
    const std::string written = renderSettings(Settings{});

    CHECK(written.starts_with("# subedit settings."));
    CHECK_THAT(written, ContainsSubstring("commented out"));
}
