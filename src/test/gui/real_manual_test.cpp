// Le vrai manuel, ouvert dans la vraie fenêtre — issue #268.
//
// **Ce fichier existe parce que le voisin ne suffisait pas.** `manual_window_test.cpp`
// éprouve la fenêtre sur un manuel écrit dans le test : deux pages, un tableau
// de deux lignes, trois liens. C'est le bon manuel pour éprouver une fenêtre —
// il est petit, il est là, et il ne bouge que quand on l'écrit.
//
// Ce n'est pas le bon manuel pour éprouver le *rendu*. La décision D6 du
// cadrage de la phase 7 posait une question — « nos sections usent de tableaux,
// et le rendu Markdown de Qt a ses limites ; si elles mordent, l'alternative
// est un rendu HTML à la construction » — et la réponse a été prise sur ce
// tableau de deux lignes. Les vingt-quatre pages réelles portent des citations,
// des listes imbriquées, des blocs `console`, des images, des tableaux à cinq
// colonnes et des liens relatifs à deux niveaux. Ce que MD4C en faisait n'était
// vérifié nulle part.
//
// **Les pages lues sont celles du dépôt, et l'installé en est la copie page
// pour page** — `cmake/Installation.cmake` dépose `docs/manual/` entier, et
// `check-installation.sh` vérifie qu'il n'en manque aucune. Le chemin est donné
// à la compilation, comme il l'est au programme de captures : rien ici ne
// résout un emplacement.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/gui/manual_window.hpp>

#include <QLatin1Char>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::RealFileSystem;
using subedit::gui::ManualWindow;

/// La racine du manuel du dépôt, donnée par le système de construction.
[[nodiscard]] std::filesystem::path manualRoot() {
    return std::filesystem::path{SUBEDIT_MANUAL_DIR};
}

/// Toutes les pages du manuel, relatives à sa racine, dans un ordre stable.
///
/// **Découvertes et non énumérées.** Une liste écrite ici se périmerait au
/// premier chapitre ajouté, et le manque ne se verrait que si quelqu'un pensait
/// à l'y inscrire — c'est-à-dire jamais. C'est la règle que
/// `check-installation.sh` s'applique déjà pour la même raison.
[[nodiscard]] std::vector<std::filesystem::path> everyPage() {
    std::vector<std::filesystem::path> pages;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::recursive_directory_iterator{manualRoot()}) {
        if (entry.is_regular_file() && entry.path().extension() == ".md")
            pages.push_back(std::filesystem::relative(entry.path(), manualRoot()));
    }

    std::ranges::sort(pages);
    return pages;
}

[[nodiscard]] std::string sourceOf(RealFileSystem& files, const std::filesystem::path& page) {
    return files.readFile(manualRoot() / page).value_or(std::string{});
}

/// Combien de tableaux la source d'une page déclare.
///
/// Comptés sur la ligne de séparation, celle qui suit l'en-tête : c'est la
/// seule ligne qu'un tableau Markdown ne peut pas ne pas avoir, et la seule
/// qu'un paragraphe ordinaire ne peut pas avoir par accident.
[[nodiscard]] int tablesDeclaredIn(const std::string& source) {
    static const QRegularExpression separator{QStringLiteral(R"(^\|[ :|-]+\|$)"),
                                              QRegularExpression::MultilineOption};

    int found = 0;
    QRegularExpressionMatchIterator each = separator.globalMatch(QString::fromStdString(source));
    while (each.hasNext()) {
        each.next();
        ++found;
    }
    return found;
}

/// Dit si un renvoi sort du manuel — le réseau, ou un document du dépôt.
[[nodiscard]] bool leavesTheManual(const std::filesystem::path& page, const QString& href) {
    if (href.startsWith(QStringLiteral("http:")) || href.startsWith(QStringLiteral("https:")) ||
        href.startsWith(QStringLiteral("mailto:")))
        return true;

    const QUrl target{href};
    const std::filesystem::path asked{target.path().toStdString()};
    if (asked.empty())
        return false;

    const std::filesystem::path resolved = (page.parent_path() / asked).lexically_normal();

    return resolved.empty() || *resolved.begin() == "..";
}

} // namespace

TEST_CASE("every manual page renders, and renders something", "[gui][GUI-MANUAL-01]") {
    // **Le contrôle le plus bête, et celui qui manquait.** Une page dont le
    // rendu est vide s'ouvre exactement comme une page qu'on n'a pas écrite :
    // la fenêtre ne dit rien, puisqu'elle a bien lu le fichier.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    const std::vector<std::filesystem::path> pages = everyPage();
    REQUIRE(pages.size() >= 20);

    for (const std::filesystem::path& page : pages) {
        INFO("page : " << page.string());
        manual.openPage(page);

        CHECK(manual.currentPage() == page);
        CHECK(manual.notice().isEmpty());
        CHECK_FALSE(manual.shownText().trimmed().isEmpty());

        // Le titre de la page est sa première ligne, et il survit au rendu :
        // c'est ce qui distingue « rendu » de « rendu en partie ». Ses accents
        // graves partent avec le rendu — la moitié des titres du manuel de la
        // ligne de commande est un nom de sous-commande, écrit en code.
        const std::string source = sourceOf(files, page);
        REQUIRE_FALSE(source.empty());
        const QString title = QString::fromStdString(source.substr(0, source.find('\n')))
                                  .mid(2)
                                  .remove(QLatin1Char{'`'})
                                  .trimmed();
        REQUIRE_FALSE(title.isEmpty());
        CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring(title.toStdString()));
    }
}

TEST_CASE("every table in the manual becomes a table", "[gui][GUI-MANUAL-01]") {
    // **La question de la décision D6, posée sur les vraies pages.** Elle avait
    // été répondue sur un tableau de deux lignes écrit dans un test ; le manuel
    // en porte quatre-vingt-dix, dont des cinq colonnes, dont certains portent
    // du code et des liens dans leurs cellules. Si les limites du rendu Markdown
    // de Qt mordent quelque part, c'est là.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    int total = 0;

    for (const std::filesystem::path& page : everyPage()) {
        INFO("page : " << page.string());
        manual.openPage(page);

        const int declared = tablesDeclaredIn(sourceOf(files, page));
        CHECK(manual.shownTables() == declared);
        total += declared;

        // Et les barres de séparation ne restent pas dans le texte : ce serait
        // le signe d'un dialecte qui laisse les tableaux tels quels.
        CHECK_THAT(manual.shownText().toStdString(), !ContainsSubstring("| :---"));
    }

    // Un garde contre le pire des verts : une heuristique qui ne compterait
    // rien ferait passer toutes les égalités ci-dessus.
    CHECK(total > 50);
}

TEST_CASE("every manual image loads in the page that shows it", "[gui][GUI-MANUAL-01]") {
    // Les captures vivent dans un sous-répertoire voisin de la page, et le
    // rendu ne les trouve que par les chemins de recherche qu'`openPage` pose.
    // Un chemin posé pour la mauvaise page donnerait une page qui s'affiche
    // sans ses images, sans un mot.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    for (const std::filesystem::path& page : everyPage()) {
        INFO("page : " << page.string());
        manual.openPage(page);

        CHECK(manual.missingImages().join(QStringLiteral(", ")).toStdString().empty());
    }
}

TEST_CASE("every manual link resolves, or is reported", "[gui][GUI-MANUAL-01]") {
    // **Ce que `check-manual-links.py` ne peut pas dire.** Il confronte les
    // renvois de la source à ce qui existe sur le disque, depuis la racine du
    // dépôt — un point de vue que la fenêtre n'a pas. Elle voit le manuel
    // installé, où la feuille de route et les ADR n'existent pas, et elle
    // résout les chemins relatifs elle-même. Deux résolutions différentes du
    // même lien, et une seule était éprouvée.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    int followed = 0;
    int said = 0;
    int descended = 0;

    for (const std::filesystem::path& page : everyPage()) {
        manual.openPage(page);
        const QStringList links = manual.shownLinks();

        for (const QString& href : links) {
            INFO("page : " << page.string() << " → " << href.toStdString());

            // Le lien est suivi depuis la page qui le porte : c'est ce que fait
            // un clic, et la page courante est ce par rapport à quoi la fenêtre
            // résout. La page est donc rouverte avant chacun, le précédent
            // l'ayant quittée.
            manual.openPage(page);
            REQUIRE(manual.currentPage() == page);

            manual.followLink(QUrl{href});

            if (leavesTheManual(page, href)) {
                // Dit, jamais suivi : la feuille de route et les ADR vivent
                // dans le dépôt et ne sont pas installés.
                CHECK(manual.currentPage() == page);
                CHECK_THAT(manual.notice().toStdString(),
                           ContainsSubstring("not part of the installed manual"));
                ++said;
                continue;
            }

            if (href.startsWith('#')) {
                // Une ancre seule ne change pas de page : elle y descend.
                CHECK(manual.currentPage() == page);
                CHECK(manual.notice().isEmpty());
                CHECK(manual.currentSection() == QUrl{href}.fragment());
                ++followed;
                ++descended;
                continue;
            }

            const std::filesystem::path target =
                (page.parent_path() / std::filesystem::path{QUrl{href}.path().toStdString()})
                    .lexically_normal();

            CHECK(manual.currentPage() == target);
            CHECK(manual.notice().isEmpty());
            if (!QUrl{href}.fragment().isEmpty()) {
                // **Ce que le renvoi promet en plus d'une page** : la section.
                // Le rendu Markdown de Qt ne nomme aucune ancre, donc ces
                // renvois ouvraient la page sans y descendre — c'est le défaut
                // que #268 a trouvé, et la fenêtre les suit désormais.
                CHECK(manual.currentSection() == QUrl{href}.fragment());
                ++descended;
            }

            ++followed;
        }
    }

    // Les trois comptes se gardent l'un l'autre : une extraction de liens qui
    // rendrait une liste vide ferait passer toute la boucle ci-dessus.
    CHECK(followed > 100);
    CHECK(said > 0);
    CHECK(descended > 30);
}
