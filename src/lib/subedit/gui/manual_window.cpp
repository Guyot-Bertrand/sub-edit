#include <subedit/core/io/file_system.hpp>
#include <subedit/gui/manual_window.hpp>

#include <QAction>
#include <QLabel>
#include <QString>
#include <QTextBrowser>
#include <QTextDocument>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#include <expected>
#include <filesystem>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// La page sur laquelle le manuel s'ouvre.
///
/// Celle que #243 a écrite pour cela : elle dit ce que chacun des deux
/// programmes fait, et par où commencer.
const std::filesystem::path kHomePage{"index.md"};

/// La taille d'ouverture, en pixels.
///
/// Assez large pour que les tableaux du manuel tiennent sans repli — ils y sont
/// nombreux et c'est ce qu'ils rendent le plus mal — et assez haute pour qu'une
/// section se lise sans défiler à chaque paragraphe.
constexpr int kWidth = 900;
constexpr int kHeight = 700;

/// Ce qu'une page devient une fois lue, relative à la racine du manuel.
///
/// **Normalisée, parce que les liens sont relatifs à la page qui les porte** :
/// `../subedit-cli/installation.md` cliqué depuis `subedit-gui/index.md` doit
/// désigner une page du manuel et pas un chemin qui remonte au-dessus de lui.
[[nodiscard]] std::filesystem::path resolved(const std::filesystem::path& from,
                                             const std::filesystem::path& target) {
    return (from.parent_path() / target).lexically_normal();
}

/// Dit si `page` reste sous la racine du manuel.
[[nodiscard]] bool inside(const std::filesystem::path& page) {
    return !page.empty() && *page.begin() != "..";
}

} // namespace

ManualWindow::ManualWindow(core::FileSystem& files,
                           std::filesystem::path directory,
                           QWidget* parent)
    : QWidget(parent, Qt::Window),
      m_files(&files),
      m_directory(std::move(directory)),
      m_view(new QTextBrowser{this}),
      m_notice(new QLabel{this}),
      m_back(new QAction{QStringLiteral("Back"), this}),
      m_home(new QAction{QStringLiteral("Contents"), this}) {
    setWindowTitle(QStringLiteral("subedit — manual"));
    resize(kWidth, kHeight);

    // **Les liens ne sont pas ouverts par le navigateur de texte.** Le sien
    // appellerait `setSource`, qui lirait un fichier Markdown comme du texte
    // brut ; nous les suivons nous-mêmes, ce qui est aussi ce qui permet de
    // dire ce qu'on ne sait pas ouvrir.
    m_view->setOpenLinks(false);
    connect(m_view, &QTextBrowser::anchorClicked, this, &ManualWindow::followLink);

    connect(m_back, &QAction::triggered, this, [this] {
        if (m_visited.empty())
            return;

        const std::filesystem::path previous = m_visited.back();
        m_visited.pop_back();
        // Ouverte sans être empilée : `openPage` empilerait celle qu'on quitte,
        // et le retour ferait alors du surplace entre deux pages.
        const std::vector<std::filesystem::path> kept = m_visited;
        openPage(previous);
        m_visited = kept;
        refreshActions();
    });

    connect(m_home, &QAction::triggered, this, [this] { openPage(kHomePage); });

    auto* bar = new QToolBar{this};
    bar->addAction(m_back);
    bar->addAction(m_home);

    m_notice->setWordWrap(true);
    m_notice->hide();

    auto* stack = new QVBoxLayout{this};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(bar);
    stack->addWidget(m_notice);
    stack->addWidget(m_view);

    openPage(kHomePage);
    m_visited.clear();
    refreshActions();
}

void ManualWindow::openPage(const std::filesystem::path& page) {
    const std::expected<std::string, core::FileError> content =
        m_files->readFile(m_directory / page);

    if (!content) {
        // **Dit plutôt que tu.** Une installation partielle est le cas que le
        // cadrage nomme, et une fenêtre qui s'ouvre vide laisserait croire à un
        // manuel vide plutôt qu'à un fichier manquant.
        m_notice->setText(QStringLiteral("%1 could not be read.")
                              .arg(QString::fromStdString((m_directory / page).string())));
        m_notice->show();
        return;
    }

    if (!m_current.empty())
        m_visited.push_back(m_current);

    m_notice->clear();
    m_notice->hide();

    // Les images du manuel sont voisines de la page qui les montre, et leurs
    // adresses sont relatives à elle : c'est ce répertoire-là que le rendu doit
    // chercher, et il change à chaque page.
    const std::filesystem::path here = (m_directory / page).parent_path();
    m_view->setSearchPaths({QString::fromStdString(here.string())});

    m_view->document()->setMarkdown(QString::fromStdString(*content),
                                    QTextDocument::MarkdownDialectGitHub);
    m_view->moveCursor(QTextCursor::Start);

    m_current = page;
    refreshActions();
}

void ManualWindow::followLink(const QUrl& target) {
    const std::filesystem::path asked{target.path().toStdString()};

    // Une ancre seule — « #le-thème » — désigne la page courante ; il n'y a rien
    // à charger, et le navigateur y va tout seul.
    if (asked.empty())
        return;

    const std::filesystem::path page = resolved(m_current, asked);

    if (!inside(page) || !m_files->exists(m_directory / page)) {
        m_notice->setText(QStringLiteral("%1 is not part of the installed manual; it lives in the "
                                         "project's repository.")
                              .arg(QString::fromStdString(asked.string())));
        m_notice->show();
        return;
    }

    openPage(page);
}

void ManualWindow::refreshActions() {
    m_back->setEnabled(!m_visited.empty());
    m_home->setEnabled(m_current != kHomePage);
}

QString ManualWindow::shownText() const {
    return m_view->document()->toPlainText();
}

QString ManualWindow::notice() const {
    // `isHidden()` et non `isVisible()` : la seconde est fausse tant que la
    // fenêtre elle-même n'est pas à l'écran, si bien qu'un test qui ne montre
    // rien lirait toujours une bande vide. La première dit ce qu'on veut
    // savoir — la bande a-t-elle été cachée — sans rien demander aux ancêtres.
    return m_notice->isHidden() ? QString{} : m_notice->text();
}

} // namespace subedit::gui
