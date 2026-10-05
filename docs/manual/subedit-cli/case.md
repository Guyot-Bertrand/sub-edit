# `case`

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FICHIER | --output-dir DOSSIER | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FICHIER] [--align-method position|number]
                 <fichier>...
```

Met les textes en **casse de titre, de phrase, en capitales ou en minuscules**, sans toucher une
balise. C'est `Text ▸ Case` de la [fenêtre](../subedit-gui/operations.md), avec ses règles.

| `--to` | Effet | `bonjour marie` devient |
| :----- | :---- | :---------------------- |
| `title` | chaque mot prend une capitale, le reste redescend | `Bonjour Marie` |
| `sentence` | la première lettre du texte, et elle seule ; le reste redescend | `Bonjour marie` |
| `upper` | tout en capitales | `BONJOUR MARIE` |
| `lower` | tout en minuscules | `bonjour marie` |

- **Ce qui précède la première lettre ou le premier chiffre n'est pas touché** : un tiret de
  dialogue, un guillemet ouvrant ou une parenthèse survivent (`- bonjour` donne `- Bonjour`).
- **Les balises ne sont pas du texte** : elles ne changent pas de casse — `<I>` reste `<I>` — et ne
  changent pas de place : une balise qui coupe un mot le coupe encore après.
- **ICU décide où commence un mot** : `l'été sera chaud` donne `L'été Sera Chaud`, et les accents
  tiennent (`éléonore` donne `Éléonore`).

<!-- exemple: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **oui** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | non | `N-M`, ou `N-` jusqu'à la fin — voir [`adjust`](adjust.md#--range) | tout le fichier |
| `--document` | non | `main` ou `translation` — voir [Une traduction](invocation.md#une-traduction) | `main` |
| `-t`, `--translation-file` | avec `--document translation` | un fichier de traduction, pour **une seule** entrée | — |
| `--align-method` | non, et seulement avec `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

**Le texte principal par défaut ; la traduction avec `--document translation -t FICHIER`.** C'est alors
la traduction qui est changée et écrite, et elle seule — voir [Une traduction](invocation.md#une-traduction).

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un fichier dont **aucun texte ne change** — tout est déjà dans la casse demandée — est écrit tel quel et
le dit :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit. Avec `--dry-run`, la liste des
changements, décrite dans [Voir avant d'écrire](invocation.md#voir-avant-décrire).

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles recased -> <destination>`, ou `nothing to change` |
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
| `--to` absent | `--to is required` |
| une valeur qui n'est pas l'une des quatre | `--to: … not in {title,sentence,upper,lower}` |
| plage mal écrite ou hors du fichier | voir [`adjust`](adjust.md#--range) |

Les erreurs de la destination sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
