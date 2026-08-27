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

- **Langue** — français pour la documentation, les specs, les ADR, les manuels,
  les messages de commit, les scripts, le système de construction et les
  échanges. Anglais pour le code C++ : identifiants, commentaires et intitulés
  de tests. La frontière est celle du compilateur C++, pas celle du dépôt.

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
  3. **rejouer `make bench`** et regarder les chiffres. Avant la porte, parce
     qu'une régression se corrige tant qu'il est temps — la découvrir après
     obligerait à tout reprendre. **Cette exécution reste diagnostique :** la
     version n'a pas encore bougé, donc son relevé ne doit pas rester dans le
     journal — il porterait le nom de la version précédente ;
  4. **bumper le patch** ;
  5. **`make manual`** — le bump vient de périmer l'exemple `--version`, qui
     cite le numéro. Ça tient en dix secondes, et l'oublier fait échouer
     l'étape suivante : `check-local` enchaîne `manual-check`, qui compare ce
     bloc à ce que le binaire écrit. Le défaut a été payé trois fois avant
     d'être inscrit ici ;
  6. `make check` — une seule fois, elle voit le code *et* le bump — puis
     `make check-local`, dont le `make bench` qu'il enchaîne tourne cette
     fois sur la version bumpée : **c'est cette exécution-là, postérieure au
     bump, qui enregistre la mesure qui reste dans le journal** ;
  7. **mettre à jour le reste de la documentation et le relevé de mesures** —
     notes, sections de manuel que le ticket change, chiffres relevés à
     l'étape 6. Rien de tout cela n'est lu par la porte, donc rien ne la
     réouvre ;
  8. commiter, régénérer le CHANGELOG, ouvrir la PR.

  L'étape 7 vient après la 6 par construction : c'est la seule position où la
  documentation ne coûte rien. L'exemple de version, lui, ne peut pas y
  attendre — il est lu par `check-local`, d'où l'étape 5 qui lui est réservée.

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

  **Elle n'analyse que ce qui a changé, en local comme en CI.**
  `src/scripts/tidy-scope.sh` calcule le périmètre depuis `TIDY_BASE`
  — `origin/main` par défaut — en fermeture transitive des en-têtes, en voyant
  le travail non commité, et **il retombe sur l'analyse complète au moindre
  doute**. C'est ce qui rend la porte tenable : clang-tidy en est 90 %, et le
  cas courant tombe de 827 s à **72 s**. `make check TIDY_BASE=` analyse tout ;
  la CI le fait chaque semaine sur `main`.

  **Elle ne se relance que si elle peut voir la différence.** Les modifications
  qu'elle ne lit pas ne valent pas son coût. Ce qu'elle lit :

  | Elle voit | Elle ne voit pas |
  | :-------- | :--------------- |
  | `src/**/*.cpp`, `src/**/*.hpp` | `docs/**`, y compris le manuel |
  | `CMakeLists.txt`, `CMakePresets.json`, `cmake/*.cmake` | `CHANGELOG.md`, `CLAUDE.md`, `README.md` |
  | `Makefile` | `.github/workflows/**` — c'est la CI qui les lit |
  | `src/scripts/*.sh` | `cliff.toml`, `LICENSE` |
  | `src/test/data/**`, `src/data/**` — les tests de corpus les lisent | |

  Conséquence pratique : une note ajoutée à un document après une porte verte ne
  la réouvre pas.

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
- **Définition de « terminé »** — code, tests, **benchmarks rejoués et mesures
  relevées**, section de manuel à jour, patch bumpé, entrée de CHANGELOG
  régénérée.

  Les benchmarks se rejouent **à chaque issue**, et non seulement quand la
  performance est en jeu : une mesure ne dit rien seule, elle ne parle que
  comparée à la précédente. Ils tournent en `Release`, le mode livré, et la
  porte ne les construit pas — c'est donc une commande à part,
  `make bench`. Tant qu'ils prennent une minute, la question de leur coût ne
  se pose pas ; elle se posera, et se traitera alors par un sous-ensemble.
