# 0004 — Résoudre les dépendances par les paquets système

**Date :** 2026-08-04
**Statut :** acceptée

## Contexte

Le projet dépendra de Qt 6, d'un moteur d'expressions régulières, d'ICU et d'un
correcteur orthographique. Deux gestionnaires de paquets C++ pouvaient être
adoptés d'emblée : Conan 2 et vcpkg.

## Décision

Les dépendances sont résolues par `find_package` sur les paquets de la
distribution. Aucun gestionnaire de paquets C++ n'est mis en place.

Catch2 fait exception, récupéré par `FetchContent` : il se compile en quelques
secondes et sa version doit être identique partout, faute de quoi une mise à
jour de distribution pourrait casser les tests.

## Alternatives écartées

- **vcpkg dès maintenant** — garantirait des versions identiques sur toutes les
  plateformes, mais compile Qt depuis les sources : des heures au premier build
  et à chaque CI dont le cache est froid.
- **Conan 2 dès maintenant** — meilleur que vcpkg sur le versionnement, les
  profils de compilation et les paquets binaires. Écarté parce qu'il ne tient
  pas sa promesse là où elle compterait : la recette Qt de ConanCenter est
  lourde et se recompile souvent depuis les sources. On paierait un second
  système de construction pour économiser l'installation de quelques paquets.

## Conséquences

Le premier build est rapide et ne dépend que de `apt`. En contrepartie, les
versions des dépendances sont celles de la distribution, donc variables d'une
machine à l'autre — acceptable tant que la cible est une seule plateforme.

**Ce qui rend la décision peu coûteuse à défaire :** Conan et vcpkg fonctionnent
tous deux en s'interposant sur `find_package`. Tant que le CMake du projet est
écrit autour de ce mécanisme, et non autour de chemins codés en dur, les adopter
plus tard demandera un fichier de manifeste, pas une réécriture.

Le déclencheur : un portage Windows effectif — voir
[0003](0003-linux-d-abord.md).
