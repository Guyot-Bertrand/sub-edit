# `repair`

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Fait, en une fois, ce que font trois commandes l'une après l'autre : [`join-repeats`](join-repeats.md), puis
[`sort`](sort.md), puis [`disjoint`](disjoint.md). **Dans cet ordre, qui est le seul qui marche** : le découpage lit
l'ordre des débuts, et les morceaux répétés d'un même texte sont mieux partis avant l'un comme l'autre. Sur un fichier
mal segmenté, mal ordonné et qui se chevauche, c'est la commande à lancer.

Les réglages sont ceux de `disjoint`, qui termine la chaîne ; `--sort` n'existe pas ici, puisque le tri est toujours fait.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<file>...` | oui | un ou plusieurs chemins | — |
| `--minimum` | non | une durée : `0.08`, `1.5`, `00:00:00,080`. Zéro garde tout intervalle | `0.08` (environ deux images) |
| `--max-lines` | non | un entier ; `0` lève la limite | `2` |
| `--dashes` | non | un drapeau : un tiret devant le texte que chaque sous-titre apporte à un sous-titre fusionné | désactivé |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, coupé en deux morceaux qui se suivent, redevient un seul sous-titre ; il occupe alors la même plage que `Beta`,
et les deux n'en font qu'un à deux lignes, le plus tôt d'abord.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Sur la sortie d'erreur, au premier niveau, **une phrase
qui enchaîne les trois comptes**, séparés par des points-virgules : ce que `join-repeats`, `sort` et `disjoint` disent
chacun, dans leurs mots — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), les `counts` de chaque fichier : `subtitles`, `joined`,
`moved`, puis ceux de [`disjoint`](disjoint.md#en-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Codes de retour et erreurs

Ceux de l'outil. Aucune erreur n'est propre à `repair` hors des réglages, qui sont jugés comme ceux de `disjoint` :
`--minimum` qui n'est pas une durée ou est négatif, `--max-lines` qui n'est pas un entier, code `1`.
