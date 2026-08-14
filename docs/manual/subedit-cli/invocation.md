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
subedit 0.2.12
```

## Sous-commandes

| Sous-commande | Ce qu'elle fait |
| :------------ | :-------------- |
| [`inspect`](inspect.md) | rapporte ce qu'un fichier contient, sans rien modifier |
| [`convert`](convert.md) | écrit un fichier dans un autre format, ou une autre forme |
| [`shift`](shift.md) | décale toutes les positions d'une même durée |

La transformation et la conversion de fréquence d'image relèvent des tickets
suivants de la phase 3 ; elles n'existent pas encore et l'aide ne les annonce
pas.

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
| 3 | `-vvv` | et la trace de mise au point : taille lue, nombre de diagnostics |

**Les erreurs ne sont jamais tues, `--quiet` compris.** Une commande qui échoue
en silence ne laisserait que son code de retour.

Le bilan n'apparaît qu'à partir de deux fichiers : sur une entrée unique, il
répéterait la ligne qui le précède.

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
