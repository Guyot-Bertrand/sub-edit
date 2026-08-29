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
  frame rate grid: none (too few subtitles to tell)
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
| `frame rate grid` | la fréquence d'image sur laquelle les positions ont été calculées, **déduite** — voir ci-dessous |
| `anomalies` | `none`, ou ce qui cloche, sous-titre par sous-titre |

**`span` n'est pas « du premier au dernier »** mais du plus tôt au plus tard :
sur un fichier dont l'ordre est rompu, les deux diffèrent, et seul le second dit
la vérité sur ce que le fichier couvre.

**`anomalies` compte les sous-titres à partir de 1**, comme le fichier les
numérote — et non les lignes du fichier. Un numéro de sous-titre survit à une
modification ; un numéro de ligne, non. Voir ci-dessous.

## Ce que `frame rate grid` rapporte

**Un fichier de sous-titres ne déclare pas sa fréquence d'image.** SubRip n'a pas
d'en-tête, celui de WebVTT est du texte libre. Cette ligne ne lit donc rien :
elle **déduit**, en mesurant si les positions tombent sur une grille d'images.

Ce qui est mesuré, ce sont les **débuts**. Partout où une grille existe ils y
sont sans exception, tandis que les fins s'en écartent souvent — une fin est
fréquemment calculée par une règle de vitesse de lecture plutôt que posée sur
une image.

La première ligne dit toujours la même chose : la fréquence retenue, le verdict,
et la concentration qui l'appuie.

| Verdict | Ce qu'il veut dire |
| :------ | :----------------- |
| `clean` | les positions sont sur une grille |
| `partial` | une partie l'est, le reste non |
| `none` | aucune candidate n'explique quoi que ce soit |

`none` a **deux causes, et ce ne sont pas la même réponse** : ou bien aucune
grille ne convient — la ligne donne alors la meilleure candidate et son score,
pour qu'on voie de combien elle échoue — ou bien le fichier compte trop peu de
sous-titres pour qu'on puisse en dire quoi que ce soit. Deux débuts paraissent
toujours parfaitement alignés ; ça ne prouve rien.

**Un fichier régulier sur une fréquence non normalisée sort `none`, et c'est
voulu.** L'ensemble des candidates est clos — les huit fréquences que la
conversion propose — et rapporter la moins fausse d'entre elles serait donner
une mauvaise réponse là où « je ne sais pas » est la bonne.

### Les lignes qui suivent, quand elles ont quelque chose à dire

| Ligne | Quand elle apparaît |
| :---- | :------------------ |
| `grid offset` | les positions sont sur la grille **à une constante près** : le fichier a été décalé, et voici de combien |
| `also fits` | une autre candidate convient tout aussi bien, parce qu'elle est un multiple entier de celle retenue |
| `too short a span to separate` | l'étendue du fichier ne permet pas de départager la fréquence retenue de celles nommées |
| `off the grid` | combien de débuts s'écartent de la grille, et en combien de **suites** |

**`also fits` n'est pas une hésitation.** Une grille à 25 images par seconde est
*incluse* dans une grille à 50 : un fichier calé sur 25 convient donc aux deux.
L'inverse est faux — un vrai 50 s'effondre sur 25 — donc c'est la plus basse qui
est retenue, et l'autre est nommée plutôt que passée sous silence. Les seules
paires concernées sont 25 et 50, 29,97 et 59,94, 30 et 60.

**`too short a span to separate` protège d'une réponse trop confiante.** 23,976
et 24 dérivent d'une milliseconde par seconde de film : sur un long métrage elles
se séparent sans appel, sur dix secondes elles sont indiscernables. La ligne dit
laquelle reste possible.

**`off the grid` distingue deux histoires que le seul pourcentage confond.**
Beaucoup de suites d'un seul début, ce sont des positions corrigées à la main,
une par une. Quelques longues suites, c'est une section recalée ou un fichier
assemblé à partir de deux autres. Le nombre de suites est ce qui les sépare.

### Un exemple

Douze répliques posées sur une grille à 25 images par seconde — donc à des
positions multiples de 40 millisecondes — étalées sur près de neuf minutes :

<!-- exemple: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

L'exemple fabrique son fichier plutôt que d'en lire un du dépôt, et c'est
délibéré : il est rejouable tel quel, et la boucle montre ce qu'est une grille
mieux qu'une phrase — des positions multiples d'une durée d'image.

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

## Erreurs

Celles d'une lecture, et elles seules : `inspect` ne prend aucune option dont la
valeur puisse être fautive, et n'écrit rien qui puisse être refusé.

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| aucun chemin donné | `files is required`, suivi d'un renvoi à `--help` |
| fichier absent | `<chemin>: does not exist` |
| fichier illisible | `<chemin>: cannot be opened: permission denied` |
| octets qui ne sont pas de l'UTF-8 | `<chemin>: is not valid UTF-8` |
| format non reconnu | `<chemin>: is in no format this tool knows` |
| rien qui ressemble à un sous-titre | `<chemin>: holds nothing recognisable as a subtitle` |

Ce sont les mêmes messages pour les sept sous-commandes : la recette d'ouverture
est écrite une fois, au noyau.

## Un exemple qui échoue

Un fichier vide ne correspond à aucun format. Il n'est pas rapporté comme vide :
il est refusé, et nommé.

<!-- exemple: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
