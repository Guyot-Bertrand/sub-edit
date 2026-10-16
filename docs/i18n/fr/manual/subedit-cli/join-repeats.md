# `join-repeats`

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Fait **un seul sous-titre** des sous-titres qui montrent la même chose à la suite : un texte découpé en morceaux
qui se suivent, comme le laissent certains outils de segmentation. C'est `Join Repeated Subtitles` de la fenêtre —
[le même moteur](../subedit-gui/lines.md#fusionner-les-sous-titres-répétés).

**Un répété** est un sous-titre qui a **le même texte, la même traduction et les mêmes réglages de format** (un
style, une position, des réglages de cue) qu'un autre, **et qui commence au plus tard quand celui-ci finit** : ils
se touchent ou se recouvrent. Une suite de répétés devient **celui qui commence le premier**, dont la fin est
reportée à la plus grande fin de la suite ; les autres disparaissent.

- **L'ordre du fichier n'y change rien, et n'en change pas** : le sous-titre gardé est celui qui commence le premier,
  où que les autres se trouvent. Un fichier dans l'ordre le reste.
- **Rien n'est joint par erreur** : un autre texte, une autre traduction, d'autres réglages, un écart entre deux
  morceaux, un sous-titre sans texte ou dont la fin précède le début — dans tous ces cas ils restent séparés.
- **À faire avant le reste** : le tri et le [découpage des chevauchements](disjoint.md) lisent mieux une suite propre.
  [`repair`](repair.md) enchaîne les trois, et [`convert`](convert.md#rendre-les-plages-disjointes) l'accepte en option (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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
| `<file>...` | oui | un ou plusieurs chemins | — |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Les trois morceaux d'`Alpha`, qui se suivent de 1 s à 6 s, n'en font plus qu'un. `Beta`, séparé par un écart, est
resté. Un second passage ne change rien et le dit : `no repeated subtitles`.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, ou `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, puis **chaque diagnostic de lecture** |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), les `counts` de chaque fichier : `subtitles` (ceux du fichier
écrit), `joined` (les sous-titres retirés parce qu'un autre les couvre) et `chains` (les suites jointes).

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités — un fichier sans répété l'est —, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

Aucune qui soit propre à `join-repeats` : un fichier illisible échoue comme pour toute sous-commande, et les erreurs de
la destination sont communes aux sous-commandes qui écrivent : voir [Invocation](invocation.md#la-destination).
