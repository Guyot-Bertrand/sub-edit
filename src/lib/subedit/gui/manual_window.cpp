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

/// The page the manual opens on.
///
/// The one #243 wrote for it: it says what each of the two programs does, and
/// where to start.
const std::filesystem::path kHomePage{"index.md"};

/// The size it opens at, in pixels.
///
/// Wide enough that the tables of the manual fit without wrapping — there are
/// many of them and it is what they render worst — and tall enough that a
/// section reads without scrolling at every paragraph.
constexpr int kWidth = 900;
constexpr int kHeight = 700;

/// What a page becomes once read, relative to the root of the manual.
///
/// **Normalised, because links are relative to the page that carries them**:
/// `../subedit-cli/installation.md` clicked from `subedit-gui/index.md` has to
/// name a page of the manual and not a path that climbs above it.
[[nodiscard]] std::filesystem::path resolved(const std::filesystem::path& from,
                                             const std::filesystem::path& target) {
    return (from.parent_path() / target).lexically_normal();
}

/// Tells whether `page` stays under the root of the manual.
[[nodiscard]] bool inside(const std::filesystem::path& page) {
    return !page.empty() && *page.begin() != "..";
}

/// The anchor GitHub gives a heading.
///
/// **The same rule as `check-manual-links.py`**, written twice for want of a
/// way to write it once: the script checks the references of the repository,
/// this window follows them, and nothing ties a Python script to a C++ library.
/// The two copies are confronted through the corpus rather than by reading —
/// the manual carries some forty references with an anchor, the script demands
/// that they name a heading, and the test on the real pages demands that the
/// window find them. A divergence between the two rules would fail one of the
/// two.
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

/// The headings of the page and their anchor, in the order they come.
///
/// **Repeats are numbered**, as GitHub does it: two sections named "The errors"
/// in one page give `the-errors` and `the-errors-1`.
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

/// Counts the tables of a frame and of everything it holds.
///
/// Recursive because the document is not flat: a table is a child frame, and it
/// may carry frames of its own. Counting the children of the root alone would
/// do for today's manual and say nothing about tomorrow's.
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

    // **Links are not opened by the text browser.** Its own would call
    // `setSource`, which would read a Markdown file as plain text; we follow
    // them ourselves, which is also what makes it possible to say what cannot
    // be opened.
    m_view->setOpenLinks(false);
    connect(m_view, &QTextBrowser::anchorClicked, this, &ManualWindow::followLink);

    connect(m_back, &QAction::triggered, this, [this] {
        if (m_visited.empty())
            return;

        const std::filesystem::path previous = m_visited.back();
        m_visited.pop_back();
        // Opened without being stacked: `openPage` would stack the one being
        // left, and going back would then shuttle between two pages.
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
        // **Said rather than left unsaid.** A partial installation is the case
        // the scoping names, and a window that opened empty would suggest an
        // empty manual rather than a missing file.
        m_notice->setText(QStringLiteral("%1 could not be read.")
                              .arg(QString::fromStdString((m_directory / page).string())));
        m_notice->show();
        return;
    }

    if (!m_current.empty())
        m_visited.push_back(m_current);

    m_notice->clear();
    m_notice->hide();

    // The images of the manual sit beside the page that shows them, and their
    // addresses are relative to it: that is the directory the rendering has to
    // search, and it changes with every page.
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

    // An anchor on its own — "#the-theme" — names the current page: there is
    // nothing to load, only somewhere to go down to.
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

        // **The view is placed by its cursor, and not by `scrollToAnchor`**:
        // Qt's Markdown rendering names no anchor at all — a heading is a block
        // with a level there, not a target — so there is nothing for
        // `scrollToAnchor` to answer. That is the defect #268 found: the
        // references with an anchor were checked against GitHub's anchors,
        // never against what the window renders, and they opened the page
        // without going down to the section.
        //
        // The detour through the end puts the heading at the top of the view
        // rather than the bottom. `setTextCursor` scrolls just enough to show
        // the cursor: from the beginning of a page, "just enough" brings the
        // heading to the bottom of the window, so the section starts out of
        // sight. From the end, the same "just enough" brings it to the top, and
        // the section reads.
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

    // Block by block, fragment by fragment: it is the only way to read a
    // character format, and a link is nothing else. Iterating a document walks
    // the blocks of the tables too, where the manual puts half of its
    // references.
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

            // `resource()` goes through `QTextBrowser::loadResource`, and so
            // through the search paths `openPage` has just laid down: asking
            // the document for the image is asking what a reader will see,
            // where an `exists()` on disk would say only what the disk holds.
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

    // The loop runs to the end rather than leaving on the first match, and
    // that is what spares it a return line nothing would reach: a heading block
    // is always in this list, so a `return` after the loop would be dead code.
    QString name;
    for (const auto& [block, anchor] : headingAnchors(*m_view->document())) {
        if (block == here.blockNumber())
            name = anchor;
    }

    return name;
}

QString ManualWindow::notice() const {
    // `isHidden()` and not `isVisible()`: the second is false for as long as
    // the window itself is off screen, so a test that shows nothing would
    // always read an empty band. The first says what one wants to know — was
    // the band hidden — without asking anything of the ancestors.
    return m_notice->isHidden() ? QString{} : m_notice->text();
}

} // namespace subedit::gui
