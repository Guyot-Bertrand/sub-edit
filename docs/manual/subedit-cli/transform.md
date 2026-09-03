# `transform`

```
subedit-cli transform --first <indice>=<temps> --last <indice>=<temps>
                      (--output FICHIER | --output-dir DOSSIER | --in-place)
                      <fichier>...
```

Corrige **toutes** les positions du fichier à partir de deux points dont on
connaît la bonne valeur. C'est l'opération qu'appelle un sous-titrage qui dérive
— juste au début, faux à la fin — là où [`shift`](shift.md), qui déplace tout de
la même durée, ne peut rien.

Le format du fichier lu est **conservé** : changer de format est le travail de
[`convert`](convert.md).

<!-- exemple: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--first` | **oui** | un repère, `<indice>=<temps>` | — |
| `--last` | **oui** | un repère, `<indice>=<temps>` | — |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois** | voir [Invocation](invocation.md#la-destination) | — |

Il n'y a pas de forme courte : ces deux options ne s'écrivent qu'en toutes
lettres.

## Écrire un repère

Un repère nomme un sous-titre et dit où son **début** doit désormais tomber :

```
<indice>=<temps>
```

**L'indice est compté à partir de 1**, comme il s'affiche et comme un fichier
SubRip le numérote. `0` est refusé, et un indice qui dépasse le dernier
sous-titre aussi.

**Le temps s'écrit comme celui de [`shift`](shift.md)** — secondes ou
horodatage, séparateur décimal le point, résolution la milliseconde :

| Forme | Exemples |
| :---- | :------- |
| secondes | `10`, `2.999` |
| horodatage | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` et `--first 3=00:00:10.000` disent la même chose.

`--first` et `--last` nomment deux points, pas un intervalle. Leurs noms disent
l'usage attendu — un repère au début du fichier, l'autre à la fin — et non une
contrainte : les inverser donne le même résultat, et **tout ce qui est en dehors
des deux repères suit comme ce qui est entre eux**.

## Ce que les autres positions deviennent

Les deux repères fixent un facteur d'échelle, et toute position le suit :

```
t′ = y₁ + (t − x₁) × r      avec r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` sont les débuts actuels des deux sous-titres nommés, `y₁`, `y₂` les
temps demandés. Le facteur est tenu comme un rationnel exact et **l'arrondi
n'a lieu qu'une fois**, à la milliseconde ; c'est ce qui fait que les deux
repères tombent **exactement** sur les temps demandés et non à un millième près
(voir l'[ADR 0013](../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Chaque sous-titre reste à l'écran **proportionnellement** à ce qu'il y restait :
la fin suit la même échelle que le début. Une transformation n'est pas un
décalage, et n'en a pas la propriété.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Le premier repère ne bouge pas, le troisième va de 9 s à 10 s : le facteur est
`9/8`, et le deuxième sous-titre — qui n'était nommé par personne — passe de
`5,001` à `5,501`, soit `1000 + 4001 × 9/8` arrondi une fois.

## Deux repères qui n'en sont pas

| Situation | Quand c'est vu | Code |
| :-------- | :------------- | :--- |
| `--first` et `--last` nomment le même indice | avant toute lecture | `1` |
| un indice dépasse le dernier sous-titre | à l'ouverture du fichier | `2` ou `3` |
| les deux sous-titres nommés commencent au même instant | à l'ouverture du fichier | `2` ou `3` |

Les trois se refusent pour la même raison : `x₂ − x₁` vaut zéro, et deux points
confondus ne définissent aucune droite. Rendre une division par zéro déguisée en
résultat serait pire que refuser.

Le premier cas est une **erreur d'usage** — les deux options sont fautives quels
que soient les fichiers, donc un seul message est dû, pas un par fichier. Les
deux autres dépendent du fichier ouvert, et se comptent donc comme un échec de
traitement.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un indice hors bornes est refusé **en nommant la borne**, ce qui dit du même
coup ce qu'on pouvait écrire :

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Une position avant l'origine est refusée

Comme pour [`shift`](shift.md), et pour la même raison : une position négative
s'écrit `-00:00:01,000`, forme qu'aucun des deux formats ne définit et qu'aucun
lecteur n'accepte. Le cas se présente ici quand un sous-titre se trouve **avant
le premier repère** et que l'échelle le repousse au-delà de zéro. Rien n'est
écrit, le code est `2`, et le message nomme le sous-titre en cause.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` — le format, l'encodage, la marque et les fins de ligne du fichier lu, remis tels quels |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

Les repères sont réécrits avec le point décimal, tels qu'ils ont pu être tapés.

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| repère sans `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| repère dont une moitié manque | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| indice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| indice qui n'est pas un entier | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| temps invalide | ceux de [`shift`](shift.md) |
| `--first` et `--last` sur le même indice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| indice hors bornes | `<chemin>: subtitle N is past the end: the file holds M subtitles` |
| deux sous-titres commençant ensemble | `<chemin>: subtitles N and M start at the same moment, so they define no transform` |
| position avant l'origine | `<chemin>: subtitle N would land before the origin, which no subtitle file can hold` |

**Rien n'est écrit sans l'une des trois options de destination**, et ses
erreurs sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
