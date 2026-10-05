# `split-file`

```
subedit-cli split-file --at N --head FICHIER --tail FICHIER
                       [--dry-run]
                       <fichier>
```

Coupe un fichier **en deux** : une tête et une queue. C'est `Split Project…` de la fenêtre — [le même moteur](../subedit-gui/operations.md#split-project),
les mêmes comptes, le même refus — et **l'inverse exact de [`append`](append.md)** : ajouter la queue à la
tête rend le fichier de départ.

- **`--at N` est le numéro du premier sous-titre de la queue**, numéroté comme la table de la fenêtre le
  numérote : à partir de 1. `--at 3` met les sous-titres 1 et 2 dans la tête, et le troisième et la suite dans la
  queue. Il va **du deuxième au dernier** : une coupure veut un sous-titre de chaque côté.
- **La queue est ramenée à l'origine** : elle est décalée de moins la fin du dernier sous-titre resté dans la tête,
  ses positions gardant leur ordre et leur écart. C'est ce qui fait de `split-file` l'inverse d'`append`.
- **Les deux moitiés gardent le format, l'encodage, la marque d'octets et les fins de ligne du fichier lu**, et
  sont renumérotées à partir de 1 par l'écriture, comme tout fichier réécrit.
- **Deux sorties, donc une destination à elle** : `--head` et `--tail`, **tous deux requis**. `--output`,
  `--output-dir`, `--in-place` et `--recursive` n'existent pas pour `split-file` et sont refusés comme toute option
  inconnue, code `1` : il n'y a ni dossier où répartir deux fichiers nommés, ni entrée à écraser.

<!-- exemple: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>` | oui | **un seul** chemin | — |
| `--at` | oui | un entier, de 2 au nombre de sous-titres du fichier | — |
| `--head`, `--tail` | **oui tous les deux**, sauf avec `--dry-run` | des chemins de fichier ; leur dossier est créé s'il manque | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**Avec `--dry-run`, aucune destination n'est exigée** : on peut n'en donner aucune, mais pas une seule des deux —
`--head` et `--tail` vont ensemble. Celles qu'on donne sont jugées comme sans `--dry-run`.

**Les deux destinations ne peuvent être ni l'entrée ni un même fichier** : écrire par-dessus ce qu'on est en train
de lire, ou écrire la queue sur la tête, est refusé avant que rien soit lu, code `1`.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Le deuxième sous-titre, resté dans la tête, finit à 4 s : la queue recule de 4 s, et le troisième sous-titre,
qui commençait à 6 s, commence à 2 s.

## Quand la coupure est refusée

**Deux moitiés qui se chevauchent à la coupure ne se coupent pas.** Si un sous-titre de la queue commençait avant
la fin du dernier sous-titre resté, le décalage le porterait avant le début de la vidéo — une position négative,
qu'aucun format n'écrit. `split-file` refuse **en nommant le sous-titre**, avec les mots de la fenêtre :

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Le code est `2` et **rien n'est écrit**. On coupe ailleurs. Arriver exactement au début est permis : zéro est une
position.

**`--at` hors du fichier est refusé de la même façon**, le fichier lu : `--at 1` — le premier sous-titre ne peut
pas commencer une queue, il ne resterait rien — et `--at` au-delà du dernier sous-titre, qui laisserait une queue
vide. Un fichier d'un seul sous-titre ne se coupe pas.

## Sortie

**Sortie standard** — rien : le résultat est les deux fichiers écrits.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <tête>, <queue>` ; avec `--dry-run`, `(dry run, nothing written)` à la place de `-> …` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, H and T written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), **un seul objet**, dont `file` est le fichier coupé.
**`destination` est la tête** — le fichier que l'entrée devient — et **`tail` la queue**, un chemin, ou `null` avec
`--dry-run` comme `destination`. Les `counts` : `subtitles` (ceux du fichier lu), `head` et `tail` (ceux de chaque
moitié).

## Codes de retour

| Code | Quand |
| :--- | :---- |
| `0` | les deux fichiers sont écrits — ou calculés, avec `--dry-run` |
| `1` | erreur d'usage : pas de `--head` et de `--tail`, une option que `split-file` n'a pas, une destination qui est l'entrée ou qui est l'autre destination, un `--at` qui n'est pas un entier positif |
| `2` | le fichier n'a pas pu être lu, **la coupure est refusée** (hors du fichier, ou chevauchement), ou une destination n'a pas pu être écrite. Il n'y a pas de code `3` : un seul fichier est traité |

**La tête est écrite la première** : un disque qui refuse la queue laisse la tête derrière lui, et le code est `2`.

## Erreurs

| Ce qui les déclenche | Message |
| :------------------- | :------ |
| ni `--head` ni `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| une seule des deux, avec `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| les deux destinations sont un même fichier | `<chemin>: named as both the head and the tail` |
| une destination est l'entrée | `<chemin>: would be written over the input <entrée>` |
| `--at` hors du fichier | `<fichier>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un fichier d'un sous-titre | `<fichier>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| chevauchement | `<fichier>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un fichier absent, illisible ou d'un format inconnu | `<chemin>: does not exist`… — les mêmes que partout |
