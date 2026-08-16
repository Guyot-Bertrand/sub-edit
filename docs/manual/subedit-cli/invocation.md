# Invocation

```
subedit-cli [options globales] <sous-commande> [options] <fichier>...
```

Sans sous-commande, l'outil écrit son aide et s'arrête.

<!-- exemple: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  hearing-impaired            Remove the sounds described between brackets or parentheses
```

## La ligne de commande est en anglais

Noms de sous-commandes, noms d'options, aide et messages d'erreur sont en
anglais. Ce manuel reste en français et cite la sortie telle qu'elle est
produite — ces blocs sont engendrés par `make manual`, jamais recopiés.

## Options globales

Elles s'écrivent **avant ou après** le nom de la sous-commande, indifféremment.

| Option | Effet |
| :----- | :---- |
| `-h`, `--help` | écrit l'aide et s'arrête ; celle de la sous-commande si l'on en nomme une |
| `--version` | écrit `subedit <version>` et s'arrête |
| `-q`, `--quiet` | niveau 0 — plus aucune narration |
| `-v`, `-vv`, `-vvv` | niveaux 1 à 3 ; le niveau 1 est celui par défaut |

`--quiet` et `-v` dans la même invocation sont refusés : deux intentions
opposées ne sont pas arbitrées au profit de la dernière écrite.

<!-- exemple: subedit-cli --version -->
```console
$ subedit-cli --version
subedit 0.4.0
```

## Sous-commandes

| Sous-commande | Ce qu'elle fait |
| :------------ | :-------------- |
| [`inspect`](inspect.md) | rapporte ce qu'un fichier contient, sans rien modifier |
| [`convert`](convert.md) | écrit un fichier dans un autre format, ou une autre forme |
| [`shift`](shift.md) | décale toutes les positions d'une même durée |
| [`transform`](transform.md) | corrige toutes les positions à partir de deux repères |
| [`framerate`](framerate.md) | recale un fichier d'une cadence d'images vers une autre |

Les cinq sont là ; l'aide de l'outil les énumère dans le même ordre.

## La destination

Les quatre sous-commandes qui écrivent — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md) —
prennent leur destination de la même façon. [`inspect`](inspect.md) n'écrit
aucun fichier et n'accepte aucune de ces options.

**Rien n'est jamais écrit sans destination explicite.** Les trois façons de la
donner s'excluent, et il en faut une : sans elle, rien n'est écrit et le code de
retour est `1`.

| Option | Où va le résultat |
| :----- | :---------------- |
| `--output FICHIER` | dans ce fichier, sous ce nom exact ; une seule entrée |
| `--output-dir DOSSIER` | dans ce dossier, sous le nom de base de l'entrée |
| `--in-place` | par-dessus l'entrée, par écriture atomique |

Un outil qui écrase son entrée parce qu'on ne lui a rien dit est un outil qu'on
cesse d'utiliser à la deuxième fois. Le refus coûte une ligne et sauve le
fichier.

`--output` avec plusieurs entrées est refusé : le dernier fichier s'écrirait sur
les précédents. Avec un lot, `--output-dir` est le seul des trois qui ait un
sens, avec `--in-place`.

**L'extension suit le format écrit.** Elle ne change que pour
[`convert`](convert.md), seule sous-commande qui change de format ; les trois
autres conservent celui du fichier lu, donc son extension.

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| aucune destination | `no destination given: use --output, --output-dir or --in-place` |
| deux destinations | `--output, --output-dir and --in-place exclude one another` |
| `--output` sur un lot | `--output names one file but several were given: use --output-dir instead` |
| destination non inscriptible | `<chemin>: <destination>: cannot be opened: permission denied` |

## Deux sorties, deux rôles

| Sortie | Ce qu'elle porte |
| :----- | :--------------- |
| standard | **le résultat, et lui seul** — le rapport d'`inspect` |
| erreur | **tout le reste** — la narration, les avertissements, les erreurs |

C'est ce partage qui permet de rediriger le résultat sans y récupérer le récit :

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Le rapport part dans le fichier, la narration reste à l'écran.

## Niveaux de narration

Chaque niveau **contient le précédent** : monter d'un cran ajoute, ne remplace
jamais.

| Niveau | Comment | Ce que la sortie d'erreur porte |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | rien, **sauf les erreurs** |
| 1 | par défaut, ou `-v` | une ligne par fichier traité, et un bilan dès qu'il y en a plusieurs |
| 2 | `-vv` | et ce qui a été reconnu : format, encodage, BOM, fins de ligne |
| 3 | `-vvv` | et la trace de mise au point : octets lus et écrits, et **chaque diagnostic de lecture** |

**Les erreurs ne sont jamais tues, `--quiet` compris.** Une commande qui échoue
en silence ne laisserait que son code de retour.

Le bilan n'apparaît qu'à partir de deux fichiers : sur une entrée unique, il
répéterait la ligne qui le précède.

## Les diagnostics de lecture

Les formats de sous-titres se lisent **au mieux** : devant une anomalie, le
lecteur ne s'arrête pas — il décide, ou il laisse en l'état, et il le dit. Ce
qu'il a rencontré sort au **niveau 3**, une ligne par anomalie, sur la sortie
d'erreur :

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a subtitle that ends before it starts, left as it stands
```

Les cinq sous-commandes les rapportent, pas seulement [`inspect`](inspect.md) :
un fichier lu au mieux puis réécrit a subi les mêmes décisions, et les taire
laisserait croire que rien ne s'est passé.

**Un diagnostic n'est jamais un échec.** Le fichier a été lu, la commande a
abouti, le code de retour est `0`. C'est pourquoi ils vivent au niveau le plus
détaillé : plus bas, ils enterreraient la ligne qui dit ce qui a réellement été
fait.

### Ce que chaque ligne porte

| Partie | Ce qu'elle dit |
| :----- | :------------- |
| `line N` | où, compté à partir de 1, comme un éditeur l'affiche |
| la phrase | ce qui a été rencontré, parmi les dix catégories ci-dessous |
| `("…")` | le texte fautif du fichier, quand la catégorie ne suffit pas ; tronqué à 80 octets |
| la fin | ce qui en a été fait : `settled by the reader`, ou `left as it stands` |

**La fin de la ligne est le plus important.** `settled by the reader` veut dire
que le lecteur a tranché et que le fichier écrit porte sa décision — une
numérotation absente est régénérée. `left as it stands` veut dire qu'il n'a rien
touché parce que **vous seul pouvez décider** : un sous-titre qui finit avant de
commencer reste tel quel.

### Le numéro de ligne d'un bloc

Une anomalie qui porte sur **un bloc entier** est ancrée sur sa ligne
d'horodatage, et non sur la ligne exacte qui la déclenche. Une numérotation
incohérente écrite ligne 5 se rapporte donc ligne 6, celle de l'horodatage qui
suit — le numéro fautif est dans le `("…")`. Seules les anomalies qui portent sur
**une ligne** — un horodatage illisible, du texte avant le premier — se
rapportent sur elles-mêmes.

### Les dix catégories

| Phrase | Ce qui la déclenche |
| :----- | :------------------ |
| `a line that fits nowhere` | une ligne qui n'entre dans aucun bloc |
| `a timing line that could not be read` | une ligne d'horodatage illisible |
| `a subtitle that ends before it starts` | une fin antérieure au début |
| `a subtitle starting before the previous one ends` | un chevauchement |
| `a subtitle starting before the previous one starts` | un désordre |
| `a SubRip block without its number` | numérotation absente, régénérée à l'écriture |
| `SubRip numbers that do not follow` | numérotation qui saute |
| `text before the first timing line` | du texte avant le premier horodatage |
| `a WebVTT block of an unknown kind` | un bloc WebVTT non reconnu |
| `more than one kind of line ending` | des fins de ligne mélangées |

## Plusieurs fichiers

Toutes les sous-commandes acceptent plusieurs chemins. Chacun est traité
indépendamment : l'échec de l'un n'interrompt pas les autres, et les échecs
sont rapportés en nommant le fichier et la raison.

## Codes de retour

| Code | Signification |
| :--- | :------------ |
| `0` | tout a réussi |
| `1` | erreur d'usage — option inconnue, valeur invalide, combinaison interdite |
| `2` | aucun fichier n'a pu être traité |
| `3` | certains fichiers ont été traités, d'autres non |

`2` et `3` sont distingués pour qu'un script sache agir sans relire la sortie :
« rien n'a marché » et « il en manque un » n'appellent pas la même réaction.

Une erreur d'usage est détectée **avant tout traitement** : elle ne laisse
jamais un lot à moitié traité.

## Erreurs

| Ce qui la déclenche | Ce qui est écrit, sur la sortie d'erreur |
| :------------------ | :--------------------------------------- |
| option ou sous-commande inconnue | le nom fautif, et un renvoi à `--help` |
| `--quiet` avec `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| fichier absent | `<chemin>: does not exist` |
| fichier illisible | `<chemin>: cannot be opened: permission denied` |
| octets qui ne sont pas de l'UTF-8 | `<chemin>: is not valid UTF-8` |
| format non reconnu | `<chemin>: is in no format this tool knows` |
| rien qui ressemble à un sous-titre | `<chemin>: holds nothing recognisable as a subtitle` |
