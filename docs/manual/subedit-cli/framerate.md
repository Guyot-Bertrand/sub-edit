# `framerate`

> **Ce n'est pas [`snap`](snap.md).** Cette commande **met le fichier à
> l'échelle** : elle sert quand le minutage est faux et dérive de plusieurs
> secondes en fin de film. `snap` repose les horodatages sur la grille la plus
> proche sans re-miner quoi que ce soit, et sert quand le minutage est déjà
> juste. Les deux prennent les mêmes arguments, et se tromper ne provoque
> aucune erreur.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FICHIER | --output-dir DOSSIER | --in-place)
                      <fichier>...
```

Recale un fichier minuté pour une cadence d'images vers une autre. C'est
l'opération qu'appelle un fichier de sous-titres récupéré pour une copie qui
n'est pas celle du film qu'on a — un DVD PAL contre un transfert NTSC, par
exemple.

Le format du fichier lu est **conservé** : changer de format est le travail de
[`convert`](convert.md).

<!-- exemple: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<fichier>...` | oui | un ou plusieurs chemins | — |
| `--from` | **oui** | une cadence, voir ci-dessous | — |
| `--to` | **oui** | une cadence, voir ci-dessous | — |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois** | voir [Invocation](invocation.md#la-destination) | — |

`--from` est la cadence pour laquelle le fichier **est** minuté, `--to` celle
pour laquelle on veut qu'il le soit. Les intervertir décale le fichier dans
l'autre sens — de l'ordre de sept secondes sur un long métrage entre 25 et
23.976 — et rien ne le signale avant qu'on lise le film.

## Écrire une cadence

En images par seconde, décimales comprises, le point pour séparateur :

```
25      24      23.976      29.97      60
```

**Une cadence nulle ou négative est refusée** : ce n'est pas une cadence.

### Les trois cadences NTSC ne s'écrivent pas exactement

`23.976`, `29.97` et `59.94` sont des **étiquettes**. Les cadences qu'elles
désignent sont `24000/1001`, `30000/1001` et `60000/1001`, dont aucune n'a
d'écriture décimale finie. L'outil les reconnaît comme telles :

| Ce qui s'écrit | Ce que cela désigne |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinéma transféré en vidéo NTSC |
| `24` | `24` — cinéma |
| `25` | `25` — vidéo PAL et SECAM |
| `29.97` | `30000/1001` — vidéo NTSC |
| `30`, `50`, `60` | eux-mêmes |
| `59.94` | `60000/1001` |

La reconnaissance porte sur **la valeur et non sur les lettres** : `23.9760`
désigne la même cadence que `23.976`.

Toute autre décimale est prise au pied de la lettre — `23.9` n'est le standard
de personne, il n'y a rien à y lire. C'est aussi ce qui rend la table ci-dessus
nécessaire : sans elle, `23.976` vaudrait `23976/1000`, et la ligne de commande
dirait autre chose que la fenêtre à venir pour les mêmes mots.

## Ce que les positions deviennent

```
t′ = arrondi( t × (from / to) )
```

Un film masterisé à 23.976 et joué à 25 images par seconde défile plus vite,
donc ses sous-titres arrivent plus tôt ; dans l'autre sens ils arrivent plus
tard.

**Le facteur est un rationnel exact, et l'arrondi n'a lieu qu'une fois.** Le
chemin qui paraît équivalent — convertir la position en numéro d'image à la
cadence d'entrée, puis revenir en millisecondes à celle de sortie — ne l'est
pas : il quantifie chaque position sur la grille d'entrée et se trompe jusqu'à
une demi-image, soit 21 ms mesurées. Voir
l'[ADR 0013](../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Concrètement, de 25 vers 23.976, le facteur vaut `1001/960` :

| Position lue | Écrite | Ce que donnerait le passage par les images |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<chemin>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<chemin>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<chemin>: N bytes read, M written`, puis **chaque diagnostic de lecture** — voir [Invocation](invocation.md#les-diagnostics-de-lecture) |

**La narration nomme la cadence exacte, pas l'étiquette tapée.** Écrire
`23.976` sur cette ligne rapporterait une conversion qui n'a pas eu lieu ; c'est
le seul endroit où l'on voit laquelle des deux a servi. Une cadence qu'une
décimale écrit exactement s'y affiche en décimale, les autres en fraction.

## Codes de retour

Ceux de l'outil : `0` si tous les fichiers ont été écrits, `2` si aucun, `3` si
certains seulement, `1` sur une erreur d'usage.

Aucun refus ne dépend du contenu d'un fichier : deux cadences définissent
toujours un facteur. C'est la seule des sous-commandes qui écrivent dont
tous les refus tombent avant la première lecture.

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Erreurs

| Ce qui la déclenche | Message |
| :------------------ | :------ |
| cadence nulle ou négative | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| virgule décimale | `"…" is not a frame rate: use a decimal point and not a comma, the command line being English` |
| texte qui n'est pas un nombre | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| cadence démesurée | `"…" is not a frame rate: no video runs at that many frames per second` |

**Rien n'est écrit sans l'une des trois options de destination**, et ses
erreurs sont communes aux sous-commandes qui écrivent :
voir [Invocation](invocation.md#la-destination).
