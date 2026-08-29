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
/// **Ne rend jamais d'erreur, et c'est la décision de l'ADR 0022** : une
/// configuration est un confort, sa défaillance doit coûter le confort et rien
/// d'autre. Un fichier absent, un fichier refusé, une valeur illisible donnent
/// tous des réglages utilisables — les défauts pour ce qui manque, ce qui se
/// lisait pour le reste.
///
/// **Le diagnostic va sur la sortie d'erreur**, comme Gaupol le fait depuis son
/// interface. Une modale au démarrage pour une préférence illisible serait un
/// mauvais échange : elle arrête l'utilisateur pour un défaut qui ne l'empêche
/// de rien, et elle arrive avant qu'il ait rien demandé.
///
/// Ici plutôt que dans `main.cpp` pour la raison que `check-architecture.sh`
/// tient : un point d'entrée câble, il ne met pas en forme des messages.
[[nodiscard]] core::Settings readUserSettings(const core::FileSystem& files,
                                              const std::filesystem::path& path,
                                              std::ostream& errors);

/// The same, at the place this user's settings live.
///
/// **Deux surcharges plutôt qu'un défaut d'argument**, et la différence est
/// celle d'une couture : celle-ci résout l'emplacement — c'est la seule chose
/// que `main.cpp` a le droit de ne pas dire —, celle du dessus le reçoit et se
/// laisse donc éprouver sans que rien n'aille voir un vrai répertoire
/// personnel. Voir `settings_path.hpp` et l'ADR 0022.
[[nodiscard]] core::Settings readUserSettings(const core::FileSystem& files, std::ostream& errors);

/// Writes `settings` to `path`, and says on `errors` if it could not.
///
/// Le pendant du précédent, et il perd de la même façon : une écriture refusée
/// est une session dont les réglages ne seront pas retrouvés, pas une raison de
/// mal se terminer.
void writeUserSettings(core::FileSystem& files,
                       const std::filesystem::path& path,
                       const core::Settings& settings,
                       std::ostream& errors);

/// The same, at the place this user's settings live.
void writeUserSettings(core::FileSystem& files,
                       const core::Settings& settings,
                       std::ostream& errors);

} // namespace subedit::gui
