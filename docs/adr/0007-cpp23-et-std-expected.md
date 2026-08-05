# 0007 — Passer le projet en C++23 et rapporter les erreurs par `std::expected`

**Date :** 2026-08-05
**Statut :** acceptée — remplace partiellement [0001](0001-cpp20-et-qt6.md)

## Contexte

Un éditeur de sous-titres passe son temps à lire des fichiers imparfaits. La
décision [0008](0008-lecture-au-mieux-avec-diagnostics.md) impose d'ouvrir au
mieux **et** de rapporter ce qui n'a pas été compris : une fonction de lecture
doit donc pouvoir renvoyer soit un résultat, soit une erreur, et dans les deux
cas une liste de diagnostics.

Cette forme se modélise naturellement par un type résultat. `std::expected`
relève de C++23, alors que [0001](0001-cpp20-et-qt6.md) retenait C++20.

## Décision

Le projet passe en **C++23**, et les fonctions dont l'échec est un résultat
attendu renvoient `std::expected<T, E>`.

Les exceptions restent réservées à ce qui est véritablement exceptionnel :
épuisement mémoire, violation d'invariant interne, erreur de programmation.

## Vérifications faites avant de décider

Mesuré le 2026-08-05 sur la machine cible, plutôt que supposé :

- **`std::expected` fonctionne avec le GCC 13.3 déjà installé**, en
  `-std=c++23`. Aucun changement de compilateur, aucun paquet supplémentaire.
- **clang-tidy 18 ne voyait pas le type.** libstdc++ garde `<expected>` derrière
  `__cpp_concepts >= 202002L`, valeur que Clang 18 ne déclare pas : le contenu
  de l'en-tête disparaissait, et l'analyse statique aurait été aveugle sur tout
  fichier l'utilisant.
- **clang-tidy 20.1.2, présent dans les dépôts Ubuntu, le lit sans erreur.** La
  chaîne d'outils a été mise à jour en conséquence.

Le mode C++23 de GCC 13 reste partiel : `std::print`, *deducing this*,
`std::ranges::to` et `std::generator` exigent GCC 14, disponible dans les
dépôts. Aucun n'est nécessaire ; le jour où l'un le deviendrait, un paquet
suffirait.

## Alternatives écartées

- **Type résultat maison en C++20** — une trentaine de lignes, aucune dépendance
  à un mode de norme partiel. Écarté parce que le coût réel n'est pas d'écrire
  le type mais de le maintenir et d'apprendre sa sémantique à chaque lecteur,
  alors que `std::expected` est documenté partout et connu.
- **Exceptions** — le plus court à écrire. Écarté sur deux points : une exception
  porte mal une **liste** de diagnostics accompagnant un résultat partiel, ce qui
  est précisément notre cas ; et propager une exception par ligne malformée sur
  un fichier de plusieurs milliers de lignes coûte cher sur un chemin chaud.
- **Codes de retour avec paramètre de sortie** — force le résultat à être
  construit avant de savoir s'il est valide, et se contourne silencieusement en
  ignorant le code. `[[nodiscard]]` sur `std::expected` rend l'oubli visible.
- **Contourner clang-tidy 18 par `-D__cpp_concepts=202002L`** — testé,
  fonctionnel. Écarté parce qu'il fait croire à l'analyseur qu'il supporte un
  niveau de *concepts* qu'il n'a pas, et le fait donc suivre des chemins de
  libstdc++ qu'il gère mal. Corriger un diagnostic en mentant à l'outil qui le
  produit ne corrige rien.

## Conséquences

`CMAKE_CXX_STANDARD` passe à 23. Les autres décisions de
[0001](0001-cpp20-et-qt6.md) — Qt 6, cœur sans dépendance à l'interface —
restent valables.

L'exclusion des modules C++ reste en vigueur : leur outillage n'a pas progressé.

La chaîne d'outils dépend désormais de clang-tidy ≥ 19. Le `Makefile` prend la
version la plus récente disponible et retombe sur `clang-tidy` sans exiger
qu'une version précise soit installée ; la CI installe explicitement la 20.

Le déclencheur qui rouvrirait cette décision : une plateforme cible dont le
compilateur ne fournirait pas `std::expected`. MSVC le fournit depuis
Visual Studio 2022 17.3, donc le portage Windows de
[0003](0003-linux-d-abord.md) n'est pas concerné.
