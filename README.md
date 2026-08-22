# subedit

Éditeur de sous-titres, réécriture en C++23 et Qt 6 de
[Gaupol](https://github.com/otsaloma/gaupol).

> **État : fondations.** Le projet n'édite encore aucun sous-titre. Seule
> l'infrastructure de développement est en place. La
> [feuille de route](docs/feuille-de-route.md) décrit les seize phases prévues :
> les huit premières mènent à un MVP, les suivantes complètent
> l'iso-fonctionnalité avec Gaupol.

## Pourquoi

Gaupol est un excellent éditeur de sous-titres, écrit en Python avec GTK.
subedit vise l'**iso-fonctionnalité** dans un langage natif compilé, avec la
performance comme objectif de conception transversal — structures de données et
algorithmes adaptés dès la conception, plutôt qu'optimisation après coup.

## Construire

Dépendances : CMake ≥ 3.28, un compilateur C++23 (GCC 13 convient), et la chaîne
d'outils de vérification.

```bash
./src/scripts/setup-toolchain.sh   # ninja, clang-tidy, clang-format, gcovr, ccache, git-cliff, Qt 6, ffmpeg
make build                         # compile
make test                          # compile et exécute les tests (hors bout en bout — voir make asan)
make check                         # la porte de qualité complète
make help                          # toutes les cibles
```

## Organisation

```
src/lib/subedit/<lib>/    bibliothèques — tout le code utile vit ici
src/exe/<binaire>/        exécutables — un main et du câblage, rien d'autre
src/test/{unit,bench}/    tests et benchmarks, en miroir des bibliothèques
src/scripts/              automatisation
docs/                     specs, décisions, manuels
reference/gaupol          clone de référence, en lecture seule, non commité
```

Les inclusions internes sont toujours qualifiées depuis `src/lib` :

```cpp
#include <subedit/core/version.hpp>
```

## Qualité

`make check` est la porte, et la CI n'exécute rien d'autre : format, warnings en
erreurs, clang-tidy, invariants d'architecture, tests sous AddressSanitizer et
UndefinedBehaviorSanitizer, cliquet de couverture. Local et distant ne peuvent
pas diverger.

## Documentation

| Document | Contenu |
| :------- | :------ |
| [Feuille de route](docs/feuille-de-route.md) | le contour du MVP, les phases, leur cadrage et leur ordre |
| [Principes de conception](docs/principes-de-conception.md) | règles permanentes applicables à tout le code |
| [Specs](docs/specs/) | une spec par phase, et l'inventaire des fonctionnalités de Gaupol |
| [Décisions](docs/adr/) | ce qui a été décidé, et pourquoi les autres options ont été écartées |
| [Manuels](docs/manual/) | un manuel par exécutable |

## Licence

GPL-3.0-or-later, voir [LICENSE](LICENSE).

Ce choix permet de réutiliser les données linguistiques de Gaupol — motifs de
correction, en-têtes de formats, traductions — qui représentent des années de
travail. Voir [ADR 0002](docs/adr/0002-licence-gpl3.md).

subedit est une réécriture indépendante : aucun code de Gaupol n'est repris.
