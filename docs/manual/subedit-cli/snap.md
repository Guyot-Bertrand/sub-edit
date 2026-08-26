# `snap`

```
subedit-cli snap --rate <cadence>
                 (--output FICHIER | --output-dir DOSSIER | --in-place)
                 <fichier>...
```

Porte **chaque horodatage** — début et fin — sur l'image la plus proche de la
cadence donnée. Rien n'est re-miné : chaque position bouge d'une demi-image au
plus.

## `snap` n'est pas `framerate`, et s'en tromper ne se voit pas

Les deux commandes prennent les mêmes arguments et font des choses opposées.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| ce qu'elle calcule | `t' = t x cadence d'entrée / cadence de sortie` | l'image la plus proche, sur la grille de `--rate` |
| de combien elle déplace | proportionnellement au temps — **plusieurs secondes** sur un long métrage | une demi-image au plus, **partout** |
| pour quel fichier | un fichier dont le **minutage est faux** | un fichier dont le minutage est **juste** et dont la grille est fausse |

**Le cas d'usage de `snap`**, en toutes lettres : un film à 25 images par
seconde, un fichier de sous-titres écrit sur une grille à 24, des répliques déjà
à peu près à leur place à quelques millisecondes près. Il n'y a rien à re-miner
— seulement à reposer les horodatages sur les bonnes images.

**Le cas d'usage de `framerate`** est l'autre : un fichier calé pour un master à
25 qu'on veut jouer sur une copie à 23,976, et dont le minutage dérive de
plusieurs secondes en fin de film.

Se tromper de l'une pour l'autre **ne provoque aucune erreur** : les deux
acceptent le fichier et écrivent un résultat. La ligne de narration est ce qui
les distingue — `snap` dit toujours de combien il a bougé les positions.

<!-- exemple: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--rate` | **oui** | une cadence, dans les six formes de [`framerate`](framerate.md#les-formes-acceptées) | — |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois** | voir [Invocation](invocation.md#la-destination) | — |

`--rate` accepte **n'importe quelle cadence valide**, et pas seulement les huit
normalisées : c'est le choix de l'utilisateur, comme pour `framerate`. Une
cadence nulle ou négative est refusée.

Le format du fichier lu est **conservé** : changer de format est le travail de
[`convert`](convert.md).

## Ce qu'elle écrit

Sur la sortie d'erreur, à partir du niveau 1 :

```
a.srt: 176 subtitles aligned on 25 fps, 339 positions moved, by at most 20 ms
```

**Les deux nombres sont l'intérêt de la ligne.** Une demi-image à 25 images par
seconde vaut vingt millisecondes, et rien de plus ne peut être écrit là. Qui
voulait `framerate` et a tapé `snap` lit « by at most 20 ms » là où il attendait
des secondes, et le sait aussitôt.

## Un exemple

Une réplique à `00:00:01,010`, alignée sur 25 images par seconde — donc sur un
multiple de quarante millisecondes :

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Deux propriétés

**Aligner deux fois ne change rien la seconde fois.** Une position déjà sur une
image y reste : c'est la même opération appliquée au même résultat.

**L'ordre des sous-titres est préservé.** L'arrondi à l'image la plus proche est
monotone, donc deux débuts distants d'au moins une image le restent. Plus
proches, ils peuvent se confondre — ce qui est une anomalie que
[`inspect`](inspect.md#ce-que-anomalies-rapporte) sait nommer, et non une
inversion.

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.
