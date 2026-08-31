#include <subedit/core/io/file_system.hpp>
#include <subedit/gui/manual_window.hpp>

#include <QAction>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextImageFormat>
#include <QTextTable>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>

#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

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

/// L'ancre que GitHub donne à un titre.
///
/// **La même règle que `check-manual-links.py`**, et écrite deux fois faute de
/// pouvoir l'être une : le script vérifie les renvois du dépôt, cette fenêtre
/// les suit, et rien ne relie un script Python à une bibliothèque C++. Les deux
/// copies sont confrontées par le corpus plutôt que par la lecture — le manuel
/// porte une quarantaine de renvois avec ancre, le script exige qu'ils
/// désignent un titre, et le test des pages réelles exige que la fenêtre les
/// trouve. Une divergence entre les deux règles ferait échouer l'un des deux.
[[nodiscard]] QString anchorOf(const QString& title) {
    QString kept;
    for (const QChar letter : title.trimmed().toLower()) {
        if (letter == u' ')
            kept.append(u'-');
        else if (letter.isLetterOrNumber() || letter == u'_' || letter == u'-')
            kept.append(letter);
    }
    return kept;
}

/// Les titres de la page et leur ancre, dans l'ordre où ils viennent.
///
/// **Les doublons sont numérotés**, comme GitHub le fait : deux sections « Les
/// erreurs » dans une même page donnent `les-erreurs` et `les-erreurs-1`.
[[nodiscard]] std::vector<std::pair<int, QString>> headingAnchors(const QTextDocument& document) {
    std::vector<std::pair<int, QString>> found;
    std::map<QString, int> seen;

    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (block.blockFormat().headingLevel() == 0)
            continue;

        const QString base = anchorOf(block.text());
        const int rank = seen[base]++;
        found.emplace_back(block.blockNumber(),
                           rank == 0 ? base : base + QStringLiteral("-") + QString::number(rank));
    }

    return found;
}

/// Compte les tableaux d'un cadre et de tout ce qu'il contient.
///
/// Récursif parce que le document n'est pas plat : un tableau est un cadre
/// enfant, et il peut lui-même en porter. Compter les seuls enfants de la
/// racine suffirait au manuel d'aujourd'hui et se tairait sur celui de demain.
[[nodiscard]] int tablesUnder(const QTextFrame& frame) {
    int found = 0;
    for (QTextFrame* child : frame.childFrames()) {
        if (qobject_cast<QTextTable*>(child) != nullptr)
            ++found;
        found += tablesUnder(*child);
    }
    return found;
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

    // Une ancre seule — « #le-thème » — désigne la page courante : il n'y a
    // rien à charger, seulement à descendre.
    if (asked.empty()) {
        showSection(target.fragment());
        return;
    }

    const std::filesystem::path page = resolved(m_current, asked);

    if (!inside(page) || !m_files->exists(m_directory / page)) {
        m_notice->setText(QStringLiteral("%1 is not part of the installed manual; it lives in the "
                                         "project's repository.")
                              .arg(QString::fromStdString(asked.string())));
        m_notice->show();
        return;
    }

    openPage(page);
    showSection(target.fragment());
}

void ManualWindow::showSection(const QString& anchor) {
    if (anchor.isEmpty())
        return;

    for (const auto& [block, name] : headingAnchors(*m_view->document())) {
        if (name != anchor)
            continue;

        // **La vue est posée par son curseur, et non par `scrollToAnchor`** : le
        // rendu Markdown de Qt ne nomme aucune ancre — un titre y est un bloc
        // de niveau, pas une cible — donc il n'y a rien à quoi `scrollToAnchor`
        // pourrait répondre. C'est le défaut que #268 a trouvé : les renvois
        // avec ancre du manuel étaient vérifiés contre les ancres de GitHub,
        // jamais contre le rendu de la fenêtre, et ils y ouvraient la page sans
        // y descendre.
        //
        // Le détour par la fin met le titre en haut de la vue plutôt qu'en bas.
        // `setTextCursor` fait défiler juste assez pour montrer le curseur :
        // depuis le début d'une page, « juste assez » amène le titre au bas de
        // la fenêtre, et la section commence donc hors champ. Depuis la fin, le
        // même « juste assez » l'amène en haut, et la section se lit.
        m_view->moveCursor(QTextCursor::End);
        m_view->setTextCursor(QTextCursor{m_view->document()->findBlockByNumber(block)});
        return;
    }
}

void ManualWindow::refreshActions() {
    m_back->setEnabled(!m_visited.empty());
    m_home->setEnabled(m_current != kHomePage);
}

QString ManualWindow::shownText() const {
    return m_view->document()->toPlainText();
}

int ManualWindow::shownTables() const {
    return tablesUnder(*m_view->document()->rootFrame());
}

QStringList ManualWindow::shownLinks() const {
    QStringList found;

    // Bloc par bloc, fragment par fragment : c'est la seule façon de lire un
    // format de caractère, et un lien n'est rien d'autre. L'itération d'un
    // document parcourt aussi les blocs des tableaux, où le manuel met la
    // moitié de ses renvois.
    for (QTextBlock block = m_view->document()->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator part = block.begin(); part != block.end(); ++part) {
            const QTextFragment fragment = part.fragment();
            if (!fragment.isValid() || !fragment.charFormat().isAnchor())
                continue;

            const QString href = fragment.charFormat().anchorHref();
            if (!href.isEmpty())
                found.append(href);
        }
    }

    return found;
}

QStringList ManualWindow::missingImages() const {
    QStringList absent;

    for (QTextBlock block = m_view->document()->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator part = block.begin(); part != block.end(); ++part) {
            const QTextFragment fragment = part.fragment();
            if (!fragment.isValid() || !fragment.charFormat().isImageFormat())
                continue;

            const QString name = fragment.charFormat().toImageFormat().name();

            // `resource()` passe par `QTextBrowser::loadResource`, donc par les
            // chemins de recherche que `openPage` vient de poser : demander
            // l'image au document est demander ce qu'un lecteur verra, là où un
            // `exists()` sur le disque ne dirait que ce que le disque porte.
            const QVariant loaded =
                m_view->document()->resource(QTextDocument::ImageResource, QUrl{name});

            if (loaded.value<QImage>().isNull())
                absent.append(name);
        }
    }

    return absent;
}

QString ManualWindow::currentSection() const {
    const QTextBlock here = m_view->textCursor().block();
    if (here.blockFormat().headingLevel() == 0)
        return {};

    // La boucle va jusqu'au bout plutôt que de sortir sur la première
    // correspondance, et c'est ce qui lui évite une ligne de retour que rien
    // n'atteindrait : un bloc de titre est toujours dans cette liste, donc un
    // `return` après la boucle serait du code mort.
    QString name;
    for (const auto& [block, anchor] : headingAnchors(*m_view->document())) {
        if (block == here.blockNumber())
            name = anchor;
    }

    return name;
}

QString ManualWindow::notice() const {
    // `isHidden()` et non `isVisible()` : la seconde est fausse tant que la
    // fenêtre elle-même n'est pas à l'écran, si bien qu'un test qui ne montre
    // rien lirait toujours une bande vide. La première dit ce qu'on veut
    // savoir — la bande a-t-elle été cachée — sans rien demander aux ancêtres.
    return m_notice->isHidden() ? QString{} : m_notice->text();
}

} // namespace subedit::gui
