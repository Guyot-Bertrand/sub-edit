// The installed manual, read in the window — issue #245.
//
// **Nothing here touches a real manual.** The manual of these cases is written
// in memory, and its path is given: it is the seam of ADR 0022, the same one as
// for the settings. `installedManualPath()` is the only code that resolves the
// real location, and it has a case of its own further down.

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

/// A page showing an image nothing lays on disk.
///
/// **Images do not go through the window's file system**: the rendering looks
/// for them itself, in the directory of the page. A page in memory therefore
/// has no images, and that is what makes this case reachable without writing
/// anything.
constexpr const char* kSansImage = "# Une capture manquante\n"
                                   "\n"
                                   "![La fenêtre](captures/absente.png)\n";

/// A page that does not begin with a heading.
///
/// It exists for one case: the view sitting somewhere other than on a heading,
/// which the pages of the manual never produce — they all begin with their
/// name.
constexpr const char* kSansTitre = "Rien qu'un paragraphe.\n";

/// A manual in memory, at the location the cases give themselves.
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
    // **The risk the scoping named**: "our sections use tables, and Qt's
    // Markdown rendering has its limits". They do not bite — the GitHub dialect
    // makes real tables, and the content of the cells is there. The alternative
    // of rendering HTML at build time therefore never had to be argued.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    const std::string shown = manual.shownText().toStdString();
    CHECK_THAT(shown, ContainsSubstring("Programme"));
    CHECK_THAT(shown, ContainsSubstring("subedit-gui"));
    CHECK_THAT(shown, ContainsSubstring("la fenêtre"));
    CHECK_THAT(shown, ContainsSubstring("le lot"));

    // And the bars of the table are not rendered as they are: that would be
    // the sign of a dialect ignoring tables and leaving them as text.
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
    // The path of a real click: the link is relative to the page carrying it,
    // and that is what sets this case apart from the direct opening just
    // above.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});

    // And from there a link that climbs one level leads back to the home page:
    // it is the relative resolution that does it, not a table of mappings.
    manual.followLink(QUrl{QStringLiteral("../index.md")});

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("a link leaving the installed manual is reported, never followed",
          "[gui][GUI-MANUAL-01]") {
    // **The rule the window carries alone.** The manual points at the roadmap
    // and the ADRs, which live in the repository and are not installed. A click
    // with no effect would look like something broken.
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
    // "#the-theme" names the current page: there is nothing to load, and the
    // window goes down in it.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/table.md");
    manual.followLink(QUrl{QStringLiteral("#les-anomalies")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.notice().isEmpty());
    CHECK(manual.currentSection() == QStringLiteral("les-anomalies"));
}

TEST_CASE("a link with an anchor opens the page and scrolls to it", "[gui][GUI-MANUAL-01]") {
    // **The defect #268 found.** Qt's Markdown rendering names no anchor — a
    // heading is a block with a level there, not a target — so the references
    // of the manual opened the right page and left it at its beginning. The
    // manual carries some forty of them, all checked until then against
    // GitHub's anchors and never against the window's.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#les-anomalies")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.currentSection() == QStringLiteral("les-anomalies"));
}

TEST_CASE("two identical headings give two anchors", "[gui][GUI-MANUAL-01]") {
    // GitHub's rule, which `check-manual-links.py` holds to on its own side:
    // the second "The errors" of a page is called `the-errors-1`.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#les-erreurs-1")});

    CHECK(manual.currentSection() == QStringLiteral("les-erreurs-1"));
    CHECK_THAT(manual.shownText().toStdString(), ContainsSubstring("Les secondes"));
}

TEST_CASE("an anchor naming nothing leaves the page at its start", "[gui][GUI-MANUAL-01]") {
    // **Silent, unlike the rest of this window.** The manual ships with the
    // program and is not written by whoever uses it: a dead anchor is a defect
    // of the repository, which `check-manual-links.py` and the test on the real
    // pages both refuse. The message would never have a reader.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.followLink(QUrl{QStringLiteral("subedit-gui/table.md#une-section-effacee")});

    CHECK(manual.currentPage() == std::filesystem::path{"subedit-gui/table.md"});
    CHECK(manual.currentSection() == QStringLiteral("la-table"));
    CHECK(manual.notice().isEmpty());
}

TEST_CASE("an image the rendering cannot find is named", "[gui][GUI-MANUAL-01]") {
    // **The counterpart of the test on the real pages**, which demands that
    // the manual of the repository have none. Without this case, nothing would
    // say the check can answer anything other than "nothing is missing".
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    manual.openPage("subedit-gui/sans-image.md");

    CHECK(manual.missingImages() == QStringList{QStringLiteral("captures/absente.png")});
}

TEST_CASE("the rendered tables can be counted", "[gui][GUI-MANUAL-01]") {
    // The count, and not the text of the cells alone: it is what tells a real
    // `QTextTable` from a table left as text, and it is what the test on the
    // real pages confronts with what each source declares.
    InMemoryFileSystem files = withManual();
    ManualWindow manual{files, kManual};

    CHECK(manual.shownTables() == 1);

    manual.openPage("subedit-gui/table.md");

    CHECK(manual.shownTables() == 0);
}

TEST_CASE("the rendered links are read from the document", "[gui][GUI-MANUAL-01]") {
    // Those the rendering made, and not those the source writes: a reference
    // the Markdown failed to recognise would not be in this list.
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
    // The second guard, the one the action being out hides: `trigger()` on an
    // action that is out fires nothing, so this path has to be reached by
    // switching it back on by hand. It exists all the same.
    InMemoryFileSystem files = withManual();
    const ManualWindow manual{files, kManual};

    manual.backAction()->setEnabled(true);
    manual.backAction()->trigger();

    CHECK(manual.currentPage() == std::filesystem::path{"index.md"});
}

TEST_CASE("a missing page is reported, and replaces nothing", "[gui][GUI-MANUAL-01]") {
    // The case of a partial installation: the window says so and keeps what it
    // was showing.
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

    // Out for as long as nobody has said where to look: it is the state of a
    // binary run from the build tree.
    CHECK_FALSE(window.manualAction()->isEnabled());

    window.setManualPath(kManual);

    CHECK(window.manualAction()->isEnabled());
}

TEST_CASE("the entry stays disabled on a partial installation", "[gui][GUI-MANUAL-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    // A manual whose home page is missing: the directory is, the page is not.
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
    // **Worked out and not carved in**: the configure prefix and the install
    // prefix are not the same, and the manual recommends precisely an
    // installation under another prefix.
    const std::filesystem::path resolved = installedManualPath();

    CHECK(resolved.is_absolute());
    CHECK(resolved.filename() == "manual");
    CHECK(resolved.parent_path().filename() == "subedit");
    CHECK(resolved.parent_path().parent_path().filename() == "share");

    // Beside the executable, and not under it: `bin` and `share` are
    // siblings, which is what `GNUInstallDirs` produces whatever the prefix.
    const std::filesystem::path binaries{QApplication::applicationDirPath().toStdString()};
    CHECK(resolved.parent_path().parent_path().parent_path() == binaries.parent_path());
}
