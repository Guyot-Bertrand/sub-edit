# `italics`

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FICHIER | --output-dir DOSSIER | --in-place)
                    [--dry-run]
                    [--recursive]
                    <fichier>...
```

Met les textes **en italique**, ou **retire leur italique**, dans les balises du format du fichier.
C'est `Italic` de la [fenêtre](../subedit-gui/operations.md).

**Deux mots et non une bascule** : le bouton de la fenêtre demande au noyau dans quel sens il
joue ; ici on le dit. `--on` et `--off` s'excluent, et il en faut un — l'omettre, ou donner les deux,
est une erreur d'usage (code `1`).

- **`--on`** enveloppe chaque texte d'une seule paire de balises, **après avoir retiré celles qui y
  étaient** : un texte déjà en italique n'est pas enveloppé deux fois, et un italique coupé en deux
  est refait d'un seul tenant. Sur plusieurs lignes, une paire couvre le tout.
- **`--off`** retire les balises d'italique ; **le reste d'une balise est gardé** (`{\b1\i1}` donne
  `{\b1}`).
- Le texte est écrit **comme le format du fichier écrit l'italique** : `<i>…</i>` pour SubRip, WebVTT et
  SubViewer 2, `{\i1}…{\i0}` pour les deux Sub Station Alpha, `{Y:i}` pour MicroDVD.

<!-- exemple: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--on` / `--off` | **l'un des deux** | des drapeaux | — |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir [`adjust`](adjust.md#--range) | tout le fichier |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**Le texte principal, et lui seul.**

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un format sans style refuse

**TMPlayer et LRC n'écrivent aucun style** : il n'y a ni italique à poser ni italique à retirer. La
fenêtre grise l'entrée plutôt que de la cacher ; la ligne de commande **refuse, fichier par
fichier, avec la raison**, et les autres fichiers du lot continuent. Ce qui compte est ce que le
format peut porter, non la façon dont il l'écrit : SubRip et WebVTT écrivent l'italique de la même
manière, et ne disent pas la même chose de la couleur.

<!-- exemple: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

En JSON, l'échec porte l'identifiant **`no-style`**.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Avec `--dry-run`, la liste des
changements, décrite dans [Voir avant d'écrire](invocation.md#voir-avant-décrire).

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles put in italics -> <destination>`, ou `N subtitles taken out of italics`, ou `nothing to change` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

`N` est le nombre de sous-titres **dont le texte a changé**, lu de l'opération elle-même.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), l'objet de chaque fichier porte
`counts.changed` — le `N` de la phrase — et `changes`, la liste des sous-titres changés
(`{"subtitle", "document", "before", "after"}`, **vide** quand rien n'a changé).

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été traités — un fichier où rien ne change l'est —,
`2` si aucun, `3` si certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| ni `--on` ni `--off` | `italics needs --on to put the texts in italics, or --off` |
| les deux | `--on and --off say opposite things; give one of them` |
| un format sans style | `<chemin>: TMPlayer writes no style: there are no italics to put on` (ou `take out`) |
| plage mal écrite ou hors du fichier | voir [`adjust`](adjust.md#--range) |

Les erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
