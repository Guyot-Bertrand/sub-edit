# Fixtures sur grille connue

Treize fichiers SubRip engendrés, dont on ne lit ni le texte ni le sens : ce
qu'on y lit, c'est **si les positions tombent sur une grille d'images, et
laquelle**. C'est la donnée dont la phase 16 a besoin, et qu'un fichier de
sous-titres ne déclare jamais.

**Ne pas les éditer à la main, et ne pas les croire sur parole.**
[`src/scripts/subtitle-fixtures.py`](../../../scripts/subtitle-fixtures.py)
porte la table de ce que chaque fixture est, et reconstruit chaque fichier pour
le confronter à celui du disque :

```console
$ ./src/scripts/subtitle-fixtures.py --check      # le disque, contre la table
$ ./src/scripts/subtitle-fixtures.py --generate   # les refabriquer
$ ./src/scripts/subtitle-fixtures.py --measure    # ce qu'elles donnent
```

`--check` tourne dans `make check-local`. C'est là qu'est la garantie : un
fichier de cent soixante-seize répliques est illisible dans un diff, et personne
ne relira ces 128 Ko.

## Ce que chaque fixture est

| Fixture | Fréquence | Étendue | Décalage | Répliques | Poids |
| :------ | :-------- | ------: | -------: | --------: | ----: |
| `grille-23-976.srt` | `24000/1001` | 10 min | — | 176 | 10 734 o |
| `grille-24.srt` | `24/1` | 10 min | — | 176 | 10 734 o |
| `grille-25.srt` | `25/1` | 10 min | — | 170 | 10 367 o |
| `grille-29-97.srt` | `30000/1001` | 10 min | — | 174 | 10 636 o |
| `grille-30.srt` | `30/1` | 10 min | — | 176 | 10 763 o |
| `grille-50.srt` | `50/1` | 10 min | — | 171 | 10 494 o |
| `grille-59-94.srt` | `60000/1001` | 10 min | — | 166 | 10 190 o |
| `grille-60.srt` | `60/1` | 10 min | — | 170 | 10 440 o |
| `grille-absurde.srt` | `263/10` | 10 min | — | 177 | 10 810 o |
| `grille-24-decalee.srt` | `24/1` | 10 min | +2 999 ms | 168 | 10 242 o |
| `grille-24-courte.srt` | `24/1` | 10 s | — | 37 | 2 152 o |
| `melange-groupe.srt` | `30000/1001` | 10 min | — | 168 | 10 264 o |
| `melange-disperse.srt` | `25/1` | 10 min | — | 171 | 10 454 o |

Chaque position vaut `round(n × 1000 / R)`, arrondi au plus proche et la moitié
vers le haut, plus le décalage. Le pas entre deux répliques est irrégulier —
deux à cinq secondes — et tiré d'un générateur congruentiel écrit dans le
script : un pas constant rendrait les phases périodiques, et le fichier à
fréquence absurde ressortirait concentré sur une candidate qu'il ne touche pas.

**Les débuts et les fins sont tous les deux sur la grille.** Dans un vrai
fichier, seuls les débuts le sont : un *cue-out* est souvent calculé par une
règle de vitesse de lecture, ce qui fait descendre les fins entre 55 et 100. Une
fixture qui reproduirait ce comportement reste à écrire, et le jour où elle le
sera, c'est ici qu'il faudra la décrire.

## Ce qu'elles donnent

Concentration de phase sur les huit candidates, en pour cent. Refaisable par
`--measure` : ces chiffres sont mesurés sur ces fichiers-là, et non rapportés.

| Fixture | 23,976 | 24 | 25 | 29,97 | 30 | 50 | 59,94 | 60 |
| :------ | -----: | -: | -: | ----: | -: | -: | ----: | -: |
| `grille-23-976.srt` | **99,9** | 4,6 | 3,1 | 5,1 | 4,7 | 6,7 | 9,1 | 1,9 |
| `grille-24.srt` | 2,6 | **99,9** | 7,6 | 9,0 | 6,3 | 2,7 | 3,6 | 10,2 |
| `grille-25.srt` | 8,6 | 10,1 | **100** | 5,0 | 8,5 | **100** | 9,9 | 2,7 |
| `grille-29-97.srt` | 7,5 | 16,1 | 6,2 | **99,8** | 1,8 | 10,3 | **99,4** | 1,8 |
| `grille-30.srt` | 8,4 | 14,4 | 5,0 | 2,6 | **99,9** | 6,5 | 0,1 | **99,5** |
| `grille-50.srt` | 1,6 | 9,6 | 15,8 | 6,2 | 2,1 | **100** | 2,3 | 7,3 |
| `grille-59-94.srt` | 1,6 | 4,1 | 5,0 | 4,8 | 0,8 | 3,8 | **99,4** | 2,7 |
| `grille-60.srt` | 15,1 | 8,8 | 7,1 | 6,4 | 1,3 | 8,2 | 2,3 | **99,5** |
| `grille-absurde.srt` | 9,2 | 8,3 | 15,3 | 2,1 | 13,2 | 6,2 | 3,8 | 9,0 |
| `grille-24-decalee.srt` | 3,7 | **99,9** | 11,8 | 2,6 | 9,5 | 5,4 | 8,6 | 4,7 |
| `grille-24-courte.srt` | *92,7* | **99,9** | 3,9 | 10,9 | 22,2 | 10,1 | 27,2 | 8,6 |
| `melange-groupe.srt` | 3,4 | 2,7 | 2,6 | *65,1* | 1,8 | 1,7 | *66,4* | 1,1 |
| `melange-disperse.srt` | 4,1 | 7,6 | *75,4* | 8,1 | 2,6 | *84,9* | 2,9 | 11,1 |

## Ce que chacune éprouve

**Les huit grilles parfaites** disent que la déduction trouve la bonne
candidate. Elles disent aussi que **l'ambiguïté harmonique est réelle** : une
grille à 25 est incluse dans une grille à 50, et le fichier sort à 100 sur les
deux. Les seules paires ambiguës de l'ensemble normalisé sont celles dont l'une
est le multiple entier de l'autre — 25 et 50, 29,97 et 59,94, 30 et 60. Aucune
autre : 24 et 60 sont dans un rapport de deux et demi, donc une image sur deux
seulement coïncide.

**`grille-absurde.srt`** dit que **l'échec est bruyant**. Un fichier régulier,
mais sur une grille qui n'est aucune des huit, reste sous 16 partout : c'est un
« je ne sais pas » sans équivoque, et non une mauvaise réponse. C'est la
propriété qui autorise à se servir de la déduction, là où une heuristique de nom
de fichier ne l'aurait jamais offerte.

**`grille-24-decalee.srt`** dit que **la phase n'est pas un déchet**. Ses
positions sont celles de `grille-24.srt` translatées de 2 999 ms — un décalage
que Gaupol applique tel quel — et la grille reste à 99,9. Une première version
de la méthode cherchait une grille de *phase nulle* et notait à zéro un fichier
comme celui-ci.

**`grille-24-courte.srt`** dit que **distinguer 23,976 de 24 demande de
l'étendue**. Les deux grilles dérivent d'une milliseconde par seconde de film,
et une image en dure 41,7 : la phase sur la mauvaise candidate fait un tour
complet toutes les quarante-deux secondes. Sur dix secondes, elle n'en couvre
qu'un quart, et 23,976 reste à 92,7 contre 99,9. La déduction doit rendre
l'étendue qu'elle a eue sous les yeux, et non une concentration seule.

Le fichier est **serré** — trente-sept répliques en dix secondes — et c'est
délibéré. À densité de dialogue normale, dix secondes ne donnent que trois
répliques, et trois points ne disent rien : la concentration d'une candidate
quelconque y vaut déjà `1/√3`. Le hasard n'est pas l'ambiguïté.

**`melange-groupe.srt`** est le cas partiel **groupé** : deux tiers sur une
grille à 29,97, dernier tiers passé par une transformation affine qui détruit la
grille. C'est ce que produit un fichier assemblé, ou une section recalée. La
concentration tombe à 65, entre la grille nette et le bruit — et dire « les deux
tiers de vos débuts sont sur une grille 29,97 » est vrai sans aider personne. Ce
fichier existe pour qu'on doive dire **lesquels**.

**`melange-disperse.srt`** est le cas partiel **dispersé** : une position sur
cinq déplacée de une à dix images, comme le ferait une correction à la main dans
la table. La concentration tombe dans la même bande, et c'est précisément ce qui
rend le cas difficile : les deux fichiers se ressemblent par le chiffre et ne se
ressemblent pas du tout par la cause.

## Deux mises en garde

**Ces fixtures n'ont pas de fins réalistes**, comme dit plus haut. Un test qui
mesurerait la concentration des *fins* y trouverait 100, ce qu'aucun fichier réel
ne donne.

**Le fichier absurde n'est pas du bruit.** Il est parfaitement régulier, sur une
grille à 26,3 images par seconde. Un fichier écrit en millisecondes sans aucune
grille — celui qu'un logiciel de transcription produit — reste à écrire, et son
comportement attendu est le même : sous quelques pour cent partout.
