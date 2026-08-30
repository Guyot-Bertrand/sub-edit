#pragma once

#include <QWidget>

#include <filesystem>
#include <vector>

class QAction;
class QLabel;
class QTextBrowser;

namespace subedit::core {
class FileSystem;
} // namespace subedit::core

namespace subedit::gui {

/// Le manuel installé, lu dans une fenêtre.
///
/// **Le manuel installé et non une URL** — décision D6 du cadrage de la phase 7.
/// Gaupol ouvre une adresse ; nous ne le suivons pas, parce qu'une URL décrit
/// `main` et non la version qu'on a sous la main. Un utilisateur qui lit le
/// manuel d'une version qu'il n'a pas est moins bien servi qu'un utilisateur
/// sans manuel.
///
/// **Du Markdown rendu par Qt, sans dépendance nouvelle.**
/// `QTextDocument::setMarkdown` le fait, dialecte GitHub, et le cadrage
/// demandait de vérifier tôt que nos tableaux tiennent : ils tiennent — ils
/// deviennent de vrais `QTextTable`, lignes et colonnes comprises. L'alternative
/// d'un rendu HTML à la construction n'a donc pas eu à être discutée.
///
/// **Une fenêtre et non une modale.** On consulte un manuel *pendant* qu'on
/// travaille, et une modale interdirait précisément cela. Elle échappe donc à
/// `Prompts`, et c'est sans conséquence pour les tests : cette couture existe
/// pour tenir `exec()` hors de leur portée, et rien ici ne l'appelle.
class ManualWindow final : public QWidget {
    Q_OBJECT

public:
    /// Ouvre le manuel qui vit sous `directory`, sur sa page d'accueil.
    ///
    /// `files` doit lui survivre. Le manuel est lu à travers lui plutôt que par
    /// Qt : c'est ce qui permet à un test de poser un manuel en mémoire, sans
    /// répertoire ni fichier réels.
    ManualWindow(core::FileSystem& files,
                 std::filesystem::path directory,
                 QWidget* parent = nullptr);

    /// La page ouverte, relative à la racine du manuel.
    [[nodiscard]] std::filesystem::path currentPage() const { return m_current; }

    /// Ce que la page affiche, texte seul — ce qu'un test lit à la place d'un
    /// lecteur.
    [[nodiscard]] QString shownText() const;

    /// L'action de retour, pour qu'un test la déclenche et lise son état.
    [[nodiscard]] QAction* backAction() const { return m_back; }

    [[nodiscard]] QAction* homeAction() const { return m_home; }

    /// Ce que la fenêtre dit quand elle ne peut pas ouvrir quelque chose, ou
    /// rien. Une bande sous la barre d'outils, effacée dès qu'une page s'ouvre.
    [[nodiscard]] QString notice() const;

    /// Ouvre `page`, relative à la racine du manuel. Publique pour qu'un test
    /// navigue sans cliquer.
    void openPage(const std::filesystem::path& page);

    /// Suit un lien de la page courante — ce qu'un clic déclenche.
    ///
    /// **Un lien qui sort du manuel installé est dit, jamais suivi.** Le manuel
    /// renvoie huit fois à la feuille de route et aux ADR, qui sont des
    /// documents du dépôt et non du paquet : ils existent là où le manuel
    /// s'écrit, pas là où il s'installe. Le taire laisserait croire à un clic
    /// sans effet.
    ///
    /// Publique pour la raison qui rend `openPage` publique, et une de plus :
    /// c'est ici que vit la règle ci-dessus, donc c'est ici qu'un test doit
    /// pouvoir la mettre à l'épreuve.
    void followLink(const QUrl& target);

private:
    /// Recompute ce que les deux actions ont le droit de faire.
    void refreshActions();

    core::FileSystem* m_files;
    std::filesystem::path m_directory;
    std::filesystem::path m_current;

    /// Les pages déjà ouvertes, la dernière en tête de ce qui reste à défaire.
    ///
    /// Une pile à nous plutôt que celle de `QTextBrowser` : la sienne suit
    /// `setSource`, qui chargerait le Markdown comme du texte brut. Nous
    /// chargeons nous-mêmes, donc nous retenons nous-mêmes.
    std::vector<std::filesystem::path> m_visited;

    QTextBrowser* m_view;
    QLabel* m_notice;
    QAction* m_back;
    QAction* m_home;
};

} // namespace subedit::gui
