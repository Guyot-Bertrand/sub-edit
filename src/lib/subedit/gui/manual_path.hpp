#pragma once

#include <filesystem>

namespace subedit::gui {

/// Où le manuel installé se trouve, qu'il y soit ou non.
///
/// **Déduit de l'exécutable, et jamais d'un chemin figé à la compilation.** Le
/// préfixe de configuration et celui d'installation ne sont pas le même : le
/// manuel décrit `cmake --install build/release --prefix ~/.local`, où le
/// second vaut `~/.local` alors que le premier est resté `/usr/local`. Un
/// chemin gravé à la compilation désignerait donc le mauvais endroit dans
/// l'usage même que le manuel recommande.
///
/// `<répertoire de l'exécutable>/../share/subedit/manual` est juste pour tous
/// les préfixes — `/usr` du paquet, `/usr/local`, `~/.local`, le répertoire
/// temporaire de `check-installation.sh` — parce que c'est la disposition que
/// `GNUInstallDirs` produit, quel que soit le préfixe.
///
/// **Le même rôle que `userSettingsPath()`, et la même règle** — ADR 0022 : ceci
/// est le seul code qui résout cet emplacement, `main.cpp` l'appelle et passe
/// la réponse à la fenêtre. Aucun test n'atteint donc le vrai manuel par
/// accident : il donne le chemin qu'il veut.
///
/// Absolu, et il peut ne pas exister — un binaire lancé depuis l'arbre de
/// construction n'a pas de manuel à côté de lui, ce qui n'est pas une erreur.
[[nodiscard]] std::filesystem::path installedManualPath();

} // namespace subedit::gui
