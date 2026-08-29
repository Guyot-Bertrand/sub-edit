// Ce que la fenêtre retrouve d'une session à l'autre — issue #240.
//
// **L'aller-retour complet est le critère**, et non chaque moitié prise à part.
// Une fenêtre qui dit sa géométrie et une autre qui sait en poser une ne
// prouvent rien ensemble tant que ce qui sort de la première n'est pas entré
// dans la seconde en passant par un fichier.
//
// Rien ici ne touche un emplacement réel : le fichier est en mémoire, et son
// chemin est donné. C'est la couture de l'ADR 0022 et le harnais de #238.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/theme.hpp>

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QRect>
#include <Qt>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::Settings;
using subedit::core::Theme;
using subedit::core::WindowGeometry;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:03,000 --> 00:00:04,000\n"
                               "Deux.\n"
                               "\n";

constexpr const char* kPath = "/config/subedit/settings.conf";

/// « Sombre », troisième de la liste — l'ordre du dialogue est système, clair,
/// sombre, celle qui ne fait rien en premier puisqu'elle est le défaut.
constexpr int kDarkIndex = 2;

/// Une fenêtre sur un document, montrée : une fenêtre jamais montrée n'a pas de
/// vraie géométrie, donc rien à retenir.
class Windowed {
public:
    Windowed() {
        m_files.addFile("film.srt", kThree);
        m_window = std::make_unique<MainWindow>(
            m_files, openProject(m_files, "film.srt").value(), m_prompts);
    }

    [[nodiscard]] MainWindow& window() { return *m_window; }

    [[nodiscard]] InMemoryFileSystem& files() { return m_files; }

    [[nodiscard]] FakePrompts& prompts() { return m_prompts; }

private:
    InMemoryFileSystem m_files;
    FakePrompts m_prompts;
    std::unique_ptr<MainWindow> m_window;
};

} // namespace

TEST_CASE("la fenêtre dit où elle est et ce que ses colonnes font",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    window.setGeometry(30, 50, 1000, 700);

    const Settings said = window.settings();

    // `value_or` plutôt qu'un déréférencement après `REQUIRE` : l'analyse
    // statique ne lit pas une macro de Catch2 comme une garde, et un défaut
    // rendrait ici zéro, ce que la comparaison refuse tout autant.
    CHECK(said.geometry.value_or(WindowGeometry{}).width == 1000);
    CHECK(said.geometry.value_or(WindowGeometry{}).height == 700);
    CHECK_FALSE(said.maximised);
    CHECK(said.columnWidths.size() == subedit::core::kColumnWidthCount);
}

TEST_CASE("une géométrie posée est celle que la fenêtre prend", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(
        Settings{.geometry = WindowGeometry{.x = 20, .y = 40, .width = 900, .height = 640}});
    window.show();

    CHECK(window.width() == 900);
    CHECK(window.height() == 640);
}

TEST_CASE("des largeurs de colonnes posées sont celles que la table prend",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();

    window.applySettings(Settings{.columnWidths = {40, 130, 130, 130}});

    CHECK(window.table()->columnWidth(0) == 40);
    CHECK(window.table()->columnWidth(1) == 130);
}

// **Le critère de l'exigence, et l'aller-retour entier** : ce qu'une session
// laisse, la suivante le retrouve, en passant par le fichier.
TEST_CASE("une session retrouve la géométrie et les colonnes de la précédente",
          "[gui][config][GUI-CONFIG-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    {
        Windowed first;
        first.window().show();
        first.window().setGeometry(15, 25, 1100, 720);
        first.window().applySettings(Settings{.columnWidths = {45, 125, 125, 125}});

        subedit::gui::writeUserSettings(files, kPath, first.window().settings(), errors);
    }

    Windowed second;
    second.window().applySettings(subedit::gui::readUserSettings(files, kPath, errors));
    second.window().show();

    CHECK(second.window().width() == 1100);
    CHECK(second.window().height() == 720);
    CHECK(second.window().table()->columnWidth(0) == 45);
    CHECK(second.window().table()->columnWidth(3) == 125);
    CHECK(errors.str().empty());
}

TEST_CASE("une fenêtre laissée agrandie se rouvre agrandie", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(Settings{.maximised = true});
    window.show();

    CHECK((window.windowState() & Qt::WindowMaximized) != 0);
}

// **Une géométrie absente laisse la fenêtre se dimensionner elle-même**, et
// c'est ce qui rend le premier lancement identique à celui d'avant les
// préférences : assez large pour qu'on lise la table — #211.
TEST_CASE("sans réglages, la fenêtre s'ouvre comme elle l'a toujours fait",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(Settings{});
    window.show();

    CHECK(window.width() >= 1200);
    CHECK(window.height() >= 800);
}

// ## La poignée — #254

TEST_CASE("la part donnée à la table se pose et se relit", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.setGeometry(0, 0, 1000, 800);
    window.show();

    window.applySettings(Settings{.tableShare = 40});
    const Settings once = window.settings();

    // **Stable plutôt qu'exacte**, et c'est la bonne promesse. La bande du haut
    // a une hauteur minimale, donc une part trop petite est ramenée à ce que le
    // séparateur accepte — ce qui est juste, et non un défaut. Ce qui doit
    // tenir est qu'une part relue et reposée ne bouge plus : sans cela, la
    // poignée dériverait d'un lancement à l'autre.
    REQUIRE(once.tableShare.has_value());
    window.applySettings(once);
    CHECK(window.settings().tableShare == once.tableShare);
}

TEST_CASE("une part plus grande donne une table plus haute", "[gui][config][GUI-CONFIG-01]") {
    // L'autre moitié : une part stable qui ne voudrait rien dire serait stable
    // pour rien.
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.setGeometry(0, 0, 1000, 800);
    window.show();

    window.applySettings(Settings{.tableShare = 30});
    const int narrow = window.settings().tableShare.value_or(0);
    window.applySettings(Settings{.tableShare = 80});

    CHECK(window.settings().tableShare.value_or(0) > narrow);
}

TEST_CASE("sans part enregistrée, la poignée reste où la fenêtre la met",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    const Settings before = window.settings();

    window.applySettings(Settings{});

    CHECK(window.settings().tableShare == before.tableShare);
}

// ## Le dernier répertoire — #254

TEST_CASE("la boîte « ouvrir » s'ouvre sur le répertoire du dernier fichier",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.lastDirectory = std::filesystem::path{"/films/quai"}});
    window.show();

    window.openAction()->trigger();

    CHECK(fixture.prompts().lastOpenDirectory == std::filesystem::path{"/films/quai"});
}

TEST_CASE("une boîte annulée ne déplace pas le répertoire retenu", "[gui][config][GUI-CONFIG-01]") {
    // Ce qui compte est où l'utilisateur travaille, pas où il a regardé.
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.lastDirectory = std::filesystem::path{"/films/quai"}});
    window.show();
    fixture.prompts().nextFileToOpen = {};

    window.openAction()->trigger();

    CHECK(window.settings().lastDirectory == std::filesystem::path{"/films/quai"});
}

TEST_CASE("ouvrir un fichier retient son répertoire", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    fixture.files().addFile("/ailleurs/autre.srt", kThree);
    fixture.prompts().nextFileToOpen = "/ailleurs/autre.srt";
    window.show();

    window.openAction()->trigger();

    CHECK(window.settings().lastDirectory == std::filesystem::path{"/ailleurs"});
}

// ## Le thème — #241

TEST_CASE("le thème choisi dans les préférences est celui que la fenêtre rend",
          "[gui][config][GUI-THEME-01]") {
    const subedit::gui::PreferencesDialog probe{Theme::System};
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();

    // Le faux remplit le dialogue puis répond « validé », ce qu'un humain fait
    // en choisissant dans la liste avant de cliquer.
    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences != nullptr)
            preferences->themeBox()->setCurrentIndex(kDarkIndex);
    };
    fixture.prompts().nextRun = true;

    window.preferencesAction()->trigger();

    CHECK(window.settings().theme == Theme::Dark);
    CHECK(probe.theme() == Theme::System);
}

TEST_CASE("des préférences annulées ne changent rien", "[gui][config][GUI-THEME-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.theme = Theme::Light});
    window.show();

    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences != nullptr)
            preferences->themeBox()->setCurrentIndex(kDarkIndex);
    };
    fixture.prompts().nextRun = false;

    window.preferencesAction()->trigger();

    CHECK(window.settings().theme == Theme::Light);
}

TEST_CASE("un thème enregistré est celui que la fenêtre rouvre", "[gui][config][GUI-THEME-01]") {
    Windowed fixture;

    fixture.window().applySettings(Settings{.theme = Theme::Light});

    CHECK(fixture.window().settings().theme == Theme::Light);
}

// ## Ce que le câblage écrit sur la sortie d'erreur

TEST_CASE("une valeur illisible est nommée, avec ce qui était écrit",
          "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.geometry = plus tard\n");
    std::ostringstream errors;

    const Settings read = subedit::gui::readUserSettings(files, kPath, errors);

    CHECK_FALSE(read.geometry.has_value());
    CHECK_THAT(errors.str(), ContainsSubstring("window.geometry"));
    // La valeur fautive est citée : sans elle, l'utilisateur cherche.
    CHECK_THAT(errors.str(), ContainsSubstring("\"plus tard\""));
    CHECK_THAT(errors.str(), ContainsSubstring("keeping the default"));
}

TEST_CASE("un fichier absent ne fait dire mot", "[gui][config][GUI-CONFIG-02]") {
    const InMemoryFileSystem files;
    std::ostringstream errors;

    CHECK(subedit::gui::readUserSettings(files, kPath, errors) == Settings{});
    CHECK(errors.str().empty());
}

TEST_CASE("un fichier illisible le dit, et la fenêtre part sur les défauts",
          "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.maximised = true\n");
    files.failNextRead(FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    CHECK(subedit::gui::readUserSettings(files, kPath, errors) == Settings{});
    CHECK_THAT(errors.str(), ContainsSubstring("permission denied"));
}

// **Ce que la réécriture commentée achète**, vu du fichier que le programme
// écrit vraiment : une option jamais touchée n'est pas figée à la valeur du jour
// où elle a été écrite, donc un défaut qu'on améliore atteint tout le monde. Sans
// elle, l'améliorer n'atteindrait plus personne.
TEST_CASE("le fichier écrit commente ce qui est resté au défaut", "[gui][config][GUI-CONFIG-03]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{}, errors);

    const std::string written = files.contentOf(kPath).value_or("");
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
    CHECK(errors.str().empty());
}

TEST_CASE("une option réglée est écrite sans commentaire", "[gui][config][GUI-CONFIG-03]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{.maximised = true}, errors);

    CHECK_THAT(files.contentOf(kPath).value_or(""),
               ContainsSubstring("\nwindow.maximised = true\n"));
}

// **Les deux surcharges qui résolvent l'emplacement se répondent.** C'est tout
// ce qu'il y a à prouver d'elles : qu'écrire puis relire, sans jamais dire où,
// rende ce qu'on a écrit — donc que les deux parlent du même endroit.
//
// **Rien n'atteint un vrai fichier**, et pas seulement parce que le harnais
// déplace `XDG_CONFIG_HOME` : le système de fichiers est en mémoire, donc le
// chemin résolu n'est utilisé que comme une clé.
TEST_CASE("écrire puis relire sans dire où rend ce qui a été écrit",
          "[gui][config][GUI-CONFIG-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    const Settings chosen{.maximised = true, .columnWidths = {41, 121, 121, 121}};

    subedit::gui::writeUserSettings(files, chosen, errors);

    CHECK(subedit::gui::readUserSettings(files, errors) == chosen);
    CHECK(errors.str().empty());
}

TEST_CASE("une écriture refusée le dit, et rien de plus", "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{}, errors);

    CHECK_THAT(errors.str(), ContainsSubstring("settings could not be written"));
}
