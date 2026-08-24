// What `subedit-gui` answers before it opens anything.
//
// It lives in the library rather than in `main.cpp` so that a test can call it
// — before, only a whole process could, and the end-to-end harness had to
// launch one to read a single line.

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
