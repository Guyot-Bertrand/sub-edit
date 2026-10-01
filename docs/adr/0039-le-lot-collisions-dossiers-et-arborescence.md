# 0039 — Un lot sûr : collisions refusées, dossier créé, arborescence conservée

**Date :** 2026-10-01
**Statut :** acceptée

Décidée en cadrant la phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542), sur les constats de
[#544](https://github.com/Guyot-Bertrand/sub-edit/issues/544).

## Contexte

Le lot de la phase 3 est une boucle séquentielle sur des chemins donnés : chacun est lu,
traité et écrit indépendamment, les échecs sont nommés, le code de retour est global
(`0`, `2`, `3`). **Cela suffisait tant que le lot était un harnais** ; ce n'est plus assez
pour ce qu'un script fait à des milliers de fichiers. Les tests de #544 ont écrit trois
comportements qui n'avaient jamais été décidés, et que le manuel
(`docs/manual/subedit-cli/invocation.md`) avoue sous « trois choses que l'outil ne fait
pas » :

- **deux entrées de même nom de base** — `a/film.srt`, `b/film.srt` — **écrivent au même
  endroit** avec `--output-dir` : code `0`, chaque entrée annonce sa destination, la seconde
  écrase la première, sans un mot ;
- **le dossier de `--output-dir` n'est pas créé** : s'il manque, toute entrée échoue, et le
  libellé dit « `cannot be read` » pour une **écriture** — `reasonOf(FileErrorKind::Io)` est
  la phrase de la lecture, que l'écriture emprunte — alors que le manuel annonce « `cannot be
  opened: permission denied` » ;
- **une destination qui existe est écrasée**, sans question ni option, par écriture
  atomique.

Un quatrième, que #544 n'a pas vu et que l'analyse du cadrage a trouvé : `--output-dir .`
avec des entrées qui sont dans `.` **écrase les entrées elles-mêmes**, sans `--in-place`. La
règle « rien n'est écrit sans destination explicite, et écraser son entrée est un geste
qu'on nomme » (spec 03) n'y tient pas.

Et la phase demande ce que la boucle ne sait pas faire : prendre des **répertoires** en
entrée.

## Décision

**Quatre règles de sûreté, et une extension.**

1. **Deux entrées qui aboutissent à la même destination sont refusées avant tout écrit.**
   Erreur d'usage, code `1`, **aucun fichier écrit** — la règle de `CLI-USAGE-03` : une
   erreur d'usage ne laisse jamais un lot à moitié écrit. Le message nomme la destination
   **et les deux entrées**. La comparaison porte sur la destination **finale** — extension
   comprise, `convert` la change — et se fait sur les chemins normalisés.
2. **Une destination qui est l'une des entrées est refusée sans `--in-place`**, même règle,
   même code, mêmes raisons ; `std::filesystem::equivalent` tranche quand le fichier existe.
3. **Le dossier de sortie est créé** s'il manque, parents compris, **après** la validation
   de la ligne de commande et **avant** le premier fichier traité ; jamais en `--dry-run`.
   Le dossier où `--output FICHIER` écrit suit la même règle. Si la création échoue, **c'est
   dit une fois**, aucun fichier n'est lu, et le code est `2`.
4. **Une destination qui existe est écrasée**, par écriture atomique — le contenu d'avant
   est perdu, le fichier n'est jamais écrit à moitié. **Ce n'est pas un défaut qu'on garde :
   c'est la règle, et le manuel la dit** dans la section de la destination, en tête et non
   plus sous une rubrique de réserves.
5. **Des répertoires en entrée, avec `--recursive` (`-r`).**

   - Un répertoire donné **sans** `--recursive` est une erreur d'usage (code `1`) :
     `<dossier>: is a directory: use --recursive`. Le refuser vaut mieux que de n'en
     traiter rien et de rendre `0`.
   - Avec `--recursive`, le parcours est **récursif, déterministe et sans surprise** :
     les entrées de chaque répertoire sont triées par nom (comparaison d'octets), en
     profondeur d'abord ; **les liens symboliques ne sont pas suivis** (pas de boucle) ;
     **les fichiers cachés sont ignorés**.
   - **Seuls les fichiers dont l'extension est celle d'un format connu sont retenus** :
     `.srt`, `.vtt`, `.ssa`, `.ass`, `.lrc`, `.sub`. **`.txt` est exclu du parcours** : il
     désigne MPL2 et TMPlayer, mais surtout tous les `LISEZMOI.txt` du monde, et un lot qui
     échouerait sur chacun serait inutilisable. **Un fichier nommé sur la ligne de commande
     n'est jamais filtré** : le nommer, c'est le vouloir. Ce qui a été laissé de côté est
     compté, et dit au niveau 2.
   - **L'arborescence est conservée sous `--output-dir`** : le chemin d'un fichier **relatif
     au répertoire donné** est reproduit sous le dossier de sortie (`films/a/x.srt`,
     donné `films/`, s'écrit `out/a/x.srt`). Le nom du répertoire donné n'en fait pas partie
     — la sémantique de `rsync` avec une barre finale, non celle de `cp -r` — : l'utilisateur
     nomme la destination, et ajoute le niveau qu'il veut. **Avec deux répertoires, la règle 1
     tranche** ce qui se recouvre.
   - **Le dossier de sortie, s'il est dans l'arbre parcouru, est exclu du parcours** : une
     seconde exécution ne relit pas ce que la première a écrit.
   - La liste des entrées est **établie en entier avant le premier écrit** — c'est ce qui
     rend les règles 1 et 2 vérifiables avant tout, et le parcours indépendant de ce que
     l'écriture change.
   - `--in-place` avec `--recursive` réécrit chaque fichier retenu à sa place.
6. **Le libellé d'une écriture qui échoue dit « written »**, jamais « read » ni « opened » :
   `<destination>: cannot be written: permission denied`, ou `: cannot be written` pour le
   reste. La lecture et l'écriture ne partagent plus `reasonOf(FileErrorKind)`.

**Ce qui ne change pas** : chaque fichier est traité indépendamment, l'échec de l'un
n'arrête pas les autres, les échecs sont nommés, les codes de retour restent les quatre de
la phase 3. Le parcours produit **une liste de chemins**, que le reste du lot consomme comme
avant : `--format json` (ADR 0038) y trouve ses « un objet par entrée ».

## Alternatives écartées

- **Renommer la seconde entrée** (`film-2.srt`). Le nom d'un fichier écrit devient une
  fonction de l'ordre des entrées : un script qui relance le même lot dans un autre ordre
  écrit autre chose, et celui qui cherche `film.srt` ne sait pas lequel il a trouvé. Un
  refus coûte une ligne ; un renommage silencieux coûte une enquête.
- **Laisser passer la première, échouer la seconde** (code `3`). Un lot à moitié écrit est
  le pire cas, et la règle de la phase 3 existe pour l'empêcher. La collision est connue
  *avant* d'écrire : la dire alors est possible, donc exigible.
- **Garder « la dernière gagne »** et l'écrire au manuel. C'est l'état d'aujourd'hui ; il
  perd un fichier sans que rien ne le dise.
- **Refuser un dossier de sortie absent.** Plus prudent contre une faute de frappe, et
  c'était la pente du premier constat de #544. Écarté : l'utilisateur **nomme** ce dossier,
  il est le produit de la commande, et lui demander un `mkdir` préalable est le geste que
  chaque script ajoute. La faute de frappe crée un dossier de plus, visible et inoffensif,
  là où l'écrasement d'un fichier ne l'est pas.
- **Une option `--no-clobber`.** Légitime, et non demandée : la collision et l'atomicité
  couvrent les deux dégâts qu'on a vus. Candidate si un appelant la réclame.
- **S'en remettre au shell** (`**/*.srt`, `find … -exec`). `globstar` est désactivé par
  défaut dans bash, `**` n'existe pas partout, la liste dépasse parfois la longueur d'une
  ligne de commande, et surtout **le shell ne conserve pas l'arborescence sous un dossier
  de sortie** : c'est la moitié du besoin.
- **Un motif de noms (`--include '*.fr.srt'`)**. Utile, et plus de grammaire : les
  extensions connues règlent le cas courant. Candidat.
- **Suivre les liens symboliques.** Une boucle d'arborescence est un défaut qu'on trouve en
  production ; ne pas les suivre en est un qu'on lit au manuel.
- **Un lot parallèle.** La boucle est séquentielle, son coût est la somme de ses fichiers,
  et la sortie reste déterministe sans rien arbitrer. La mesure qui en dirait l'intérêt
  naît avec le parcours (issue du lot, banc de #541) ; le parallélisme n'est retenu que si
  elle le montre — et il demanderait de rouvrir l'ordre des objets de l'ADR 0038.
- **Inclure le nom du répertoire donné sous `--output-dir`** (`cp -r`). Plus familier, mais
  l'on ne peut pas alors écrire « mon arbre `films/` dans `out/` sans niveau de plus » ; on
  peut toujours écrire `--output-dir out/films`.

## Conséquences

**Le comportement change pour qui comptait sur l'écrasement ou sur l'absence de création** :
deux entrées de même nom de base, qui s'écrasaient, sont maintenant refusées ; un dossier de
sortie absent, qui faisait échouer, est maintenant créé. Les tests de #544 qui **fixent
l'état d'avant** — `batch_test.cpp`, `generated_batch_test.cpp` — deviennent des diffs
visibles, c'est le but de les avoir écrits avant ; le paragraphe d'`invocation.md` se
réécrit.

**La liste des entrées est un objet** (`std::vector` de `{chemin donné, destination
calculée, relatif}`) que la validation, le parcours, l'écriture et le JSON se partagent ; la
destination se calcule **une fois**, à la validation, et l'écriture la relit.

**Le banc du lot** : le parcours rend le lot mesurable — N fichiers engendrés dans un
`Scratch` contre le même travail en un seul —, et la mesure naît avec lui, dans le journal.

**Ce qui justifierait de rouvrir** : un lot assez grand pour que la boucle séquentielle se
voie ; un appelant qui veut un motif de noms ou `--no-clobber` ; un système de fichiers
insensible à la casse qui ferait collisionner deux noms que la comparaison d'octets
distingue (la règle 1 s'appuie sur `equivalent` quand le fichier existe, et non sur la
chaîne, pour cela).

## Mise en œuvre de la première moitié (#554)

Relue à l'implémentation ; **aucune décision n'est changée**, trois précisions que
l'ADR ne disait pas :

- **Une écriture qui trouve son dossier absent dit « cannot be written »**, comme
  les autres refus d'écriture hors permission : le dossier de sortie est créé avant
  tout écrit, ce cas n'est plus l'ordinaire. La création qui échoue a sa propre phrase,
  `cannot be created` (`reasonOfCreating`).
- **La séparation lecture/écriture vaut aussi pour la fenêtre** : `reasonOf(SaveError)`
  et l'enregistrement des remplacements du correcteur disent « written » eux aussi.
  Les deux surfaces gardaient la même phrase fausse ; elles gardent maintenant la même
  phrase juste.
- **L'identité de deux chemins** se juge par l'orthographe absolue normalisée, puis par
  `FileSystem::equivalent` (nouveau, vrai quand les deux existent) — avec un tamis sur
  le nom de fichier, sans casse, pour que la comparaison reste linéaire dans la taille
  du lot.

## Mise en œuvre de la seconde moitié (#555)

Relue à l'implémentation ; **aucune décision n'est changée**, quatre précisions :

- **Le parcours passe par `FileSystem`** — `entriesIn` (entrées typées, liens non suivis,
  triées par octets) et `isDirectory` — et non par `std::filesystem` : il se teste sans
  disque, et l'ordre est celui du contrat, non celui du système.
- **« Laissé de côté » ne compte que les fichiers d'une extension inconnue** (dont
  `.txt`) : les entrées cachées et les liens sont ignorés sans bruit, parce que les
  compter ferait un chiffre que personne n'a demandé.
- **Un répertoire sans rien à parcourir est dit au niveau 1 et n'est pas une erreur** :
  le lot de zéro fichier réussit, comme tout lot vide.
- **L'arborescence passe par `Destination::withRoots`** plutôt que par un type de liste
  d'entrées : les fonctions du lot gardent leurs signatures, et une entrée trouvée sous
  une racine garde son chemin relatif tandis qu'une entrée nommée n'en garde que le nom.
