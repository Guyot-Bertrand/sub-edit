# `append`

```
subedit-cli append --output FICHIER
                   [--dry-run]
                   <base> <fichier>...
```

Met des fichiers **à la suite du premier**, dans **un seul** fichier : le film en deux parties, rendu en un
seul jeu de sous-titres. C'est `Append File…` de la fenêtre — [le même moteur](../subedit-gui/operations.md#append-file),
les mêmes comptes — appliqué autant de fois qu'il y a de fichiers à ajouter.

- **Le premier fichier est la base.** Il donne au résultat **son format, son encodage, sa marque
  d'octets et ses fins de ligne**.
- **Rien n'est inséré entre deux fichiers** : chacun est décalé de **la fin du dernier sous-titre de ce qui
  le précède**, ses positions gardant leur ordre et leur écart. Le troisième suit le deuxième, non la
  base : les fichiers se suivent dans l'ordre où ils sont donnés.
- **Chaque fichier ajouté traverse la conversion** vers le format de la base : ses balises sont traduites,
  et ce que ce format ne sait pas écrire est dit, fichier par fichier, dans les mots de la fenêtre.
- **Les fichiers ajoutés n'ont pas de traduction** : un fichier ouvert seul n'en a pas à donner.
- **Un fichier peut être donné plusieurs fois**, ou ajouté à lui-même : rien ne l'interdit.

**`append` n'est pas un lot.** N entrées, **une** sortie : la boucle des autres sous-commandes — un résultat
par fichier, et un échec qui laisse les autres aller — n'a pas de sens ici. Il en découle trois choses :

- **la destination est `--output`, et rien d'autre.** `--output-dir` et `--in-place` n'existent pas pour
  `append` et sont refusés comme toute option inconnue, code `1` ; `--recursive` de même : l'ordre des
  fichiers est celui qu'on donne, et un arbre parcouru n'en a pas ;
- **tout ou rien** : le premier fichier qui ne se lit pas arrête le lancement, et **rien n'est écrit**. Un
  film en trois parties dont une manque n'est pas un film en deux ;
- **une seule ligne par lancement** pour le résultat, et, en JSON, **un seul objet**.

<!-- exemple: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<base> <fichier>...` | oui, **deux au moins** | des chemins : la base, puis ce qu'on met à sa suite, dans l'ordre | — |
| `--output` | **oui**, sauf avec `--dry-run` | un chemin de fichier ; son dossier est créé s'il manque | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**La destination ne peut être aucune des entrées**, la base comprise : écrire par-dessus un fichier qu'on est
en train de lire est refusé avant que rien soit lu, code `1`. `--output` existant et distinct des entrées est
écrasé, comme partout.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Le dernier sous-titre de `partie1.srt` finit à 4 s ; celui de `partie2.srt`, qui commençait à 0,5 s, commence à
4,5 s.

## Quand les formats diffèrent

Le résultat est dans **le format de la base**, quel que soit celui des autres. Ce que la traversée coûte est
dit fichier par fichier, avec les mots de la fenêtre — la même phrase que `Append File…` et que
[le collage d'un autre format](../subedit-gui/presse-papiers.md#les-balises-dun-autre-format-sont-traduites) :

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Ce que l'on perd ne dépend que du format de **chaque** fichier ajouté : une base en Advanced SSA reçoit un
fichier SubRip sans rien perdre, l'inverse non. Pour choisir le format du résultat, on met en premier un fichier
qui est déjà dans ce format, ou l'on convertit ensuite avec [`convert`](convert.md).

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | une ligne par fichier ajouté — `<chemin>: appended N subtitles`, suivie, quand le format a coûté quelque chose, de `; <format source> into <format de la base>: <ce qui est perdu>` — puis `<base>: N subtitles from M files -> <destination>` (avec `--dry-run`, `(dry run, nothing written)` à la place de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne de la base, remis tels quels |
| 3 | `<base>: N bytes read, M written` (tous les fichiers ensemble), puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), **un seul objet** pour tout le lancement,
dont `file` est **la base**. Ses `counts` :

| Compte | Ce qu'il dit |
| :----- | :----------- |
| `files` | les fichiers du lancement, la base comprise |
| `subtitles` | les sous-titres du résultat |
| `appended` | ceux qui viennent des fichiers ajoutés |
| `lost_ends`, `lost_header` | `0` ou `1` : un des fichiers ajoutés l'a perdu |
| `joined_lines`, `lost_tags`, `lost_fields` | la somme de ce que les fichiers ajoutés ont perdu |
| `furthest_ms` | le plus grand déplacement d'une position, tous fichiers confondus |

**Un tableau `inputs`** accompagne les `counts`, hors d'eux : un élément par fichier ajouté, dans l'ordre,
`{"file", "counts"}` — `counts` portant `appended`, puis les mêmes pertes, **pour ce fichier seul**. C'est ce qui
dit, dans un lancement de dix fichiers, lequel a coûté quoi. Les avertissements de lecture de tous les fichiers
sont réunis dans `warnings`.

Un échec est un objet lui aussi, **un seul** : celui du premier fichier qui n'a pas pu être lu.

## Codes de retour

| Code | Quand |
| :--- | :---- |
| `0` | le résultat est écrit — ou calculé, avec `--dry-run` |
| `1` | erreur d'usage : moins de deux fichiers, pas de `--output`, une option qu'`append` n'a pas, une destination qui est une des entrées |
| `2` | **un fichier n'a pas pu être lu**, ou la destination n'a pas pu être écrite : rien n'est écrit. Il n'y a pas de code `3` : un lancement qui n'écrit qu'un fichier est réussi ou ne l'est pas |

## Erreurs

| Ce qui les déclenche | Message |
| :------------------- | :------ |
| un seul fichier donné | `append needs a base file and at least one file to put after it` |
| ni `--output` ni `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` désigne une entrée | `<destination>: would be written over the input <entrée>` |
| un fichier absent, illisible ou d'un format inconnu | `<chemin>: does not exist`, `<chemin>: is in no format this tool knows`… — les mêmes que partout |

**Un fichier vide ne s'ajoute pas** : aucun format ne se lit sans un sous-titre au moins, et le fichier est
refusé comme tout fichier qui ne se lit pas — `<chemin>: holds nothing recognisable as a subtitle` —, ce qui
arrête le lancement et n'écrit rien.
