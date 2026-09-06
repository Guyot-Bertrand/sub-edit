// The real manual, opened in the real window — issue #268.
//
// **This file exists because its neighbour was not enough.**
// `manual_window_test.cpp` puts the window to the test on a manual written in
// the test: two pages, a table of two rows, three links. It is the right manual
// for testing a window — it is small, it is right there, and it moves only when
// it is written.
//
// It is not the right manual for testing the *rendering*. Decision D6 of the
// scoping of phase 7 asked a question — "our sections use tables, and Qt's
// Markdown rendering has its limits; if they bite, the alternative is rendering
// HTML at build time" — and the answer was taken on that table of two rows. The
// twenty-four real pages carry block quotes, nested lists, `console` blocks,
// images, tables of five columns and relative links two levels deep. What MD4C
// made of them was checked nowhere.
//
// **The pages read are those of the repository, and the installed copy is this
// one page for page** — `cmake/Installation.cmake` lays down the whole of
// `docs/manual/`, and `check-installation.sh` checks that none is missing. The
// path is given at build time, as it is to the screenshot program: nothing here
// resolves a location.

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

/// The root of the repository's manual, given by the build system.
[[nodiscard]] std::filesystem::path manualRoot() {
    return std::filesystem::path{SUBEDIT_MANUAL_DIR};
}

/// Every page of the manual, relative to its root, in a stable order.
///
/// **Discovered and not listed.** A list written here would go stale at the
/// first chapter added, and the gap would show only if somebody thought to
/// write it in — which is to say never. It is the rule `check-installation.sh`
/// already holds itself to, for the same reason.
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

/// How many tables the source of a page declares.
///
/// Counted on the separator row, the one that follows the header: it is the one
/// row a Markdown table cannot do without, and the one an ordinary paragraph
/// cannot have by accident.
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

/// Tells whether a reference leaves the manual — the network, or a document of
/// the repository.
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
    // **The dullest check, and the one that was missing.** A page whose
    // rendering is empty opens exactly like a page nobody wrote: the window
    // says nothing, since it did read the file.
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

        // The title of the page is its first line, and it survives the
        // rendering: that is what tells "rendered" from "rendered in part". Its
        // backticks go with the rendering — half the titles of the command line
        // manual are the name of a subcommand, written as code.
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
    // **The question of decision D6, asked of the real pages.** It had been
    // answered on a table of two rows written in a test; the manual carries
    // ninety, some of five columns, some with code and links inside their
    // cells. If the limits of Qt's Markdown rendering bite anywhere, it is
    // there.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    int total = 0;

    for (const std::filesystem::path& page : everyPage()) {
        INFO("page : " << page.string());
        manual.openPage(page);

        const int declared = tablesDeclaredIn(sourceOf(files, page));
        CHECK(manual.shownTables() == declared);
        total += declared;

        // And the separator bars do not stay in the text: that would be the
        // sign of a dialect leaving the tables as they are.
        CHECK_THAT(manual.shownText().toStdString(), !ContainsSubstring("| :---"));
    }

    // A guard against the worst kind of green: a heuristic that counted
    // nothing would let every equality above pass.
    CHECK(total > 50);
}

TEST_CASE("every manual image loads in the page that shows it", "[gui][GUI-MANUAL-01]") {
    // The screenshots live in a subdirectory beside the page, and the
    // rendering finds them only through the search paths `openPage` lays down.
    // A path laid down for the wrong page would give a page that shows without
    // its images, without a word.
    RealFileSystem files;
    ManualWindow manual{files, manualRoot()};

    for (const std::filesystem::path& page : everyPage()) {
        INFO("page : " << page.string());
        manual.openPage(page);

        CHECK(manual.missingImages().join(QStringLiteral(", ")).toStdString().empty());
    }
}

TEST_CASE("every manual link resolves, or is reported", "[gui][GUI-MANUAL-01]") {
    // **What `check-manual-links.py` cannot say.** It confronts the references
    // of the source with what exists on disk, from the root of the repository —
    // a point of view the window does not have. It sees the installed manual,
    // where the roadmap and the ADRs do not exist, and it resolves the relative
    // paths itself. Two different resolutions of the same link, and only one
    // was being tested.
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

            // The link is followed from the page that carries it: that is what
            // a click does, and the current page is what the window resolves
            // against. The page is therefore reopened before each one, the
            // previous having left it.
            manual.openPage(page);
            REQUIRE(manual.currentPage() == page);

            manual.followLink(QUrl{href});

            if (leavesTheManual(page, href)) {
                // Said, never followed: the roadmap and the ADRs live in the
                // repository and are not installed.
                CHECK(manual.currentPage() == page);
                CHECK_THAT(manual.notice().toStdString(),
                           ContainsSubstring("not part of the installed manual"));
                ++said;
                continue;
            }

            if (href.startsWith('#')) {
                // An anchor alone changes no page: it goes down in it.
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
                // **What the reference promises beyond a page**: the section.
                // Qt's Markdown rendering names no anchor, so these references
                // opened the page without going down in it — that is the defect
                // #268 found, and the window follows them now.
                CHECK(manual.currentSection() == QUrl{href}.fragment());
                ++descended;
            }

            ++followed;
        }
    }

    // The three counts guard one another: an extraction of links that answered
    // an empty list would let the whole loop above pass.
    CHECK(followed > 100);
    CHECK(said > 0);
    CHECK(descended > 30);
}
