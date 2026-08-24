#pragma once

#include <QStringList>

#include <iosfwd>

// `QStringList` is an alias for `QList<QString>` and not a class, so it cannot
// be declared by hand the way `QWidget` is elsewhere in this directory.

namespace subedit::gui {

/// Answers `--version` if that is what was asked, and says whether it was.
///
/// **Here rather than in `main.cpp`**, which is the rule
/// `check-architecture.sh` holds and which it enforced out loud: the entry
/// point went over its budget the day it also had to choose a Qt platform, and
/// this was the logic sitting in it. Out here it is a function a test can call,
/// where before only a whole process could.
///
/// `out` is where the line goes — `std::cout` for the program, a string stream
/// for a test.
///
/// Returns whether the version was asked for, which is also whether there is
/// anything left to do: `--version` is answered and nothing else happens.
[[nodiscard]] bool reportVersion(const QStringList& arguments, std::ostream& out);

} // namespace subedit::gui
