# `sort`

```
subedit-cli sort (--output FICHIER | --output-dir DOSSIER | --in-place)
                 [--dry-run]
                 [--recursive]
                 <fichier>...
```

Remet les sous-titres **dans l'ordre de leur début**. Rien d'autre ne change : ni les textes, ni les
positions, ni le format du fichier. Les sous-titres sont **renumérotés** à partir de 1 par l'écriture,
comme pour tout fichier réécrit.

**Le noyau ne trie jamais de lui-même** : lire un fichier laisse son ordre tel qu'il était, et
[`inspect`](inspect.md#ce-que-anomalies-rapporte) signale une rupture d'ordre par
`subtitle N starts before the previous one starts`. `sort` est **le geste fait exprès**, pour un fichier
ou un lot dont l'ordre est rompu — et il se défait, dans la fenêtre, comme n'importe quelle opération.

- **Le tri est stable** : deux sous-titres qui commencent au même instant **gardent l'ordre que le
  fichier leur donnait**. Aucun ne précède l'autre ; les déplacer serait une décision que personne n'a
  demandée.
- **`sort` ne prend ni plage ni document** : il n'y a rien à choisir, l'ordre est celui des débuts.

<!-- exemple: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

<!-- exemple: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Ce qui est compté

**Le compte est celui des places qui ont changé de sous-titre**, tel que le noyau le rapporte — ce que
l'historique de la fenêtre appellerait déplacé. Un seul sous-titre mal placé en décale plusieurs : dans
l'exemple ci-dessus, aucune des trois places ne garde son sous-titre, et la phrase dit `3 subtitles moved`.

**Un fichier déjà en ordre est écrit quand même**, et le dit : une destination donnée est une
destination écrite, et faire de `sort` l'exception obligerait un script à savoir quelles
sous-commandes produisent parfois un fichier et parfois rien.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Avec `--dry-run`, rien non plus : il n'y a
pas de fichier, et la ligne de narration le dit — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire).

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles moved -> <destination>`, ou `<chemin>: already in order -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), l'objet de chaque fichier porte
`counts.subtitles` (tous les sous-titres du fichier) et `counts.moved` (les places qui ont changé ;
`0` pour un fichier déjà en ordre).

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités — un fichier déjà en ordre l'est —, `2` si
aucun, `3` si certains seulement, `1` sur une erreur d'usage.

## Erreurs

Aucune qui soit propre à `sort` : un fichier illisible échoue comme pour toute sous-commande, et les
erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
