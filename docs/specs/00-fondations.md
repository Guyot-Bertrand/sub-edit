# Sous-projet 0 — Fondations

**État :** réalisée le 2026-08-05
**Milestone :** `0 — Fondations`

## Objectif

Poser la structure, la chaîne de construction et le filet de vérification du
projet, avant toute ligne de code métier. La question à laquelle ce sous-projet
répond : **comment juger qu'un code produit est bon, au-delà du fait qu'il
compile ?**

La réponse tient en une commande, `make check`, exécutée à l'identique en local
et en intégration continue.

## Contexte

`subedit` réécrit [Gaupol](https://github.com/otsaloma/gaupol) — éditeur de
sous-titres GTK/Python — en C++20 + Qt 6, avec un objectif d'iso-fonctionnalité.
L'inventaire des fonctionnalités à couvrir est dans
[`gaupol-reference.md`](gaupol-reference.md). Le travail est découpé en
phases ; celle-ci est la première.

Les règles de conception applicables à tout le code — modèles de données typés,
propriété mémoire explicite, abstraction là où la variation est connue — sont
dans [`../principes-de-conception.md`](../principes-de-conception.md). Ce
sous-projet en mécanise la part vérifiable : `make check` doit refuser ce que
ces règles interdisent, plutôt que compter sur la vigilance à la relecture.

La performance est un **objectif de conception transversal** — choix de
structures de données, d'algorithmes et d'outils qui ne gaspillent pas — et non
la correction d'une lenteur mesurée. Cela justifie que les benchmarks soient
outillés dès les fondations plutôt qu'ajoutés après coup.

## Portée

**Inclus :** organisation du dépôt, conventions de code, CMake et presets,
façade `make`, framework de test et de benchmark, analyse statique, sanitizers,
couverture, CI GitHub Actions, hooks git, templates d'issues et de PR,
génération du CHANGELOG, squelette de documentation, ADR rétrodocumentées.

**Exclu volontairement :** Conan et vcpkg actifs, génération Doxygen,
empaquetage, publication automatisée, matrice multi-compilateurs, multi-OS.
Chacun a un déclencheur identifié plus loin ; aucun n'est déclenché aujourd'hui.

## Organisation du dépôt

```
subedit/
├── CMakeLists.txt
├── CMakePresets.json
├── Makefile                    façade ergonomique sur CMake
├── LICENSE                     GPL-3.0-or-later
├── README.md
├── CONTRIBUTING.md
├── CHANGELOG.md                généré par git-cliff
├── cliff.toml
├── .clang-format
├── .clang-tidy
├── .editorconfig
├── .gitignore
├── .github/
│   ├── CODEOWNERS
│   ├── workflows/ci.yml
│   ├── ISSUE_TEMPLATE/{bug,story,enhancement,task}.yml
│   └── pull_request_template.md
├── cmake/                      modules : warnings, sanitizers, coverage, helpers
├── docs/
│   ├── configuration-github.md
│   ├── specs/                  une spec par sous-projet
│   ├── adr/                    décisions techniques
│   └── manual/                 manuels utilisateur, un dossier par exécutable
├── src/
│   ├── lib/subedit/<lib>/…     bibliothèques
│   ├── exe/<binaire>/main.cpp  exécutables
│   ├── test/{unit,bench,data}/ tests, benchmarks, fixtures
│   └── scripts/                automatisation
└── reference/gaupol            clone de référence, ignoré par git
```

### Règle d'architecture : tout le code utile est en bibliothèque

Un exécutable se réduit à un `main`, l'analyse de ses arguments si nécessaire, et
l'appel des bibliothèques. Aucune logique fonctionnelle dans `src/exe` — pas même
dans l'interface graphique : la fenêtre principale, le modèle de table, les
dialogues et les actions vivent dans `subedit_gui`, et `src/exe/gui/main.cpp` se
limite à instancier `QApplication`, construire la fenêtre et lancer la boucle
d'événements.

L'intérêt est direct : ce qui est en bibliothèque est testable, y compris le
code Qt, pilotable par Qt Test.

### Bibliothèques

```
src/lib/subedit/core/    subedit_core   modèle, formats, opérations — aucune dépendance Qt
src/lib/subedit/cli/     subedit_cli    analyse d'arguments et commandes, au-dessus de core
src/lib/subedit/gui/     subedit_gui    interface Qt, au-dessus de core
```

Chaque enfant direct de `src/lib/subedit/` est une cible CMake. Si `core`
grossit trop, il se scinde sans impact sur le reste.

**Deux invariants vérifiés par la CI :**

1. `subedit_core` ne dépend d'aucun symbole Qt ni d'aucune UI.
2. `src/exe/**/main.cpp` ne contient ni classe, ni algorithme, ni appel système
   direct.

### Inclusions

Racine d'inclusion unique : `src/lib/`. Toute inclusion interne au projet est
qualifiée et absolue.

```cpp
#include <subedit/core/model/subtitle.hpp>
```

Aucun chemin relatif (`../`), aucune ambiguïté, et les cibles restent
déplaçables. Ce qui est interne à une bibliothèque va dans un sous-dossier
`detail/` ; le reste constitue l'API publique. Headers et sources sont côte à
côte — la séparation `include/` + `src/` dupliquerait l'arborescence sans rien
apporter à un dépôt applicatif.

### Tests

`src/test/` reflète l'arborescence des bibliothèques :

```
src/test/unit/core/model/subtitle_test.cpp
src/test/bench/core/format/srt_bench.cpp
src/test/data/                              fichiers de sous-titres de référence
```

Un binaire de test par bibliothèque, enregistré dans CTest via
`catch_discover_tests` pour que chaque cas soit visible et filtrable
individuellement.

## Conventions de code

| Élément | Convention |
| :------ | :--------- |
| Fichiers | `snake_case.hpp` / `snake_case.cpp` |
| Espaces de noms | `subedit::core`, minuscules |
| Types | `PascalCase` |
| Fonctions et variables | `camelCase` |
| Membres privés | préfixe `m_` |
| Constantes de compilation | `kPascalCase` |

Le mélange de conventions entre le cœur et le code Qt serait pire que n'importe
lequel des deux styles ; `camelCase` s'aligne donc sur l'API Qt pour l'ensemble
du projet. Ces règles sont **appliquées mécaniquement** par
`readability-identifier-naming` de clang-tidy, pas laissées à la vigilance.

Format : `.clang-format` dérivé du style LLVM, indentation de 4 espaces, limite
de colonne à 100, pointeurs alignés à gauche.

Norme : C++20. Concepts et ranges sont disponibles avec GCC 13. **Les modules
C++20 sont exclus** — le support des outils, en particulier clang-tidy et les
générateurs, n'est pas mûr.

## Construction

CMake ≥ 3.28 (version présente sur la machine). Ninja est le générateur préféré,
mais **les presets ne le figent pas** : la façade `make` le sélectionne s'il est
installé et retombe sur les Makefiles Unix sinon. Le figer rendrait le projet
inconstructible sur une machine qui ne l'a pas encore, pour un gain nul.

### Presets

| Preset | Contenu |
| :----- | :------ |
| `dev` | Debug, warnings en erreurs, tests activés — le preset par défaut |
| `asan` | Debug + AddressSanitizer + UndefinedBehaviorSanitizer |
| `coverage` | Debug + instrumentation de couverture |
| `release` | RelWithDebInfo + LTO |

`ccache` est utilisé s'il est présent, ignoré sinon.

### Façade `make`

CMake est verbeux à l'usage ; le `Makefile` n'ajoute aucune logique de
construction, seulement des raccourcis.

| Cible | Effet |
| :---- | :---- |
| `make setup` | installe la chaîne d'outils manquante et les hooks git |
| `make build` | configure et compile le preset `dev` |
| `make test` | compile et exécute les tests |
| `make bench` | exécute les benchmarks en `release` |
| `make format` | applique `clang-format` |
| `make tidy` | exécute `clang-tidy` |
| `make coverage` | produit le rapport de couverture |
| `make changelog` | régénère `CHANGELOG.md` |
| `make check` | **la porte de qualité, décrite ci-dessous** |
| `make clean` | supprime le répertoire de build |

## Dépendances

Résolues par `find_package` sur les paquets système, à une exception près.

| Dépendance | Origine | Requise pour |
| :--------- | :------ | :----------- |
| Catch2 v3 | `FetchContent` | tests et micro-benchmarks |
| Qt 6 | paquets système | sous-projet 5 |
| PCRE2 ou RE2 | paquets système | phase 12 — choix à trancher |
| ICU | paquets système | encodages, phase 8 |
| hunspell | paquets système | phase 12 |

Catch2 fait exception parce qu'il se compile en quelques secondes et que sa
version doit être identique partout ; l'épingler évite qu'une mise à jour de
distribution casse les tests.

Aucun gestionnaire de paquets C++ n'est mis en place. Le CMake étant écrit
autour de `find_package`, adopter Conan ou vcpkg plus tard est presque gratuit —
tous deux fonctionnent en s'interposant précisément sur ce mécanisme. Le
déclencheur serait le portage Windows effectif.

`src/scripts/setup-toolchain.sh` installe ce qui manque sur une machine Ubuntu : ninja, clang-tidy, clang-format, gcovr, ccache, git-cliff,
gh, et les paquets de développement Qt6 le moment venu.

## La porte de qualité : `make check`

Cinq étapes, verdict binaire, aucune tolérance :

1. **Format** — `clang-format --dry-run --Werror` sur tous les fichiers suivis.
2. **Compilation** — preset `dev` avec `-Wall -Wextra -Wpedantic -Wconversion
   -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
   -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Werror`.
3. **Analyse statique** — `clang-tidy` : familles `bugprone-*`, `performance-*`,
   `modernize-*`, `readability-*`, `misc-*`, plus une sélection de
   `cppcoreguidelines-*`. Y compris, explicitement, les vérifications qui
   mécanisent les règles de propriété mémoire des principes de conception :
   `cppcoreguidelines-owning-memory`, `cppcoreguidelines-no-malloc`,
   `cppcoreguidelines-special-member-functions`, `modernize-make-unique`,
   `modernize-avoid-c-arrays`, `misc-const-correctness`. Les exclusions sont
   listées dans `.clang-tidy` **avec leur justification en commentaire** ; une
   exclusion non justifiée est un défaut.
4. **Tests** — exécution sous le preset `asan`, pour que toute erreur mémoire ou
   comportement indéfini échoue au lieu de passer inaperçu. Le preset active
   aussi **LeakSanitizer** : une fuite fait échouer les tests, elle ne se
   découvre pas six mois plus tard.
5. **Couverture** — seuil de **80 % minimum sur les bibliothèques**, relevable
   par bibliothèque quand c'est justifié. Les exécutables sont exclus du calcul :
   la règle d'architecture les vide de tout ce qui mérite d'être couvert.

**La CI appelle `make check`, et rien d'autre.** Aucune étape n'est recopiée dans
le YAML. C'est la seule manière que le filet local et le filet distant restent
identiques dans six mois.

## Intégration continue

Un workflow, `.github/workflows/ci.yml`, sur `push` et `pull_request` :

- **`check`** — runner `ubuntu-24.04`, installation de la chaîne d'outils par le
  même script qu'en local, cache `ccache`, exécution de `make check`, dépôt du
  rapport de couverture en artefact.
- **`commits`** — validation des messages de commit et du titre de PR contre la
  grammaire Conventional Commits.

Pas de service de couverture externe, pas de matrice d'OS ni de compilateurs :
la cible est Linux, et le portage Windows viendra avec sa propre issue.

## Git et GitHub

### Conventional Commits

```
<type>(<scope>): <description>
```

**Types :** `feat`, `fix`, `perf`, `refactor`, `test`, `docs`, `build`, `ci`,
`chore`.

**Scopes :** identiques aux labels `area:` — `build`, `core`, `format`, `text`,
`cli`, `gui`, `video`, `i18n`, `ci`, `doc`. Une issue et les commits qui la
traitent partagent ainsi le même vocabulaire.

Validation par un hook `commit-msg` local **et** en CI. Les hooks sont des
scripts shell installés dans `.git/hooks` par `src/scripts/install-hooks.sh` —
pas de dépendance à un gestionnaire de hooks tiers.

- `pre-commit` : `clang-format` sur les fichiers indexés, refus si `reference/`
  est indexé par mégarde.
- `commit-msg` : validation de la grammaire.

### Branches

`main` protégée par un ruleset, travail sur branches `feat/…`, `fix/…`. Push
direct sur `main` autorisé pour le propriétaire — pas de PR obligatoire à ce
stade, l'exigence pourra être relevée plus tard sans rien changer d'autre.

### Issues

Templates de formulaire pour bug, story, amélioration et tâche. Labels et
milestones sont décrits dans [`../configuration-github.md`](../configuration-github.md),
avec la configuration de verrouillage du dépôt.

## CHANGELOG

`CHANGELOG.md` à la racine, au format *Keep a Changelog*, **généré par
`git-cliff` depuis l'historique des commits**. C'est le retour sur
investissement direct des Conventional Commits : la classification est déjà dans
les messages, la génération est déterministe, et aucune entrée ne peut être
oubliée.

`make changelog` régénère le fichier. **Cette régénération fait partie de la
définition de « terminé » d'une issue**, au même titre que les tests.

## Documentation

| Emplacement | Contenu |
| :---------- | :------ |
| `docs/principes-de-conception.md` | règles permanentes de conception et de gestion mémoire |
| `docs/specs/NN-<sujet>.md` | une spec par sous-projet, conception durable |
| `docs/adr/NNNN-<titre>.md` | décisions techniques et alternatives écartées |
| `docs/manual/<exécutable>/` | manuel utilisateur, un fichier par section |
| `README.md` | ce qu'est le projet, pourquoi, comment le construire |
| `CONTRIBUTING.md` | flux de travail, conventions, commandes |

Une spec décrit une conception qui doit rester lisible après coup ; une issue
décrit un travail qui meurt quand il est fait. Les deux ne se recouvrent pas, et
les issues renvoient vers la spec de leur sous-projet.

Les manuels sont structurés en un dossier par exécutable, un fichier par
section, un `index.md` en sommaire. **Rédiger la section de manuel concernée fait
partie du travail d'une issue**, pas d'une passe de rattrapage finale : formuler
le comportement attendu en français est un test de conception.

Le code porte des commentaires au format Doxygen sur l'API publique. Aucune
génération n'est mise en place ; le déclencheur serait une API stabilisée.

### ADR à rétrodocumenter

Décisions déjà prises, à écrire pendant ce sous-projet :

| N° | Décision |
| :- | :------- |
| 0001 | C++20 + Qt 6 plutôt que Rust + GTK4 ou Rust + toolkit portable |
| 0002 | GPL-3.0-or-later, pour réutiliser les motifs de correction de Gaupol |
| 0003 | Linux d'abord, en évitant tout choix rendant le portage Windows coûteux |
| 0004 | Paquets système et `find_package` plutôt que Conan ou vcpkg |
| 0005 | Catch2 v3 pour les tests et les micro-benchmarks |

Format : contexte, décision, conséquences, alternatives écartées et pourquoi.
Une ADR n'est jamais modifiée — elle est remplacée par une ADR ultérieure.

## Critères d'acceptation

Le troisième critère a donné lieu à un script rejouable,
`src/scripts/verify-gates.sh`, exposé par `make verify-gates` : il injecte un
défaut de chaque type, vérifie que l'étape correspondante échoue, puis rétablit
les sources. À relancer après toute modification de `.clang-format`,
`.clang-tidy`, des options de compilation ou du seuil de couverture — une porte
qu'on n'a jamais vue se refermer n'est qu'une croyance.

Le sous-projet est terminé quand :

1. `make setup` installe la chaîne d'outils sur une machine Ubuntu 24.04 nue.
2. Le dépôt contient une bibliothèque `subedit_core` réduite à un module
   trivial, son test et son benchmark, et un exécutable `subedit-cli` qui
   l'appelle — de quoi prouver que toute la chaîne fonctionne de bout en bout.
3. `make check` passe et **échoue effectivement** quand on introduit
   délibérément : un défaut de format, un warning, un motif rejeté par
   clang-tidy, une erreur mémoire, une baisse de couverture sous le seuil. La
   vérification que les portes se ferment fait partie du travail.
4. La CI est verte sur GitHub et exécute `make check`.
5. Les hooks git rejettent un message de commit non conforme.
6. `make changelog` produit un `CHANGELOG.md` cohérent.
7. Les cinq ADR sont écrites, ainsi que `README.md` et `CONTRIBUTING.md`.
8. Le dépôt GitHub public est créé et configuré selon
   [`../configuration-github.md`](../configuration-github.md).

### Réalisation

| Critère | État | Preuve |
| :------ | :--- | :----- |
| 1 — chaîne d'outils | ✅ | `setup-toolchain.sh` exécuté sur la machine cible |
| 2 — chaîne de bout en bout | ✅ | `subedit_core`, son test, son benchmark, `subedit-cli` |
| 3 — les portes se referment | ✅ | `make verify-gates`, cinq défauts injectés, cinq échecs |
| 4 — CI verte exécutant `make check` | ✅ | workflow `ci`, jobs `porte de qualité` et `messages de commit` |
| 5 — hooks git | ✅ | message non conforme rejeté, indexation de `reference/` refusée |
| 6 — CHANGELOG | ✅ | `make changelog`, git-cliff 2.10.1 |
| 7 — ADR et documentation | ✅ | ADR 0001 à 0005, `README.md`, `CONTRIBUTING.md` |
| 8 — dépôt configuré | ⚠️ | labels, milestones et rulesets faits ; deux réglages restants |

Les deux réglages restants n'ont pas d'API et se font dans l'interface :
approbation des workflows de fork, et limites d'interaction. Ils protègent
d'interactions extérieures qui ne se produisent pas encore sur un dépôt sans
visibilité ; leur absence ne bloque aucun travail.

**Trois écarts assumés** par rapport à la conception initiale, chacun justifié
sur place dans ce document :

- les presets **ne figent pas Ninja** ;
- le critère 3 a donné lieu à un **script rejouable** plutôt qu'à une
  vérification unique ;
- un **troisième invariant d'architecture** a été ajouté après coup — les
  scripts doivent être exécutables dans l'index git — parce que son absence a
  effectivement cassé la CI.

## Points ouverts

À trancher dans les sous-projets suivants, mentionnés ici parce qu'ils
influenceront le code dès les premières lignes :

- **Stratégie de gestion d'erreurs** — exceptions, codes de retour ou type
  résultat. `std::expected` relève de C++23 : indisponible en C++20, mais
  fonctionnel avec le GCC 13 déjà installé en `-std=c++23`. Retenir le type
  résultat reviendrait donc à passer le projet en C++23, sans changer de
  compilateur. À trancher dans la phase 1, par ADR — voir
  [`../adr/README.md`](../adr/README.md) pour les réserves mesurées.
- **Moteur d'expressions régulières** — PCRE2, compatible avec la syntaxe Python
  des motifs de Gaupol, ou RE2, nettement plus rapide mais sans références
  arrière. À trancher dans la phase 12, après mesure.
