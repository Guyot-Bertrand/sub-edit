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

| Argument | Requis | Valeur | Défaut |
| :------- | :----- | :----- | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins de fichiers de sous-titres | — |

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
  anomalies: none
```

| Champ | Ce qu'il dit |
| :---- | :----------- |
| `format` | `SubRip` ou `WebVTT` |
| `encoding` | `UTF-8` — le seul encodage lu à ce jour, les autres sont refusés |
| `byte order mark` | `present` ou `absent` |
| `line endings` | `LF`, `CRLF` ou `CR`, suivi de `, mixed from line N` si le fichier en mélange |
| `subtitles` | le nombre de sous-titres lus |
| `span` | du début le plus tôt à la fin la plus tardive, `HH:MM:SS.mmm` |
| `anomalies` | `none`, ou ce qui cloche, sous-titre par sous-titre |

**`span` n'est pas « du premier au dernier »** mais du plus tôt au plus tard :
sur un fichier dont l'ordre est rompu, les deux diffèrent, et seul le second dit
la vérité sur ce que le fichier couvre.

**`anomalies` compte les sous-titres à partir de 1**, comme le fichier les
numérote — et non les lignes du fichier. Un numéro de sous-titre survit à une
modification ; un numéro de ligne, non. Voir ci-dessous.

## Ce que `anomalies` rapporte

Trois choses peuvent clocher dans un document, et chacune se répare autrement :

| Ce qui est écrit | Ce que ça veut dire |
| :--------------- | :------------------ |
| `subtitle N ends before it starts` | la fin précède le début |
| `subtitle N starts before the previous one ends` | il chevauche celui d'avant |
| `subtitle N starts before the previous one starts` | il rompt l'ordre du fichier |

**Un même sous-titre peut apparaître deux fois**, et c'est voulu : celui qui
commence avant que le précédent ait commencé commence aussi avant qu'il ait
fini. Le premier point se règle en déplaçant le sous-titre, le second en
changeant une durée.

**Le sous-titre nommé est celui qui est mal placé**, pas celui contre lequel il
l'est. Sur des débuts à `0`, `4 s`, `2 s`, `3 s` : le troisième rompt l'ordre, le
quatrième suit pourtant le troisième et n'est donc pas nommé — il n'y a rien à
faire de lui.

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -1 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -1
  anomalies: subtitle 3 starts before the previous one ends, subtitle 3 starts before the previous one starts
```

**Ce n'est pas un diagnostic de lecture.** Les diagnostics disent ce que la
lecture a rencontré et pointent une **ligne du fichier** ; les anomalies disent
ce que le document *est* et pointent un **sous-titre**. Un fichier dont l'ordre
est rompu n'est pas malformé pour autant : la lecture n'a rien à en dire.

## Narration

Sur la sortie d'erreur, selon le niveau demandé :

| Niveau | Ce qui s'ajoute |
| :----- | :-------------- |
| 1 | `<chemin>: N subtitles` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<chemin>: N bytes read`, `<chemin>: N diagnostic(s) while reading`, puis **chacun d'eux** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

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
