# Instructions du projet subedit

## RÈGLE CRITIQUE — le dépôt de référence est en lecture seule

`reference/gaupol` est un clone de [Gaupol](https://github.com/otsaloma/gaupol),
présent uniquement pour être **lu**. Il n'est pas suivi par git et ne fait pas
partie du projet.

**Aucune modification, jamais.** Sont interdits, sans exception ni exception
apparente :

- écrire, éditer, créer ou supprimer un fichier sous `reference/` ;
- toute commande git qui écrit dans ce dépôt — `add`, `commit`, `checkout`,
  `reset`, `clean`, `stash`, `rm`, `restore`, `branch`, `merge`, `rebase` ;
- y lancer un build, un formateur ou un outil qui produit des artefacts.

**Seules opérations autorisées :** la lecture des fichiers, et
`git -C reference/gaupol fetch` puis `git -C reference/gaupol log`/`diff` pour
consulter les évolutions amont. Même la mise à jour de la copie de travail
(`pull`, `merge`) se discute avant d'être faite.

**La lecture est pour l'humain, jamais pour le code.** Rien sous `src/lib`,
`src/exe` ou `src/test` ne désigne `reference/` : le clone est absent de la CI,
absent d'une machine fraîchement clonée, absent d'une archive du dépôt. Un test
qui le lit passe chez qui l'a et échoue chez tous les autres — ou se déclare
ignoré, ce qui est pire, puisqu'il reste vert sans rien prouver. La donnée de
test versionnée vit dans `src/test/data/` ; `src/data/` n'est pas une option,
c'est le corpus privé de chaque machine et il est ignoré par git.
`check-architecture.sh` le vérifie.

**Le contenu de `src/data/` ne se cite jamais dans un fichier versionné.** Ni le
code, ni la documentation, ni une spec, ni une ADR, ni un message de commit, ni
le corps d'une issue ou d'une pull request ne nomme un fichier du corpus privé —
pas plus son titre que son nom de fichier, pas plus dans un tableau de mesures
que dans une phrase. Cela vaut pour `src/data/exemples/` comme pour le reste.

La raison est double. Ce sont **des fichiers qui n'appartiennent pas au projet**
et dont le dépôt n'a pas à publier l'inventaire. Et un chiffre attribué à un fichier que
personne d'autre ne possède est **une preuve que personne ne peut rejouer** :
le lecteur croit lire une mesure là où il n'a qu'un témoignage.

Ce qui reste permis, et qui suffit : parler du corpus **globalement** — « huit
des quinze fichiers », « les jeux d'exemples 004 à 006 », « le corpus privé ne
tranche pas ». Une observation ainsi formulée s'assume comme telle. Et quand un
chiffre doit devenir une garantie plutôt qu'une observation, il lui faut une
fixture dans `src/test/data/`, versionnée, que tout le monde peut lire.

Défaut déjà commis, en août 2026, dans la feuille de route et une spec de phase :
cinq titres de films cités dans un tableau de mesures. Corrigé après coup.

**Piège pratique, déjà rencontré :** le répertoire de travail du shell persiste
d'un appel à l'autre. Un `cd reference/gaupol` fait plus tôt dans la session
rend toute commande ultérieure dangereuse — `git add -A` cible alors le clone,
pas le projet.

> **Conséquence : toute commande git de ce projet s'écrit avec un chemin
> explicite,** `git -C /home/beber/Projects/subedit …`, ou est précédée d'une
> vérification de `pwd`. Ne jamais s'en remettre au répertoire courant supposé.

### Verrou mécanique

L'arborescence du clone est maintenue **non inscriptible**, ce qui fait échouer
toute écriture au niveau du système de fichiers plutôt qu'au niveau de la
vigilance. `src/scripts/reference.sh` pilote ce verrou :

| Commande | Effet |
| :------- | :---- |
| `./src/scripts/reference.sh status` | état du verrou, révision, écarts éventuels |
| `./src/scripts/reference.sh fetch` | récupère l'amont, **rétablit le verrou automatiquement** |
| `./src/scripts/reference.sh restore` | rétablit le clone dans l'état de son HEAD |
| `./src/scripts/reference.sh lock` / `unlock` | verrouille, déverrouille |

`unlock` ne s'utilise que délibérément et se referme aussitôt après. C'est un
garde-fou contre l'erreur, pas une barrière de sécurité : le propriétaire peut
défaire un `chmod`. Le but est précisément qu'une modification accidentelle soit
impossible sans qu'un geste explicite l'ait précédée.

Si le clone se retrouve modifié, le signaler et proposer la restauration — ne
pas la faire silencieusement.

## Le projet

`subedit` réécrit Gaupol — éditeur de sous-titres GTK/Python — en C++23 + Qt 6,
avec un objectif d'iso-fonctionnalité.

| Document | Contenu |
| :------- | :------ |
| [`docs/feuille-de-route.md`](docs/feuille-de-route.md) | le contour du MVP, les phases, leur cadrage et leur ordre |
| [`docs/principes-de-conception.md`](docs/principes-de-conception.md) | règles permanentes applicables à tout le code |
| [`docs/specs/`](docs/specs/) | une spec par phase, plus l'inventaire de Gaupol |
| [`docs/configuration-github.md`](docs/configuration-github.md) | verrouillage du dépôt, labels, milestones |

## Conventions

- **Langue** — deux frontières, et la première est celle du fichier : **le C++
  s'écrit en anglais de bout en bout, identifiants comme commentaires ; tout le
  reste du dépôt s'écrit en français.** La seconde ne vaut qu'à l'intérieur de
  la première : **ce que le binaire imprime est en anglais, ce qui explique
  pourquoi est en français.**

  | Anglais | Français |
  | :------ | :------- |
  | le C++, identifiants **et commentaires** | la documentation, les specs, les ADR, les manuels |
  | tout ce que le binaire écrit à un utilisateur | les scripts et le système de construction |
  | les intitulés de cas de test, que Catch2 imprime | les messages de commit et les échanges |

  **Tranché en #312, sur un compte** — et dans l'autre sens que #273, qui avait
  écrit l'inverse. Chaque famille de fichiers a sa langue, sans mélange :

  | | anglais | français |
  | :--- | ------: | -------: |
  | `src/**.cpp`, `src/**.hpp` | **6 460** | 1 161 |
  | `src/scripts` | 6 | **1 971** |
  | `cmake`, `Makefile` | 4 | **386** |

  **Une seule des deux moitiés a dérivé, et la dérive a une date.** Part de
  français dans les commentaires C++, par semaine d'écriture :

  | S32 | S33 | S34 | S35 | S36 |
  | --: | --: | --: | --: | --: |
  | 0,0 % | 0,3 % | 2,7 % | **30,2 %** | **32,8 %** |

  Trois semaines d'anglais, puis les phases d'interface. C'est ce que #273 a vu
  à la fin de la quatrième semaine — et il en a conclu que le dépôt avait
  *toujours* été français, en cherchant les fichiers portant **au moins un**
  caractère accentué, ce qui ne pouvait rendre que cela. Un fichier de cinquante
  commentaires anglais et d'un français y comptait pour français. C'est le
  défaut de #268 — vérifier avec l'outil qui ne compte pas — commis une issue
  après avoir été inscrit.

  **Un cliquet tient la règle**, parce que les deux versions précédentes ont
  cessé de décrire le dépôt sans que personne le voie :
  `check-comment-language.py` compte les lignes françaises restées dans le C++
  et **refuse qu'elles montent**. Il est appelé par `check-architecture.sh`,
  donc par `make check`. Le baisser est le geste normal d'une pull request qui
  traduit ; il n'y a pas de bouton pour le remonter.

  **Mille cent soixante et une lignes restent à traduire**, concentrées dans une
  cinquantaine de fichiers d'interface — issue #325. En attendant, la règle
  vaut pour ce qu'on écrit : **du C++ neuf se commente en anglais**, et un
  fichier qu'on modifie ne se met pas à mélanger les deux.

  Les intitulés de tests, eux, avaient bien dérivé : **cent en français**,
  presque tous nés d'une seule phase, et un `--list-tests` qui rendait deux
  langues mêlées. Ils ont été traduits, et `check-architecture.sh` tient la
  règle — un caractère hors ASCII ou un mot
  outil français dans un intitulé fait échouer la porte, sauf exemption inscrite
  et motivée.

  **Anglais aussi pour tout ce que le binaire écrit à un utilisateur** — sortie
  standard, sortie d'erreur, en-têtes de colonne, libellés de menu, messages de
  dialogue. Les deux surfaces disent les mêmes mots, et
  [`core/wording.hpp`](src/lib/subedit/core/wording.hpp) est l'unique endroit
  où ils sont écrits. La traduction est une phase à elle seule ; d'ici là, une
  seule langue.

  Cette ligne manquait, et le silence a coûté : la fenêtre des phases 5 est née
  en français — « N° », « Début », « Annuler : décalage » — pendant que la
  ligne de commande écrivait en anglais. Repris en août 2026, une fois la
  troisième issue d'interface fusionnée.
- **Commits** — Conventional Commits, scopes alignés sur les labels `area:`.
- **Titre d'une pull request** — **la même grammaire, la même limite de 72
  caractères.** `.github/workflows/pull-request.yml` passe le titre à
  `check-commit-message.sh`, exactement comme le hook `commit-msg` lui passe un
  message. « Relecture de fin de phase 5 » n'est donc pas un titre valide ;
  « docs(doc): relecture de fin de phase 5 » l'est.

  Se vérifie avant d'ouvrir, en une seconde :

  ```console
  $ ./src/scripts/check-commit-message.sh "docs(doc): relecture de fin de phase 5"
  ```

  Cette ligne manquait, et le silence a coûté : trois pull requests d'affilée
  ouvertes avec un titre non conforme en août 2026, trois fois la même étape
  rouge, et la règle découverte en lisant le YAML plutôt qu'ici. C'est le défaut
  de la langue de l'interface, à l'identique — une contrainte réelle, tenue par
  un contrôle, et absente du seul document qu'on lit avant d'agir.
- **Fermeture d'une issue** — le corps de la PR porte une ligne `Closes #N`,
  **en anglais**. Ce n'est pas de la prose mais une instruction à GitHub, qui ne
  reconnaît que ses propres mots-clés : « Ferme #26 » n'en est pas un, et
  l'issue reste ouverte après la fusion. La règle de langue ne s'y applique pas,
  comme elle ne s'applique pas au `Co-Authored-By` d'un commit. Écrire la phrase
  française **en plus** si elle apporte quelque chose ; la ligne `Closes #N`
  n'est pas négociable.
- **Versions** — le `project(VERSION)` de `CMakeLists.txt` est la source du
  numéro courant ; il bouge à deux occasions :
  - **patch, à chaque PR** — toute PR incrémente le patch dans son propre
    diff (`0.1.0` → `0.1.1` → `0.1.2`…). Pas de tag pour autant.
  - **mineur, à chaque milestone terminée** — on remet le patch à zéro, on
    bumpe le mineur, et on pose un tag `vX.Y.0` : `v0.1.0` clôt la phase 1,
    `v0.2.0` clôra la phase 2, et ainsi de suite jusqu'à une `v1.0.0` à la fin.

  **Le bump se fait au dernier moment** — jamais en début de travail. Le numéro
  n'est connu qu'à ce moment-là : entre le premier commit et la PR, d'autres PR
  ont pu être fusionnées et avoir pris le numéro visé. Bumper tôt, c'est se
  garantir un conflit sur `CMakeLists.txt` et un rebase de plus.

  « Au dernier moment » veut dire **juste avant la dernière porte, pas après**.
  Le bump touche `CMakeLists.txt`, que la porte lit — le passer après elle
  obligerait à la relancer, dix minutes pour un chiffre. L'ordre est donc :

  1. coder et tester ;
  2. relire, corriger ;
  3. **bumper le patch** ;
  4. **`make manual`** — le bump vient de périmer l'exemple `--version`, qui
     cite le numéro. Ça tient en dix secondes, et l'oublier fait échouer
     l'étape suivante : `check-local` enchaîne `manual-check`, qui compare ce
     bloc à ce que le binaire écrit. Le défaut a été payé trois fois avant
     d'être inscrit ici ;
  5. **`make check-local`**, dont le `make bench` qu'il enchaîne tourne sur la
     version bumpée : **c'est cette exécution-là qui enregistre la mesure qui
     reste dans le journal** — à moins que la machine ne soit occupée, auquel
     cas rien n'est enregistré et il faut le dire dans la PR (issue #270) ;
  6. `make check` — une seule fois, elle voit le code *et* le bump ;
  7. **mettre à jour le reste de la documentation et le relevé de mesures** —
     notes, sections de manuel que le ticket change, chiffres relevés à
     l'étape 5. Rien de tout cela n'est lu par la porte, donc rien ne la
     réouvre ;
  8. commiter, régénérer le CHANGELOG, ouvrir la PR.

  **`check-local` avant `check`, et il y a une mesure derrière ce sens-là.**
  L'ordre inverse a tenu jusqu'à la fin de la phase 8, et il coûtait presque un
  relevé de banc sur deux : `check-local` enchaîne `make bench` en dernier, donc
  le banc trouvait la traîne d'un quart d'heure de compilation — six relevés
  perdus sur treize en phase 7, trois sur six en phase 8. C'est aussi l'ordre
  que le principe voulait déjà, du moins cher au plus cher : `check` dure
  dix-sept minutes, `check-local` quelques-unes.

  **Cet échange ne suffisait pas, et sa première exécution l'a dit** — le banc a
  lu 2,21 sur une machine partie de 0,63. Le garde compilait l'arbre Release
  *puis* demandait si la machine était libre, c'est-à-dire juste après l'avoir
  occupée lui-même ; la moyenne d'une minute est le passé. La lecture passe
  désormais avant la construction.
  [ADR 0015](docs/adr/0015-memoire-des-mesures.md).

  **Il n'y a donc plus d'exécution diagnostique de `make bench` avant la
  porte** : le relevé qui reste au journal lui est désormais antérieur, et une
  régression s'y voit au même moment, tant qu'il est temps de la corriger.

  L'étape 7 vient après la 6 par construction : c'est la seule position où la
  documentation ne coûte rien. L'exemple de version, lui, ne peut pas y
  attendre — il est lu par `check-local`, d'où l'étape 4 qui lui est réservée.

  Le tag et le `project(VERSION)` doivent porter le même numéro : **bumper le
  CMake avant de tagger**, sinon le binaire annonce une version périmée.
  `src/scripts/check-architecture.sh` le vérifie dès qu'un tag pointe sur HEAD.
- **Qualité** — `make check` est la porte : format, warnings en erreurs,
  clang-tidy, tests sous ASan, seuil de couverture, et **aucun fichier laissé
  derrière**. La CI exécute la même cible. Ne jamais annoncer un travail
  terminé sans l'avoir lancée.

  > **Du 2026-08-27 au 2026-09-01, la CI ne l'exécute plus.** Le quota
  > d'Actions du mois est épuisé, donc `ci.yml` et `pull-request.yml` sont
  > débranchés et le ruleset qui exigeait leur check est en `disabled`. La
  > phrase ci-dessus n'en change pas d'un mot — elle en devient seulement la
  > seule garde, puisque plus personne d'autre ne la vérifie. Les contrôles de
  > pull request étant eux aussi coupés, `Closes #N`, le bump du patch et le
  > journal régénéré redeviennent des gestes à faire, pas à voir échouer.
  > Rétablissement : `docs/configuration-github.md`.

  Le dernier contrôle est le plus récent et le moins évident : la porte relève
  les fichiers non suivis avant de commencer et refuse ceux qui sont apparus
  entre-temps. Ce sont les tests de bout en bout qui écrivent, et un nom nu
  passé à `--output` atterrit là où CTest les lance, c'est-à-dire à la racine du
  dépôt. **Un test qui écrit passe par le harnais `Scratch`, jamais par un nom
  nu.** Un fichier non suivi déjà présent avant la porte — une source neuve
  avant son `git add` — ne la dérange pas ; c'est l'apparition qui est refusée,
  pas la présence.

  **Le même contrôle existe hors du dépôt, et il a sa propre règle.**
  `check-config-home.sh` encadre la porte de la même façon et surveille
  `~/.config/subedit`, que `git ls-files` ne voit pas. Ce qui le tient au vert :
  **un test ne résout jamais un emplacement de configuration, il en reçoit
  un.** Le seul code qui en résout un est `gui::userSettingsPath()` — ADR 0022 —
  et rien ne l'appelle pour écrire. Les trois harnais qui lancent quelque chose
  déplacent en plus `XDG_CONFIG_HOME` vers un répertoire à eux : le binaire de
  tests d'interface pour lui-même, celui de bout en bout pour chaque binaire
  qu'il lance, et `check-installation.sh` pour ce qu'il vient d'installer.

  **L'analyse statique n'a plus de périmètre à calculer, et plus de bouton.**
  clang-tidy est accroché à la règle de compilation de chaque source, sous le
  preset `tidy` — `cmake/Tidy.cmake`, issue #269. Ce qui décide de réanalyser un
  fichier est le système de construction : sa source, un en-tête de son fichier
  de dépendances, ou sa ligne de commande. Il n'y a donc plus de `TIDY_BASE`,
  plus de « fichiers gouvernants », et plus de tableau de ce que la porte lit —
  **une modification qui ne change aucune entrée de compilation ne coûte rien,
  mécaniquement.** Pour tout réanalyser : `rm -rf build/tidy`.

  Le prix, écrit plutôt que tu : l'incrémentalité vit dans `build/tidy`, donc un
  `make clean` la perd, et la CI la garde par un cache d'Actions.

  **Deux entrées échappent à Ninja, et une clé les lui montre** : le contenu de
  `.clang-tidy` et la version du binaire, dont ni l'un ni l'autre n'est sur la
  ligne de commande. `Tidy.cmake` en pose l'empreinte dans les drapeaux de
  compilation, ce qui les y met. Sans elle, une configuration durcie laisserait
  l'arbre vert sur une réponse périmée.

  **`HeaderFilterRegex` n'avait jamais filtré**, et personne ne pouvait le voir :
  le motif `src/.*\.hpp$` ne correspond à rien depuis la racine du dépôt, si
  bien que la porte n'a analysé aucun en-tête jusqu'à #269. Le preset a réglé
  cela par construction, son répertoire courant étant l'arbre de construction.
  Corollaire à connaître : **un `.clang-tidy` de répertoire ne gouverne que les
  `.cpp` de ce répertoire** — pour un diagnostic levé dans un en-tête, c'est la
  configuration de l'unité de traduction qui l'inclut qui s'applique.

  **On sait reprendre la porte au milieu.** Les étapes vivent dans
  `src/scripts/gate/`, une par fichier, et `src/scripts/gate.sh` porte l'ordre :

  ```console
  $ ./src/scripts/gate.sh --list
  $ ./src/scripts/gate.sh check --from coverage
  $ ./src/scripts/gate.sh check --only tidy
  ```

  Les cibles `make` restent l'interface qu'on tape, et `verify-gates.sh` les
  invoque toujours. Un nom d'étape inconnu fait échouer l'invocation plutôt que
  de n'en jouer aucune.

  **Le bump de version, lui, ne compte que si un tag pointe sur HEAD.** C'est le
  seul contrôle qui lit le numéro : `check-architecture.sh` le confronte au tag,
  et il est inerte sans tag. Rien d'autre ne peut en différer — `version_test`
  et le test de bout en bout de `--version` dérivent tous deux le numéro de
  `versionString()` plutôt que de l'écrire en dur, et le seul endroit où il est
  recopié, l'exemple `--version` du manuel, est tenu par `manual-check`, qui est
  dans `check-local`.

  Donc : **une PR qui ne touche que de la documentation et le patch se contente
  de `make check-local`.** C'est le cas de toute PR de cadrage, de manuel ou
  d'ADR. Dès qu'un `.cpp`, un `.hpp`, un script ou une donnée de test bouge, la
  porte reprend ses droits.
- **Manuel utilisateur** — [`docs/manual/`](docs/manual/). **Tout ticket qui
  change ce que l'utilisateur voit se termine par sa mise à jour**, une fois le
  code écrit, relu et validé — pas avant, sinon le manuel décrit une intention
  et non un logiciel. Concrètement, à l'étape 7 de l'ordre décrit plus haut :
  après la porte, qui ne le lit pas. Ce qui doit y figurer, exhaustivement :

  | Élément | Ce qu'on en dit |
  | :------ | :-------------- |
  | commande, sous-commande | son nom, ce qu'elle fait, et ce qu'elle ne fait pas |
  | argument, option | sa forme longue et courte, s'il est requis, sa valeur par défaut |
  | valeurs acceptées | l'ensemble fermé, énuméré ; les bornes, pour un intervalle |
  | sortie | ce qui est écrit, où, et sous quelle forme |
  | codes de retour | chacun, avec sa signification |
  | erreurs | ce qui les déclenche et le message correspondant |

  Un exemple d'appel réel accompagne chaque commande. **Le manuel décrit ce qui
  existe, jamais ce qui est prévu** — le prévu va dans la feuille de route.

  **Les images du manuel d'interface sont engendrées, jamais prises à la
  main.** `make manual` construit la vraie fenêtre, la montre sans écran et la
  photographie ; le programme n'écrit jamais une référence, il écrit
  `<nom>.new.png`, et `compare-screenshots.py` promeut ou efface — si bien
  qu'une image n'entre dans un diff que le jour où l'interface a changé.
  `check-screenshots.py` attrape ce qu'aucun des deux ne voit : une image que
  le manuel montre et que rien n'engendre. Une nouvelle capture s'ajoute dans
  `src/test/tools/screenshots.cpp`, et elle ne fait foi que sous les réglages
  que ce programme pose — plateforme sans écran, style Fusion, `DejaVu Sans` ;
  il refuse de photographier sans cette police. ADR 0024.
- **Définition de « terminé »** — code, tests, **benchmarks rejoués et mesures
  relevées**, section de manuel à jour, patch bumpé, entrée de CHANGELOG
  régénérée.

  Les benchmarks se rejouent **à chaque issue**, et non seulement quand la
  performance est en jeu : une mesure ne dit rien seule, elle ne parle que
  comparée à la précédente. Ils tournent en `Release`, le mode livré, et la
  porte ne les construit pas — c'est donc une commande à part,
  `make bench`. Tant qu'ils prennent une minute, la question de leur coût ne
  se pose pas ; elle se posera, et se traitera alors par un sous-ensemble.

  **« Mesures relevées » admet une absence, et une seule** — issue #270.
  `make bench` attend trente secondes que la charge passe sous
  `BENCH_MAX_LOAD` ; au-delà il mesure, affiche, et **n'inscrit rien**. Une
  section de journal qu'on ne peut comparer à rien occupe la place de celle qui
  manque et cache le fait qu'il n'y a pas eu de mesure ; une version sans
  section le dit. Ce qui reste dû, alors, est une phrase dans la PR : pas de
  mesure pour cette version, et pourquoi. Rejouer `make bench` au calme la
  produit.
