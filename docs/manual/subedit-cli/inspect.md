# `inspect`

```
subedit-cli inspect <fichier>...
```

Rapporte ce que chaque fichier contient. **Ne modifie rien et n'écrit aucun
fichier.**

<!-- exemple: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
```

## Arguments

| Argument | Requis | Valeur |
| :------- | :----- | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins de fichiers de sous-titres |

Aucun chemin n'est une erreur d'usage, donc le code `1`.

## Sortie

Le rapport va sur la **sortie standard** — c'est le résultat de la commande, et
il est écrit à tous les niveaux, `--quiet` compris. Un bloc par fichier, précédé
du chemin :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  order: in order
```

| Champ | Ce qu'il dit |
| :---- | :----------- |
| `format` | `SubRip` ou `WebVTT` |
| `encoding` | `UTF-8` — le seul encodage lu à ce jour, les autres sont refusés |
| `byte order mark` | `present` ou `absent` |
| `line endings` | `LF`, `CRLF` ou `CR`, suivi de `, mixed from line N` si le fichier en mélange |
| `subtitles` | le nombre de sous-titres lus |
| `span` | du début le plus tôt à la fin la plus tardive, `HH:MM:SS.mmm` |
| `order` | `in order`, ou les lignes qui rompent l'ordre |

**`span` n'est pas « du premier au dernier »** mais du plus tôt au plus tard :
sur un fichier dont l'ordre est rompu, les deux diffèrent, et seul le second dit
la vérité sur ce que le fichier couvre.

**`order` nomme les lignes qui rompent l'ordre** par rapport à celle qui les
précède, comptées à partir de 1 comme elles s'affichent. Une seconde lecture —
toutes les lignes en retard sur ce qui a déjà été vu — sera proposée par un
ticket ultérieur.

## Narration

Sur la sortie d'erreur, selon le niveau demandé :

| Niveau | Ce qui s'ajoute |
| :----- | :-------------- |
| 1 | `<chemin>: N subtitles` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<chemin>: N bytes read` et `<chemin>: N diagnostic(s) while reading` |

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été lus, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Un exemple qui échoue

Un fichier vide ne correspond à aucun format. Il n'est pas rapporté comme vide :
il est refusé, et nommé.

<!-- exemple: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
