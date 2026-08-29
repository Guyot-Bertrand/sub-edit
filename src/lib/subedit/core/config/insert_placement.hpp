#pragma once

namespace subedit::core {

/// De quel côté de la sélection une insertion pose ses lignes.
///
/// **Deux valeurs et un réglage retenu, parce que Gaupol en fait un** —
/// décision D7 du cadrage de la phase 7. `subtitle_insert.above` y est une
/// préférence persistée depuis vingt ans, et la raison tient à l'usage : on
/// n'insère pas une fois, on insère dix lignes de suite, toujours du même
/// côté. Redemander le côté à chaque fois serait redemander une réponse qui ne
/// change jamais.
///
/// Une énumération et non un booléen, pour la raison qui vaut pour `Theme` : le
/// fichier de configuration porte des mots plutôt qu'un `true` dont personne ne
/// devine de quoi il est vrai.
enum class InsertPlacement {
    Above, ///< avant la sélection
    Below, ///< après elle, ce qui est le défaut
};

} // namespace subedit::core
