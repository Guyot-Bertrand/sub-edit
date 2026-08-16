# `hearing-impaired`

```
subedit-cli hearing-impaired
                  (--output FICHIER | --output-dir DOSSIER | --in-place)
                  <fichier>...
```

Retire les **mentions pour malentendants** — les bruits décrits entre crochets
ou entre parenthèses — de tout le fichier.

Ce qu'elle ne fait pas : les paroles de chanson entre dièses, le nom du locuteur
avant deux-points, la remise en majuscule, la correction d'erreurs d'OCR. Ces
motifs-là viennent avec le moteur de correction complet, et rien ici ne se
règle : la transformation est décidée, pas configurable.

<!-- exemple: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between brackets or parentheses
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois** | voir [Invocation](invocation.md#la-destination) | — |

Le format du fichier lu est **conservé** : changer de format est le travail de
[`convert`](convert.md).

**Le texte principal, et lui seul.** Un document de traduction n'est pas touché,
la ligne de commande n'ayant à ce jour aucun moyen d'en désigner un.

## Ce qui est retiré, et ce qui reste

Une mention est un crochet ou une parenthèse, avec ce qu'elle enferme, **saut de
ligne compris** : dans de vrais fichiers, une mention est souvent coupée par la
fin de ligne.

| Entrée | Sortie |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Le retrait laisse **exactement une espace entre ce qui l'entourait, et rien en
bord de ligne**. Ce n'est pas un nettoyage des espaces du fichier : deux espaces
que le texte portait déjà restent deux espaces.

**Quand la mention enjambe le saut de ligne, c'est le saut qui subsiste** — les
deux lignes restent deux lignes. Ce cas n'est pas une curiosité : sur quinze
sous-titrages réels, tous les crochets qui semblaient orphelins étaient des
mentions coupées ainsi.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Ce qui n'est pas une mention et ne bouge pas :**

| Cas | Ce qui se passe |
| :-- | :-------------- |
| contenu purement numérique — `[1]`, `(12)`, `[ 156478 ]` | gardé tel quel, délimiteurs compris : une référence n'a jamais désigné un bruit |
| crochet ou parenthèse que rien ne referme | laissé tel quel |
| une balise de format — `<i>`, `</i>` | jamais retirée |

Un blanc **entre** les chiffres, une lettre ou un signe — `[1 2]`, `[1a]`,
`[-1]` — en font une mention. Un contenu vide, `[]` ou `( )`, aussi : aucun
chiffre, donc pas une référence ; rien à dire, donc rien à garder.

## Ce qui disparaît entièrement

**Une ligne que le retrait a vidée disparaît**, et **un sous-titre entièrement
vidé est retiré du fichier**. Les sous-titres qui suivent sont renumérotés.

Une ligne compte pour vide quand il n'y reste que des blancs, des balises de
format, ou un tiret de dialogue seul — `<i>[Musique]</i>` ne laisse pas un
`<i></i>` à l'écran.

**Un dialogue réduit à une seule voix n'en est plus un**, et le tiret qui reste
s'en va. Tant qu'il reste deux voix, les tirets restent.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un fichier sans aucune mention est écrit quand même

Il est recopié à l'identique, le code de retour est `0`, et le rapport dit qu'il
n'y avait rien à retirer.

C'est la règle des sous-commandes qui écrivent : **une destination donnée est
une destination écrite**. En faire l'exception ici obligerait un script à savoir
laquelle des sous-commandes produit parfois un fichier et parfois rien.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles cleaned, M removed -> <destination>`, ou `<chemin>: no mention to remove -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

`N` compte les sous-titres dont le texte a changé, `M` ceux que le retrait a
entièrement vidés et qui ont donc quitté le fichier.

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

Cette sous-commande n'a pas d'erreur qui lui soit propre : elle ne prend aucune
valeur à analyser, et aucun texte ne peut lui faire refuser un fichier. Ce qui
peut échouer est la lecture ou l'écriture, comme pour les autres.

**Rien n'est écrit sans l'une des trois options de destination**, et ses erreurs
sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
