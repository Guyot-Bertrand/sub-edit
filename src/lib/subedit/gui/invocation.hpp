#pragma once

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>

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

/// Opens what the command line names, and says on `errors` what it could not.
///
/// An empty document when nothing was named, and **an empty document when what
/// was named could not be read** — which is the decision this carries: a file
/// that will not open is a reason to say so, never a reason to refuse to start.
/// The window opens either way, and the user opens something else from it.
///
/// **It says which way it failed**, in the same words the command line uses:
/// the file does not exist, permission was refused, it cannot be read, it is
/// not valid UTF-8, no format claims it. One sentence for all of them was the
/// defect #154 came for — only the last of the causes was ever the one it
/// named.
///
/// Only the first argument is read. A second file would be a second window,
/// and this program has one.
///
/// Here rather than in `main.cpp` for the reason `check-architecture.sh`
/// holds, and it said so twice: the entry point went over its budget the day
/// it also had to choose a Qt platform, and again the day it had to hand the
/// window a reader of frame rates.
[[nodiscard]] core::OpenedFile openFromArguments(const core::FileSystem& files,
                                                 const QStringList& arguments,
                                                 std::ostream& errors);

/// Reads the settings at `path`, and says on `errors` what it could not read.
///
/// **Never answers an error, and that is the decision of ADR 0022**: a
/// configuration is a comfort, and its failing must cost the comfort and
/// nothing else. A missing file, a refused file, an unreadable value all give
/// usable settings — the defaults for what is missing, what could be read for
/// the rest.
///
/// **The diagnostic goes to the error output**, as Gaupol does from its own
/// interface. A dialog at start-up for an unreadable preference would be a bad
/// bargain: it stops the user over a defect that prevents nothing, and it
/// arrives before they have asked for anything.
///
/// Here rather than in `main.cpp` for the reason `check-architecture.sh` holds:
/// an entry point wires, it does not shape messages.
[[nodiscard]] core::Settings readUserSettings(const core::FileSystem& files,
                                              const std::filesystem::path& path,
                                              std::ostream& errors);

/// The same, at the place this user's settings live.
///
/// **Two overloads rather than a default argument**, and the difference is one
/// of a seam: this one resolves the location — the one thing `main.cpp` is
/// allowed not to say — where the one above receives it and can therefore be
/// put to the test without anything going near a real home directory. See
/// `settings_path.hpp` and ADR 0022.
[[nodiscard]] core::Settings readUserSettings(const core::FileSystem& files, std::ostream& errors);

/// Writes `settings` to `path`, and says on `errors` if it could not.
///
/// The counterpart of the one above, and it loses the same way: a refused
/// write is a session whose settings will not be found again, not a reason to
/// end badly.
void writeUserSettings(core::FileSystem& files,
                       const std::filesystem::path& path,
                       const core::Settings& settings,
                       std::ostream& errors);

/// The same, at the place this user's settings live.
void writeUserSettings(core::FileSystem& files,
                       const core::Settings& settings,
                       std::ostream& errors);

} // namespace subedit::gui
