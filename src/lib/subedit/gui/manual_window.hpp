#pragma once

#include <QStringList>
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

/// The installed manual, read in a window.
///
/// **The installed manual and not a URL** — decision D6 of the scoping of phase
/// 7. Gaupol opens an address; we do not follow it there, because a URL
/// describes `main` and not the version at hand. A user reading the manual of a
/// version they do not have is worse served than a user with no manual.
///
/// **Markdown rendered by Qt, with no new dependency.**
/// `QTextDocument::setMarkdown` does it, in the GitHub dialect, and the scoping
/// asked to check early that our tables survive: they do — they become real
/// `QTextTable`s, rows and columns included. The alternative of rendering HTML
/// at build time therefore never had to be argued.
///
/// **A window and not a dialog.** One consults a manual *while* working, and a
/// modal dialog would forbid exactly that. It escapes `Prompts` for that
/// reason, and it costs the tests nothing: that seam exists to keep `exec()`
/// out of their reach, and nothing here calls it.
class ManualWindow final : public QWidget {
    Q_OBJECT

public:
    /// Opens the manual living under `directory`, on its home page.
    ///
    /// `files` must outlive it. The manual is read through that rather than by
    /// Qt: it is what lets a test lay down a manual in memory, with no real
    /// directory and no real file.
    ManualWindow(core::FileSystem& files,
                 std::filesystem::path directory,
                 QWidget* parent = nullptr);

    /// The page open, relative to the root of the manual.
    [[nodiscard]] std::filesystem::path currentPage() const { return m_current; }

    /// What the page shows, text alone — what a test reads in a reader's
    /// stead.
    [[nodiscard]] QString shownText() const;

    /// How many tables the rendered page carries — issue #268.
    ///
    /// **The text alone does not say.** A dialect that ignored tables would
    /// leave their bars in the text, and the content of the cells would be
    /// there all the same: `shownText()` would pass. What the scoping wanted to
    /// know is whether MD4C makes *real* `QTextTable`s of them, which is
    /// readable only in the structure of the document.
    [[nodiscard]] int shownTables() const;

    /// The addresses the rendered page offers to a click, in the order they
    /// appear — issue #268.
    ///
    /// **Those of the rendered document, and not those of the source.** A link
    /// the Markdown failed to recognise would not be here: that is the
    /// difference between checking that the manual *writes* a reference and
    /// checking that a reader can follow it. Repeats are kept, a page being
    /// free to point twice at the same place.
    [[nodiscard]] QStringList shownLinks() const;

    /// The images the page shows and the rendering could not load — issue
    /// #268.
    ///
    /// Empty when everything loads. It is what puts `setSearchPaths` to the
    /// test: an image exists on disk and stays out of the document's reach if
    /// the directory searched is not the one of the page.
    [[nodiscard]] QStringList missingImages() const;

    /// The section the view sits on, as an anchor — issue #268.
    ///
    /// It is what a test reads to know where a reference led. A page opened at
    /// its beginning answers the anchor of its title, the view sitting on it;
    /// it answers an empty string if the view is anywhere but on a heading,
    /// which no path of this window produces today.
    [[nodiscard]] QString currentSection() const;

    /// The back action, for a test to trigger it and read its state.
    [[nodiscard]] QAction* backAction() const { return m_back; }

    [[nodiscard]] QAction* homeAction() const { return m_home; }

    /// What the window says when it cannot open something, or nothing. A band
    /// under the toolbar, wiped as soon as a page opens.
    [[nodiscard]] QString notice() const;

    /// Opens `page`, relative to the root of the manual. Public so that a test
    /// navigates without clicking.
    void openPage(const std::filesystem::path& page);

    /// Follows a link of the current page — what a click triggers.
    ///
    /// **A link out of the installed manual is said, never followed.** The
    /// manual points eight times at the roadmap and the ADRs, which are
    /// documents of the repository and not of the package: they exist where the
    /// manual is written, not where it is installed. Saying nothing would leave
    /// a click looking like one that did nothing.
    ///
    /// Public for the reason `openPage` is, and one more: the rule above lives
    /// here, so here is where a test has to be able to put it to the test.
    void followLink(const QUrl& target);

private:
    /// Works out afresh what the two actions are allowed to do.
    void refreshActions();

    /// Sets the view on the heading `anchor` names, or leaves it where it is.
    ///
    /// **Silent when the anchor names nothing**, unlike the rest of this
    /// window, which says what it cannot open. The manual ships with the
    /// program and is not written by whoever uses it: a dead anchor is a defect
    /// of the repository, which `check-manual-links.py` and the test on the
    /// real pages both refuse. The message would therefore never have a
    /// reader.
    void showSection(const QString& anchor);

    core::FileSystem* m_files;
    std::filesystem::path m_directory;
    std::filesystem::path m_current;

    /// The pages already opened, the last of them at the head of what is left
    /// to undo.
    ///
    /// A stack of our own rather than `QTextBrowser`'s: its own follows
    /// `setSource`, which would load the Markdown as plain text. We load it
    /// ourselves, so we remember it ourselves.
    std::vector<std::filesystem::path> m_visited;

    QTextBrowser* m_view;
    QLabel* m_notice;
    QAction* m_back;
    QAction* m_home;
};

} // namespace subedit::gui
