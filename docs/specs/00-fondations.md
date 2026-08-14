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
sous-titres GTK/Python — en C++23 + Qt 6, avec un objectif d'iso-fonctionnalité.
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
│   ├── exigences.md            registre des exigences, cité par les tests
│   ├── mesures/                relevés de couverture et de performances
│   └── manual/                 manuels utilisateur, un dossier par exécutable
├── src/
│   ├── lib/subedit/<lib>/…     bibliothèques
│   ├── exe/<binaire>/main.cpp  exécutables
│   ├── test/{unit,e2e,bench,data}/ tests, harnais de bout en bout, benchmarks, fixtures
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

**Cinq invariants vérifiés par la CI**, tous par
[`check-architecture.sh`](../../src/scripts/check-architecture.sh) :

1. `subedit_core` ne dépend d'aucun symbole Qt ni d'aucune UI.
2. `src/exe/**/main.cpp` ne contient ni classe, ni algorithme, ni appel système
   direct.
3. les scripts de `src/scripts/` sont exécutables dans l'index git.
4. un tag de version sur `HEAD` s'accorde avec `project(VERSION)`.
5. aucun nom de cas de test ne commence par un tiret.

Les trois derniers ont été ajoutés après coup, chacun parce que son absence
avait coûté quelque chose de réel — voir « Ce qui a bougé pendant la phase ».

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

`src/test/e2e/` fait exception à cette symétrie : il miroite `src/exe/`, non
`src/lib/`. Ces tests ne lient pas la logique qu'ils vérifient — ils lancent le
binaire réel par `posix_spawn`, sans shell, et confrontent sortie standard,
sortie d'erreur et code de retour. Chaque cas cite une exigence de
[`docs/exigences.md`](../exigences.md) par un tag Catch2, ce que
`src/scripts/check-requirements.sh` confronte au registre.

Ils ne sont pas enregistrés dans CTest sous le preset `coverage` : `subedit-cli`
y est instrumenté, et chaque invocation gonflerait la couverture de `src/lib`
sans qu'aucun test unitaire ait été écrit pour ce code. Voir
[l'ADR 0014](../adr/0014-registre-d-exigences.md).

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

Norme : **C++23** depuis [`../adr/0007-cpp23-et-std-expected.md`](../adr/0007-cpp23-et-std-expected.md),
C++20 à l'origine de cette phase. Exige clang-tidy ≥ 19. **Les modules C++ sont
exclus** — le support des outils, en particulier clang-tidy et les
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

### Enregistrement des tests de bout en bout

Les tests de `src/test/e2e/` se construisent dans les quatre presets — leurs
warnings sont les nôtres partout — mais ne s'enregistrent dans CTest que sous
**`asan` et `release`**, via une option de cache déclarative,
`SUBEDIT_REGISTER_E2E` (`CMakeLists.txt`), mise à `ON` explicitement par ces
deux presets dans `CMakePresets.json` et nulle part ailleurs. Le choix d'un
indicateur déclaratif plutôt que d'un `if()` composé sur `CMAKE_BUILD_TYPE` et
`SUBEDIT_SANITIZERS` est délibéré : il se relit sans reconstituer le
raisonnement.

- **`dev` n'enregistre pas les tests de bout en bout.** `dev` et `asan` sont
  tous deux des builds Debug ; `asan` est strictement plus informatif
  (AddressSanitizer, UndefinedBehaviorSanitizer, LeakSanitizer), donc les
  enregistrer aussi sous `dev` n'ajouterait qu'une exécution de plus, sans
  nouveau signal. **Conséquence pratique : `make test` n'exerce plus le
  harnais de bout en bout ; `make asan` en est désormais le moyen le plus
  rapide en boucle de développement.**
- **`release` les enregistre** : c'est le mode qui expédie, le binaire qu'un
  utilisateur lance réellement. `make e2e` configure et compile ce preset puis
  n'exécute que ces tests, filtrés par l'étiquette CTest `e2e`
  (`catch_discover_tests(... PROPERTIES LABELS e2e)`) plutôt que par nom de
  test — un nom de test unitaire qui s'en approcherait ne le tromperait pas.
- **`coverage` ne les enregistre pas**, pour la raison déjà en place avant ce
  mécanisme : sous ce preset, `subedit-cli` est lui aussi instrumenté, et
  chaque invocation gonflerait la couverture de `src/lib` sans qu'aucun test
  unitaire ait été écrit pour ce code. Voir
  [l'ADR 0014](../adr/0014-registre-d-exigences.md).

### Façade `make`

CMake est verbeux à l'usage ; le `Makefile` n'ajoute aucune logique de
construction, seulement des raccourcis. Deux cibles gouvernent tout le
reste : **`make check`**, la porte que la CI impose à chaque push, et
**`make check-local`**, l'unique commande à lancer avant d'ouvrir une pull
request. Les deux sont décrites en détail plus bas ; le tableau donne le rôle
de chaque cible, le preset CMake qu'elle configure, ce qu'elle enchaîne, et
ses paramètres.

| Cible | Rôle | Preset | Enchaîne | Paramètres |
| :---- | :--- | :----- | :------- | :--------- |
| `make help` | affiche cette liste, générée depuis les commentaires `##` du `Makefile` | — | — | — |
| `make setup` | installe la chaîne d'outils manquante et les hooks git | — | — | — |
| `make build` | configure et compile | `dev` | — | `JOBS` |
| `make test` | compile et exécute les tests, **hors bout en bout** — voir `make asan` | `dev` | — | `JOBS` |
| `make format` | applique `clang-format` sur les fichiers suivis | — | — | — |
| `make format-check` | vérifie le format sans modifier, verdict `--Werror` | — | — | — |
| `make tidy` | exécute `clang-tidy` sur `src/**/*.cpp` | `dev` (pour `compile_commands.json`) | — | `JOBS` |
| `make arch` | vérifie les invariants d'architecture (`check-architecture.sh`) | — | — | — |
| `make parallelism` | vérifie qu'aucun parallélisme ne contourne `$(JOBS)` (`check-parallelism.sh`) | — | — | — |
| `make requirements` | confronte `docs/exigences.md` aux tags des tests de bout en bout (`check-requirements.sh`) | `dev` | — | `JOBS` |
| `make e2e` | exécute **seulement** les tests de bout en bout, filtrés par l'étiquette CTest `e2e` | `release` | — | `JOBS` |
| `make asan` | exécute les tests sous ASan/UBSan/LeakSanitizer — y compris les tests de bout en bout | `asan` | — | `JOBS` |
| `make coverage` | mesure la couverture des bibliothèques, échoue si le nombre de lignes non couvertes augmente par rapport au relevé | `coverage` | — | `JOBS` |
| `make ratchet` | enregistre la couverture mesurée comme nouveau cliquet dans le relevé | — | — | — |
| `make bench` | exécute les benchmarks, verse les chiffres au journal `docs/mesures/performances.md`, verdict lu par un humain, pas binaire | `release` | — | `JOBS` |
| `make check` | **porte de qualité — CI, FIGÉE, décrite ci-dessous** | `dev`/`asan`/`coverage` via ses sous-cibles | `format-check`, `arch`, `build`, `tidy`, `asan`, `coverage` | `JOBS` |
| `make check-local` | **unique commande avant une pull request, décrite ci-dessous** | tous les presets qu'utilisent ses sous-cibles | `parallelism`, `requirements`, `e2e`, `bench` | `JOBS` |
| `make verify-gates` | prouve que `check` et `check-local` échouent chacun sur ses défauts injectés | — | — | — |
| `make changelog` | régénère `CHANGELOG.md` depuis l'historique des commits | — | — | — |
| `make clean` | supprime `build/` | — | — | — |

`make e2e` et `make bench` configurent et compilent tous deux le preset
`release` : lancer l'un après l'autre ne recompile rien *de la bibliothèque* —
`subedit_core` est un objet partagé entre les deux. Chacun lie son propre
exécutable (`subedit_e2e_test`, `subedit_core_bench`), donc l'édition de liens
et la compilation des fichiers propres à cet exécutable restent à faire.

**Pourquoi deux gates.** `make check` est ce que la CI exécute, à l'identique,
sur chaque push de chaque personne — elle en est le seul filet, et ce qui y
entre gate tout le monde. Une vérification utile mais trop coûteuse, ou trop
propre au poste de développement, pour gater chaque push (le contrôle du
parallélisme maîtrisé, la confrontation du registre d'exigences, le harnais
de bout en bout, les benchmarks) n'a donc pas sa place dans `make check` :
elle va dans `make check-local`, une cible que la CI n'exécute jamais et qu'on
lance soi-même avant d'ouvrir une pull request. **La commande à retenir avant
d'ouvrir une pull request est `make check-local`.**

### Parallélisme maîtrisé

Chaque site de parallélisme de la façade — compilation, `clang-tidy`, LTO des
benchmarks — passe par une unique variable, `JOBS`, dont la valeur par défaut
est **deux cœurs en local**. La machine de développement fait tourner d'autres
projets en même temps, et un parallélisme non borné y provoque des échecs de
tests sur délai d'attente ; deux est le plafond convenu, relevable au cas par
cas avec `make build JOBS=8` sans toucher au `Makefile`. **La CI choisit pour
elle-même** — `.github/workflows/ci.yml` appelle `make check JOBS=$(nproc)`
explicitement, sa machine n'étant qu'à elle.

La discipline ne vaudrait rien sans un contrôle mécanique : `make parallelism`,
qui exécute `src/scripts/check-parallelism.sh`, refuse toute recette du
`Makefile` où `-j` ou `-P` n'est pas suivi de `$(JOBS)`, tout script de
`src/scripts/` qui introduirait son propre parallélisme (`-j`/`-P` codé en
dur, `xargs -P`, appel à `nproc`, commande mise en arrière-plan par un `&`
final), et tout `-flto=` codé en dur dans `cmake/*.cmake` ou `CMakeLists.txt`
en dehors du mécanisme `SUBEDIT_LTO_JOBS`. Cette vérification vit dans `make
check-local`, pas dans `make check` : elle protège le poste de développement,
pas la CI, qui borne déjà son propre parallélisme autrement.

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
5. **Couverture** — le nombre de lignes de `src/lib/` que les tests n'exercent
   pas ne doit pas augmenter. Le relevé vit dans
   [`docs/mesures/couverture.md`](../mesures/couverture.md) ; la porte échoue à
   la hausse en nommant les fichiers concernés, et invite à `make ratchet` à la
   baisse. Un compte de lignes plutôt qu'un pourcentage : un pourcentage monte
   dès qu'on ajoute du code bien testé, ce qui resserrerait le cliquet sans que
   personne l'ait décidé. Voir [l'ADR 0015](../adr/0015-memoire-des-mesures.md).
   Les exécutables restent exclus du calcul : la règle d'architecture les vide
   de tout ce qui mérite d'être couvert.

**La CI appelle `make check`, et rien d'autre.** Aucune étape n'est recopiée dans
le YAML. C'est la seule manière que le filet local et le filet distant restent
identiques dans six mois.

## La cible `make check-local` : l'unique commande avant une pull request

`make check` est ce que la CI exécute — `.github/workflows/ci.yml` n'appelle
que cette cible. Une vérification qu'on veut voir passer avant d'ouvrir une
pull request, mais qu'on ne veut pas voir gater chaque push de chaque
personne, n'a donc pas sa place dans `make check` : elle va dans
`make check-local`, une cible séparée que la CI n'exécute jamais.

`check-local` enchaîne quatre étapes, dans cet ordre précis — la moins chère
d'abord, pour qu'un défaut coûte des secondes plutôt que la totalité de la
chaîne :

1. **Parallélisme maîtrisé** — `make parallelism` : `check-parallelism.sh`
   refuse tout site de parallélisme, dans le `Makefile`, dans `src/scripts/`
   ou dans `cmake/`, qui contourne `$(JOBS)` ou `SUBEDIT_LTO_JOBS`. Un grep,
   sous la seconde — d'où sa place en tête : la faire attendre derrière une
   étape qui compile coûterait à un `-j 8` codé en dur le temps de ce build
   avant qu'on l'entende. Voir
   [« Parallélisme maîtrisé »](#parallélisme-maîtrisé) plus haut.
2. **Exigences** — `make requirements` : `check-requirements.sh` confronte le
   registre [`docs/exigences.md`](../exigences.md) aux tags des tests de bout
   en bout, dans les deux sens : une exigence `implémentée` que rien ne cite
   échoue, un tag en forme d'identifiant qui ne désigne aucune exigence
   échoue. La couverture de lignes dit quel code a été exécuté ; celle-ci dit
   quelle promesse est démontrée. Voir
   [l'ADR 0014](../adr/0014-registre-d-exigences.md), dont les Conséquences
   discutent le prix de la garder hors CI.
3. **Tests de bout en bout** — `make e2e` : configure et compile le preset
   `release`, puis exécute uniquement le harnais de bout en bout (étiquette
   CTest `e2e`, filtrée avec `ctest -L` plutôt que par nom de test). Voir
   [« Enregistrement des tests de bout en bout »](#enregistrement-des-tests-de-bout-en-bout)
   plus haut pour le mécanisme d'enregistrement.
4. **Benchmarks** — `make bench` : réutilise l'objet `subedit_core` déjà
   compilé par l'étape précédente, mais lie son propre exécutable
   (`subedit_core_bench`) — ce n'est donc pas un no-op, seulement moins cher
   qu'un build `release` complet. Son verdict se lit, il n'est pas binaire —
   c'est voulu : la [définition de « terminé »](../../CLAUDE.md) du projet
   impose de rejouer les benchmarks à chaque issue, et les chaîner ici est ce
   qui le garantit plutôt que de compter sur la mémoire de qui ouvre la pull
   request.

Le critère d'entrée dans `check-local` est le même pour toute future
addition : utile avant une pull request, mais pas assez universelle — ou trop
coûteuse — pour gater chaque push sur la CI.

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
| `docs/exigences.md` | ce que le binaire promet, et l'état de ce qui le prouve |
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
  effectivement cassé la CI. Deux autres l'ont suivi pour la même raison : un
  tag de version qui ne s'accorde pas avec `project(VERSION)` ferait annoncer
  au binaire une version périmée, et **un nom de cas de test commençant par un
  tiret est lu par Catch2 comme une de ses options** quand CTest le lui passe
  en argument. Ce dernier ne se voit qu'à travers CTest — donc dans la porte et
  dans la CI, jamais en lançant le binaire de test à la main — et il a été payé
  deux fois pendant l'écriture de la CLI avant d'être inscrit.

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
