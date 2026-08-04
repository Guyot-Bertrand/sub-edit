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

Si le clone se retrouve modifié, le signaler et proposer la restauration — ne
pas la faire silencieusement.

## Le projet

`subedit` réécrit Gaupol — éditeur de sous-titres GTK/Python — en C++20 + Qt 6,
avec un objectif d'iso-fonctionnalité.

| Document | Contenu |
| :------- | :------ |
| [`docs/feuille-de-route.md`](docs/feuille-de-route.md) | les huit phases, leur cadrage et leur ordre |
| [`docs/principes-de-conception.md`](docs/principes-de-conception.md) | règles permanentes applicables à tout le code |
| [`docs/specs/`](docs/specs/) | une spec par phase, plus l'inventaire de Gaupol |
| [`docs/configuration-github.md`](docs/configuration-github.md) | verrouillage du dépôt, labels, milestones |

## Conventions

- **Langue** — documentation, specs, messages de commit et échanges en français.
  Le code, ses identifiants et ses commentaires en anglais.
- **Commits** — Conventional Commits, scopes alignés sur les labels `area:`.
- **Qualité** — `make check` est la porte : format, warnings en erreurs,
  clang-tidy, tests sous ASan, seuil de couverture. La CI l'exécute à
  l'identique. Ne jamais annoncer un travail terminé sans l'avoir lancée.
- **Définition de « terminé »** — code, tests, benchmark si la performance est en
  jeu, section de manuel, entrée de CHANGELOG régénérée.
