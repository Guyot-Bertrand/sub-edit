# `dialogue-dashes`

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FICHIER | --output-dir DOSSIER | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FICHIER] [--align-method position|number]
                            <fichier>...
```

Met un **tiret de dialogue** en tête des lignes, ou les retire. C'est `Dialogue Dashes` de la
[fenêtre](../subedit-gui/operations.md).

**Deux mots, comme [`italics`](italics.md)** : `--add` et `--remove` s'excluent, et il en faut un.

- **`--add`** met un tiret devant chaque ligne qui n'en a pas : `Bonjour` donne `- Bonjour`, et
  `Bonjour\nMarie` donne `- Bonjour\n- Marie`. **Un tiret déjà là n'est pas doublé.**
- **Trois tirets sont reconnus** — le trait d'union du clavier et les deux cadratins du typographe —
  **et c'est le trait d'union qui est écrit** : `— Bonjour` devient `- Bonjour`. Les espaces de trop
  après le tiret disparaissent.
- Le tiret se pose **devant le texte, pas devant une balise ouvrante** : `<i>Bonjour</i>` donne
  `<i>- Bonjour</i>`. Aucune balise ne bouge.
- **`--remove`** retire les tirets de tête, des trois formes.

<!-- exemple: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
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
| `--add` / `--remove` | **l'un des deux** | des drapeaux | — |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir [`adjust`](adjust.md#--range) | tout le fichier |
| `--document` | non | `main` ou `translation` — voir [Une traduction](invocation.md#une-traduction) | `main` |
| `-t`, `--translation-file` | avec `--document translation` | un fichier de traduction, pour **une seule** entrée | — |
| `--align-method` | non, et seulement avec `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**Le texte principal par défaut ; la traduction avec `--document translation -t FICHIER`.** C'est alors
la traduction qui est changée et écrite, et elle seule — voir [Une traduction](invocation.md#une-traduction).

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Avec `--dry-run`, la liste des
changements, décrite dans [Voir avant d'écrire](invocation.md#voir-avant-décrire).

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles dashed -> <destination>`, ou `N subtitles undashed`, ou `nothing to change` |
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
| ni `--add` ni `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| les deux | `--add and --remove say opposite things; give one of them` |
| plage mal écrite ou hors du fichier | voir [`adjust`](adjust.md#--range) |

Les erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
