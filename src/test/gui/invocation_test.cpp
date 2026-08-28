// What `subedit-gui` answers before it opens anything.
//
// It lives in the library rather than in `main.cpp` so that a test can call it
// — before, only a whole process could, and the end-to-end harness had to
// launch one to read a single line.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/version.hpp>
#include <subedit/gui/invocation.hpp>

#include <QString>
#include <QStringList>
#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

TEST_CASE("the version asked for is written, and nothing else happens", "[gui]") {
    std::ostringstream written;

    const bool answered =
        subedit::gui::reportVersion(QStringList{QStringLiteral("--version")}, written);

    CHECK(answered);
    // Derived, never spelled out: a number written twice is a number that ends
    // up wrong in one of the two places.
    CHECK(written.str() == "subedit " + std::string{subedit::core::versionString()} + "\n");
}

TEST_CASE("any other argument is not the version being asked for", "[gui]") {
    std::ostringstream written;

    const bool answered =
        subedit::gui::reportVersion(QStringList{QStringLiteral("film.srt")}, written);

    CHECK_FALSE(answered);
    CHECK(written.str().empty());
}

// What the command line names, opened — or said to be unopenable.

TEST_CASE("the file named on the command line is opened", "[gui]") {
    subedit::core::InMemoryFileSystem files;
    files.addFile("/films/film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    std::ostringstream errors;

    const subedit::core::OpenedFile opened = subedit::gui::openFromArguments(
        files,
        QStringList{QStringLiteral("subedit-gui"), QStringLiteral("/films/film.srt")},
        errors);

    CHECK(opened.project.count() == 1U);
    CHECK(errors.str().empty());
}

TEST_CASE("naming nothing opens an empty document", "[gui]") {
    const subedit::core::InMemoryFileSystem files;
    std::ostringstream errors;

    const subedit::core::OpenedFile opened =
        subedit::gui::openFromArguments(files, QStringList{QStringLiteral("subedit-gui")}, errors);

    CHECK(opened.project.count() == 0U);
    CHECK(errors.str().empty());
}

// **The window opens either way**, and that is the decision this carries: a
// file that will not open is a reason to say so, never a reason to refuse to
// start.
TEST_CASE("a file that will not open is said, and the document is empty", "[gui]") {
    const subedit::core::InMemoryFileSystem files;
    std::ostringstream errors;

    const subedit::core::OpenedFile opened = subedit::gui::openFromArguments(
        files, QStringList{QStringLiteral("subedit-gui"), QStringLiteral("/absent.srt")}, errors);

    CHECK(opened.project.count() == 0U);
    CHECK(errors.str() == "subedit-gui: /absent.srt: does not exist\n");
}

// **The cause, and not one sentence for all of them** — issue #154. It used to
// write « nothing to open » whatever had happened, so this case and the one
// above would have been indistinguishable; only one of the two messages was
// ever true.
TEST_CASE("a file in no format this tool knows is said to be exactly that", "[gui]") {
    subedit::core::InMemoryFileSystem files;
    files.addFile("/films/notes.txt", "rien de reconnaissable\n");
    std::ostringstream errors;

    const subedit::core::OpenedFile opened = subedit::gui::openFromArguments(
        files,
        QStringList{QStringLiteral("subedit-gui"), QStringLiteral("/films/notes.txt")},
        errors);

    CHECK(opened.project.count() == 0U);
    CHECK(errors.str() == "subedit-gui: /films/notes.txt: is in no format this tool knows\n");
}
