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
#include <QStringList>
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
                               "Retour vers [l'accueil](../index.md).\n"
                               "\n"
                               "## Les anomalies\n"
                               "\n"
                               "Ce que la table souligne.\n"
                               "\n"
                               "## Les erreurs\n"
                               "\n"
                               "Les premières.\n"
                               "\n"
                               "## Les erreurs\n"
                               "\n"
                               "Les secondes, dont l'ancre est numérotée.\n";

/// Une page qui montre une image que rien ne pose sur le disque.
///
/// **Les images ne passent pas par le système de fichiers de la fenêtre** : le
/// rendu les cherche lui-même, dans le répertoire de la page. Une page en
/// mémoire n'a donc pas d'images, et c'est ce qui rend ce cas atteignable sans
/// rien écrire.
constexpr const char* kSansImage = "# Une capture manquante\n"
                                   "\n"
                                   "![La fenêtre](captures/absente.png)\n";

/// Une page qui ne commence pas par un titre.
///
/// Elle existe pour un seul cas : la vue posée ailleurs que sur un titre, que
/// les pages du manuel ne produisent jamais — elles commencent toutes par leur
/// nom.
constexpr const char* kSansTitre = "Rien qu'un paragraphe.\n";

/// Un manuel en mémoire, à l'emplacement que les cas se donnent.
[[nodiscard]] InMemoryFileSystem withManual() {
    InMemoryFileSystem files;
    files.addFile(std::string{kManual} + "/index.md", kIndex);
    files.addFile(std::string{kManual} + "/subedit-gui/table.md", kTable);
    files.addFile(std::string{kManual} + "/subedit-gui/sans-titre.md", kSansTitre);
    files.addFile(std::string{kManual} + "/subedit-gui/sans-image.md", kSansImage);
    return files;
}

} // namespace

TEST_CASE("the manual opens on its home page", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Deux programmes"));
}

TEST_CASE("the manual's tables are rendered, which is what the scoping asked to check",
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

TEST_CASE("a manual link opens the page it names", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/table.md");

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Cinq colonnes"));
    CHECK(manual.notice().isEmpty());
}

TEST_CASE("back returns to the previous page", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK_FALSE(manual.backAction()->isEnabled());

    manual.openPage("subedit-gui/table.md");
    REQUIRE(manual.backAction()->isEnabled());

    manual.backAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_FALSE(manual.backAction()->isEnabled());
}

TEST_CASE("contents is disabled when already there", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK_FALSE(manual.homeAction()->isEnabled());

    manual.openPage("subedit-gui/table.md");
    REQUIRE(manual.homeAction()->isEnabled());

    manual.homeAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("clicking a manual link opens the page it targets", "[gui][GUI-MANUAL-01]") {
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

TEST_CASE("a link leaving the installed manual is reported, never followed",
          "[gui][GUI-MANUAL-01]") {
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

TEST_CASE("a link to a missing page is reported the same way", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-cli/inspect.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.notice().toStdString(),
               ContainsSubstring("not part of the installed manual"));
}

TEST_CASE("an anchor alone does not change page", "[gui][GUI-MANUAL-01]") {
    // « #le-thème » désigne la page courante : il n'y a rien à charger, et la
    // fenêtre y descend.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/table.md");
    manual.followLink(QUrl{QStringLiteral("#les-anomalies")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.notice().isEmpty());
    CHECK(manual.currentSection() == QStringLiteral("les-anomalies"));
}

TEST_CASE("a link with an anchor opens the page and scrolls to it", "[gui][GUI-MANUAL-01]") {
    // **Le défaut trouvé par #268.** Le rendu Markdown de Qt ne nomme aucune
    // ancre — un titre y est un bloc de niveau, pas une cible —, donc les
    // renvois du manuel ouvraient la bonne page et la laissaient à son début.
    // Le manuel en porte une quarantaine, tous vérifiés jusque-là contre les
    // ancres de GitHub et jamais contre celles de la fenêtre.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#les-anomalies")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.currentSection() == QStringLiteral("les-anomalies"));
}

TEST_CASE("two identical headings give two anchors", "[gui][GUI-MANUAL-01]") {
    // La règle de GitHub, que `check-manual-links.py` applique de son côté : le
    // second « Les erreurs » d'une page s'appelle `les-erreurs-1`.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#les-erreurs-1")});

    CHECK(manual.currentSection() == QStringLiteral("les-erreurs-1"));
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Les secondes"));
}

TEST_CASE("an anchor naming nothing leaves the page at its start", "[gui][GUI-MANUAL-01]") {
    // **Silencieuse, contrairement au reste de cette fenêtre.** Le manuel est
    // livré avec le programme et non écrit par qui l'utilise : une ancre morte
    // est un défaut du dépôt, que `check-manual-links.py` et le test des pages
    // réelles refusent tous les deux. Le message n'aurait jamais de lecteur.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#une-section-effacee")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.currentSection() == QStringLiteral("la-table"));
    CHECK(manual.notice().isEmpty());
}

TEST_CASE("an image the rendering cannot find is named", "[gui][GUI-MANUAL-01]") {
    // **Le pendant du test des pages réelles**, qui exige que le manuel du
    // dépôt n'en ait aucune. Sans ce cas-ci, rien ne dirait que le contrôle
    // sait répondre autre chose que « rien ne manque ».
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/sans-image.md");

    CHECK(manual.missingImages() == QStringList{QStringLiteral("captures/absente.png")});
}

TEST_CASE("the rendered tables can be counted", "[gui][GUI-MANUAL-01]") {
    // Le compte, et non seulement le texte des cellules : c'est ce qui
    // distingue un vrai `QTextTable` d'un tableau laissé en texte, et c'est ce
    // que le test des pages réelles confronte à ce que chaque source déclare.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK(manual.shownTables() == 1);

    manual.openPage("subedit-gui/table.md");

    CHECK(manual.shownTables() == 0);
}

TEST_CASE("the rendered links are read from the document", "[gui][GUI-MANUAL-01]") {
    // Ceux que le rendu a faits, et non ceux que la source écrit : un renvoi
    // que le Markdown n'aurait pas reconnu ne serait pas dans cette liste.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    CHECK(manual.shownLinks() == QStringList{QStringLiteral("subedit-gui/table.md"),
                                             QStringLiteral("../feuille-de-route.md")});
}

TEST_CASE("a page not starting with a heading is in no section", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/sans-titre.md");

    CHECK(manual.currentSection().isEmpty());
}

TEST_CASE("back forced with no history does nothing", "[gui][GUI-MANUAL-01]") {
    // Le second garde, celui que l'action éteinte cache : `trigger()` sur une
    // action éteinte ne déclenche rien, donc ce chemin demande de la rallumer
    // à la main pour être atteint. Il existe quand même.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    manual.backAction()->setEnabled(true);
    manual.backAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("a missing page is reported, and replaces nothing", "[gui][GUI-MANUAL-01]") {
    // Le cas d'une installation partielle : la fenêtre le dit et garde ce
    // qu'elle montrait.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-cli/inspect.md");

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
    CHECK_THAT(manual.notice().toStdString(), ContainsSubstring("could not be read"));
}

TEST_CASE("a wholly absent manual opens a window that says so", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files;
    const ManualWindow manual{files, kManual};

    CHECK(manual.currentPage().empty());
    CHECK_THAT(manual.notice().toStdString(), ContainsSubstring("could not be read"));
}

TEST_CASE("the menu entry lights up when the manual is there", "[gui][GUI-MANUAL-01]") {
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

TEST_CASE("the entry stays disabled on a partial installation", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    // Un manuel dont l'accueil manque : le répertoire existe, la page non.
    files.addFile(std::string{kManual} + "/subedit-gui/table.md", kTable);
    FakePrompts prompts;
    MainWindow window{files, openProject(files, "film.srt").value(), prompts};

    window.setManualPath(kManual);

    CHECK_FALSE(window.manualAction()->isEnabled());
}

TEST_CASE("the entry opens the manual window, and only one", "[gui][GUI-MANUAL-01]") {
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

TEST_CASE("the manual's location is derived from the executable", "[gui][GUI-MANUAL-01]") {
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
