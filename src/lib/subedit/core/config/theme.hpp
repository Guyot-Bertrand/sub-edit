#pragma once

namespace subedit::core {

/// Which palette the window wears.
///
/// **Trois valeurs, et la première ne fait rien** — décision D3 du cadrage de la
/// phase 7. Qt est en 6.4.2 sur la cible et n'a aucune API de schéma de
/// couleurs : `QStyleHints::colorScheme` est arrivée en 6.5. L'application ne
/// peut donc ni demander au système ce qu'il préfère, ni être prévenue quand il
/// change d'avis.
///
/// Gaupol fait de la même contrainte la même chose, une version en avance : son
/// thème « système » n'est résolu que sur GTK 4.20 et plus, et en dessous sa
/// mise à jour ne fait rien du tout. C'est ce qui transforme la contrainte en
/// comportement conçu plutôt qu'en impasse — on livre les deux thèmes que
/// l'utilisateur peut demander, et on n'invente pas une lecture du système
/// qu'on ne sait pas faire.
///
/// **Et c'est cette forme qui rend le thème éprouvable** : parce que clair et
/// sombre sont des palettes que nous posons, un test peut les poser aussi et
/// lire ce qu'il obtient.
enum class Theme {
    System, ///< rien n'est posé : la palette reste celle de la plate-forme
    Light,
    Dark,
};

} // namespace subedit::core
