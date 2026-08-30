// Le manuel installé, lu dans la fenêtre — issue #245.
//
// **Rien ici ne touche un vrai manuel.** Le manuel de ces cas est écrit en
// mémoire, et son chemin est donné : c'est la couture de l'ADR 0022, la même
// que pour les réglages. `installedManualPath()` est le seul code qui résout
// l'emplacement réel, et il a son propre cas plus bas.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_path.hpp>
#include <subedit/gui/manual_window.hpp>

#include <QAction>
#include <QApplication>
#include <QString>
#include <QUrl>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::installedManualPath;
using subedit::gui::MainWindow;
using subedit::gui::ManualWindow;
using subedit::test::FakePrompts;

constexpr const char* kManual = "/opt/subedit/manual";

constexpr const char* kIndex = "# subedit — manuel\n"
                               "\n"
                               "Deux programmes.\n"
                               "\n"
                               "| Programme | Pour quoi faire |\n"
                               "| :-------- | :-------------- |\n"
                               "| `subedit-gui` | la fenêtre |\n"
                               "| `subedit-cli` | le lot |\n"
                               "\n"
                               "Voir [la table](subedit-gui/table.md), et la\n"
                               "[feuille de route](../feuille-de-route.md).\n";

constexpr const char* kTable = "# La table\n"
                               "\n"
                               "Cinq colonnes, une par sous-titre.\n"
                               "\n"
                               "Retour vers [l'accueil](../index.md).\n";

/// Un manuel en mémoire, à l'emplacement que les cas se donnent.
[[nodiscard]] InMemoryFileSystem withManual() {
    InMemoryFileSystem files;
    files.addFile(std::string{kManual} + "/index.md", kIndex);
    files.addFile(std::string{kManual} + "/subedit-gui/table.md", kTable);
    return files;
}

} // namespace

TEST_CASE("le manuel s'ouvre sur sa page d'accueil", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Deux programmes"));
}

TEST_CASE("les tableaux du manuel sont rendus, et c'est ce que le cadrage demandait de vérifier",
          "[gui][GUI-MANUAL-01]") {
    // **Le risque nommé par le cadrage** : « nos sections usent de tableaux, et
    // le rendu Markdown de Qt a ses limites ». Elles ne mordent pas — le
    // dialecte GitHub fait de vrais tableaux, et le contenu des cellules est
    // là. L'alternative d'un rendu HTML à la construction n'a donc pas eu à
    // être discutée.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    const std::string shown = manual.shownText().toStdString();
    CHECK_THAT(shown, ContainsSubstring("Programme"));
    CHECK_THAT(shown, ContainsSubstring("subedit-gui"));
    CHECK_THAT(shown, ContainsSubstring("la fenêtre"));
    CHECK_THAT(shown, ContainsSubstring("le lot"));

    // Et les barres du tableau ne sont pas rendues telles quelles : ce serait
    // le signe d'un dialecte qui ignore les tableaux et les laisse en texte.
    CHECK_THAT(shown, !ContainsSubstring("| :--------"));
}

TEST_CASE("un lien du manuel ouvre la page qu'il désigne", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/table.md");

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Cinq colonnes"));
    CHECK(manual.notice().isEmpty());
}

TEST_CASE("le retour ramène à la page précédente", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK_FALSE(manual.backAction()->isEnabled());

    manual.openPage("subedit-gui/table.md");
    REQUIRE(manual.backAction()->isEnabled());

    manual.backAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_FALSE(manual.backAction()->isEnabled());
}

TEST_CASE("l'accueil s'éteint quand on y est déjà", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK_FALSE(manual.homeAction()->isEnabled());

    manual.openPage("subedit-gui/table.md");
    REQUIRE(manual.homeAction()->isEnabled());

    manual.homeAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("cliquer un lien du manuel ouvre la page visée", "[gui][GUI-MANUAL-01]") {
    // Le chemin d'un vrai clic : le lien est relatif à la page qui le porte, et
    // c'est ce qui distingue ce cas de l'ouverture directe juste au-dessus.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});

    // Et depuis là, un lien qui remonte d'un cran ramène à l'accueil : c'est la
    // résolution relative qui le fait, pas une table de correspondances.
    manual.followLink(QUrl{QStringLiteral("../index.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("un lien qui sort du manuel installé est dit, jamais suivi", "[gui][GUI-MANUAL-01]") {
    // **La règle que la fenêtre porte seule.** Le manuel renvoie à la feuille de
    // route et aux ADR, qui vivent dans le dépôt et ne sont pas installés. Un
    // clic sans effet laisserait croire à une panne.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("../feuille-de-route.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.notice().toStdString(),
               ContainsSubstring("not part of the installed manual"));
}

TEST_CASE("un lien vers une page absente est dit de la même façon", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-cli/inspect.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.notice().toStdString(),
               ContainsSubstring("not part of the installed manual"));
}

TEST_CASE("une ancre seule ne change pas de page", "[gui][GUI-MANUAL-01]") {
    // « #le-thème » désigne la page courante : il n'y a rien à charger, et le
    // navigateur y descend tout seul.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("#par-ou-commencer")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK(manual.notice().isEmpty());
}

TEST_CASE("le retour forcé sans historique ne fait rien", "[gui][GUI-MANUAL-01]") {
    // Le second garde, celui que l'action éteinte cache : `trigger()` sur une
    // action éteinte ne déclenche rien, donc ce chemin demande de la rallumer
    // à la main pour être atteint. Il existe quand même.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    manual.backAction()->setEnabled(true);
    manual.backAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("une page absente est dite, et ne remplace rien", "[gui][GUI-MANUAL-01]") {
    // Le cas d'une installation partielle : la fenêtre le dit et garde ce
    // qu'elle montrait.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-cli/inspect.md");

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.notice().toStdString(), ContainsSubstring("could not be read"));
}

TEST_CASE("un manuel entièrement absent ouvre une fenêtre qui le dit", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files;
    const ManualWindow manual{files, kManual};

    CHECK(manual.currentPage().empty());
    CHECK_THAT(manual.notice().toStdString(), ContainsSubstring("could not be read"));
}

TEST_CASE("l'entrée du menu s'allume quand le manuel est là", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    FakePrompts prompts;
    MainWindow window{files, openProject(files, "film.srt").value(), prompts};

    // Éteinte tant que personne n'a dit où regarder : c'est l'état d'un binaire
    // lancé depuis l'arbre de construction.
    CHECK_FALSE(window.manualAction()->isEnabled());

    window.setManualPath(kManual);

    CHECK(window.manualAction()->isEnabled());
}

TEST_CASE("l'entrée reste éteinte sur une installation partielle", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    // Un manuel dont l'accueil manque : le répertoire existe, la page non.
    files.addFile(std::string{kManual} + "/subedit-gui/table.md", kTable);
    FakePrompts prompts;
    MainWindow window{files, openProject(files, "film.srt").value(), prompts};

    window.setManualPath(kManual);

    CHECK_FALSE(window.manualAction()->isEnabled());
}

TEST_CASE("l'entrée ouvre la fenêtre du manuel, et une seule", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    FakePrompts prompts;
    MainWindow window{files, openProject(files, "film.srt").value(), prompts};
    window.setManualPath(kManual);

    CHECK(window.manualWindow() == nullptr);

    window.manualAction()->trigger();
    ManualWindow* opened = window.manualWindow();

    REQUIRE(opened != nullptr);
    CHECK(opened->isVisible());
    CHECK_THAT(opened->shownText().toStdString(), ContainsSubstring("Deux programmes"));

    window.manualAction()->trigger();

    CHECK(window.manualWindow() == opened);
}

TEST_CASE("l'emplacement du manuel se déduit de l'exécutable", "[gui][GUI-MANUAL-01]") {
    // **Déduit et non gravé** : le préfixe de configuration et celui
    // d'installation ne sont pas le même, et le manuel recommande justement une
    // installation sous un autre préfixe.
    const std::filesystem::path resolved = installedManualPath();

    CHECK(resolved.is_absolute());
    CHECK(resolved.filename() == "manual");
    CHECK(resolved.parent_path().filename() == "subedit");
    CHECK(resolved.parent_path().parent_path().filename() == "share");

    // À côté de l'exécutable, et non sous lui : `bin` et `share` sont frères,
    // ce que `GNUInstallDirs` produit quel que soit le préfixe.
    const std::filesystem::path binaries{QApplication::applicationDirPath().toStdString()};
    CHECK(resolved.parent_path().parent_path().parent_path() == binaries.parent_path());
}
