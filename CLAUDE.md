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
- **Commits** — Conventional Commits, scopes alignés sur les labels `area:`.
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

  **Le bump se fait au dernier moment, juste avant d'ouvrir la PR** — jamais en
  début de travail. Le numéro n'est connu qu'à ce moment-là : entre le premier
  commit et la PR, d'autres PR ont pu être fusionnées et avoir pris le numéro
  visé. Bumper tôt, c'est se garantir un conflit sur `CMakeLists.txt` et un
  rebase de plus. L'ordre est donc : coder, `make check`, relire, corriger,
  **puis** bumper le patch, régénérer le CHANGELOG, ouvrir la PR.

  Le tag et le `project(VERSION)` doivent porter le même numéro : **bumper le
  CMake avant de tagger**, sinon le binaire annonce une version périmée.
  `src/scripts/check-architecture.sh` le vérifie dès qu'un tag pointe sur HEAD.
- **Qualité** — `make check` est la porte : format, warnings en erreurs,
  clang-tidy, tests sous ASan, seuil de couverture. La CI l'exécute à
  l'identique. Ne jamais annoncer un travail terminé sans l'avoir lancée.
- **Manuel utilisateur** — [`docs/manual/`](docs/manual/). **Tout ticket qui
  change ce que l'utilisateur voit se termine par sa mise à jour**, une fois le
  code écrit, relu et validé — pas avant, sinon le manuel décrit une intention
  et non un logiciel. Ce qui doit y figurer, exhaustivement :

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
- **Définition de « terminé »** — code, tests, benchmark si la performance est en
  jeu, **section de manuel à jour**, patch bumpé, entrée de CHANGELOG
  régénérée.
