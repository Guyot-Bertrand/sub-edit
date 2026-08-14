# `inspect`

```
subedit-cli inspect [--order-report breaks|late] <fichier>...
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
  --order-report TEXT:{breaks,late} [breaks] 
                              Which lines to name when the file is out of order
```

## Arguments

| Argument | Requis | Valeur | Défaut |
| :------- | :----- | :----- | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins de fichiers de sous-titres | — |
| `--order-report` | non | `breaks` ou `late`, et rien d'autre | `breaks` |

Aucun chemin n'est une erreur d'usage, donc le code `1`. Une valeur de
`--order-report` hors de ces deux-là aussi, et le message énumère l'ensemble
attendu.

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

**`order` compte les lignes à partir de 1**, comme elles s'affichent, et sa
formulation dépend de la lecture demandée — voir ci-dessous.

## Deux lectures du désordre

`--order-report` choisit **ce que « hors d'ordre » veut dire**. Les deux
s'accordent toujours sur le fait qu'un fichier est en désordre ou non ; elles
diffèrent sur les lignes à nommer.

| Valeur | Ce qu'elle nomme | Ce que le rapport écrit |
| :----- | :--------------- | :---------------------- |
| `breaks` (défaut) | les lignes qui commencent avant celle qui les précède | `line 3 breaks the order` |
| `late` | les lignes qui commencent avant quelque chose de déjà vu | `lines 3, 4 start late` |

Sur des débuts à `0`, `4 s`, `2 s`, `3 s` : la troisième ligne rompt l'ordre, la
quatrième suit pourtant la troisième — elle ne rompt rien — mais reste en retard
sur les `4 s` déjà rencontrées. `breaks` nomme la troisième, `late` nomme la
troisième et la quatrième.

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -1; subedit-cli --quiet inspect --order-report late desordre.srt | tail -1 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -1; subedit-cli --quiet inspect --order-report late desordre.srt | tail -1
  order: line 3 breaks the order
  order: lines 3, 4 start late
```

**La formulation change avec la lecture**, et c'est volontaire : une liste
d'indices seule serait ambiguë entre les deux, alors qu'elles ne désignent pas
les mêmes lignes.

Aucune des deux n'est encore retenue comme *la* bonne. Les deux existent pour
être comparées sur des fichiers réels ; l'interface graphique tranchera, et
l'option disparaîtra alors au profit de la lecture retenue.

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
