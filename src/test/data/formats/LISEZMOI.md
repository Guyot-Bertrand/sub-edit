# Une scène, dans neuf formats

Neuf fichiers qui disent **la même chose** : quatre répliques, aux mêmes
positions, dont deux sur deux lignes et une en italique. Ce qui change d'un
fichier à l'autre est le format, et lui seul — c'est la discipline
d'[`encodages/`](../encodages), qui ne fait varier que l'encodage.

C'est ce qui permet de comparer **une lecture à une autre** plutôt que deux
textes différents, et c'est de là que sort tout ce qu'on peut dire d'une
conversion.

## La scène

| # | Début | Fin | Texte |
| -: | :--- | :--- | :---- |
| 1 | 00:00:01,000 | 00:00:03,000 | `The harbour wakes before` ⏎ `the town does.` |
| 2 | 00:00:04,000 | 00:00:06,000 | `Nothing moves on the water.` |
| 3 | 00:00:08,000 | 00:00:11,000 | *`A gull turns once above the mast`* ⏎ *`and settles.`* |
| 4 | 00:00:12,000 | 00:00:15,000 | `Then the light comes.` |

**Les positions tombent toutes sur la seconde**, et c'est une contrainte du plus
pauvre des neuf : TMPlayer ne compte qu'en secondes entières. Une scène qu'il ne
saurait pas écrire ne serait plus la même scène chez lui.

**La grille de MicroDVD est de 25 images par seconde**, exactement quarante
millisecondes par image — donc les quatre positions tombent aussi sur une image.
**Le fichier ne le dit pas**, aucun MicroDVD ne le dit, et c'est tout le sujet de
la promesse conditionnelle ci-dessous.

## Ce que chaque rendu perd, et c'est le tableau qui compte

| Fichier | Format | Sauts de ligne | Italique | Fins | Précision |
| :------ | :----- | :------------- | :------- | :--- | :-------- |
| `scene.srt` | SubRip | ⏎ | `<i>…</i>` | portées | ms |
| `scene.vtt` | WebVTT | ⏎ | `<i>…</i>` | portées | ms |
| `scene.subviewer2.sub` | SubViewer 2 | `[br]` | `<i>…</i>` | portées | centième |
| `scene.ssa` | Sub Station Alpha | `\N` | `{\i1}…{\i0}` | portées | centième |
| `scene.ass` | Advanced SSA | `\N` | `{\i1}…{\i0}` | portées | centième |
| `scene.microdvd.sub` | MicroDVD | `\|` | `{Y:i}` | portées, **en images** | image |
| `scene.mpl2.txt` | MPL2 | `\|` | `/` en tête de ligne | portées | dixième |
| `scene.tmplayer.txt` | TMPlayer | `\|` | **perdu** | **absentes** | seconde |
| `scene.lrc` | LRC | **perdus** | **perdu** | **absentes** | centième |

Trois pertes sont visibles à l'œil dans les fichiers eux-mêmes, et c'est fait
exprès : LRC rend les deux répliques de deux lignes sur une seule, LRC et
TMPlayer ne portent aucune fin, et ni l'un ni l'autre n'a de vocabulaire pour
l'italique. **La déclaration complète de ces pertes appartient au relevé de
conversion** — issue #339 ; ce tableau-ci décrit neuf fichiers, pas
soixante-douze paires.

## Elles s'écrivent à la main, et il n'y aura pas de fabrique

C'est tranché en #336, et la raison ne s'invoque pas des deux fabriques
existantes — elle les distingue.

**Ni `encoding-fixtures.py` ni `subtitle-fixtures.py` ne réimplémente ce qu'il
éprouve.** Le premier appelle le codec de Python, un tiers déterministe ; le
second pose de l'arithmétique exacte, et la syntaxe SubRip autour tient en trois
lignes.

**Un générateur d'ASS n'aurait aucun tiers à appeler.** Les sections, la ligne
`Format:`, l'ordre des champs, le `\N`, les centièmes sont exactement ce que la
phase 9 écrit en C++. Un fichier engendré depuis notre compréhension du format,
relu par notre lecteur du même format, ne prouverait que leur accord — et il en
aurait l'air d'autre chose, ce qui est pire que rien.

**Et un `.ass` se relit dans un diff**, ce qu'un fichier en CP1252 ne sait pas
faire. L'argument qui a fondé les deux fabriques ne se transporte pas.

## D'où vient la forme

**Les syntaxes et les en-têtes sont transcrits de fichiers réels** — les
exemples que Gaupol publie avec son code pour les sept formats, et deux fichiers
du corpus privé pour MicroDVD et Advanced SSA. Lus par un humain, jamais liés :
aucun test ne va les chercher là où ils sont, et le corpus privé ne se cite pas.

**Les mots, en revanche, sont les nôtres.** Le texte d'un fichier réel est du
contenu, et le dépôt n'en publie pas ; ce qui se transcrit d'un format est sa
forme.

## Ce que la scène ne porte pas, et à qui c'est dû

**Un MicroDVD du monde réel s'ouvre souvent sur deux lignes de crédits placées à
`{1}{1}`** — le nom de l'encodage vidéo, la taille du fichier — qu'un lecteur
naïf compte pour des répliques. La scène ne les porte pas : neuf rendus d'une
même scène ne peuvent pas en avoir un dixième. La fixture qui les porte est due
à l'issue qui écrira le lecteur MicroDVD, et sa place est
[`malformes/`](../malformes).

## Ce que le harnais leur demande

[`format_corpus_test.cpp`](../../unit/core/format/format_corpus_test.cpp) porte
une **table de promesses**, une ligne par format, et **refuse un fichier de ce
répertoire qui n'y figure pas** — comme `valides/` refuse depuis #289 une
fixture qu'aucune liste n'énumère.

Trois promesses, et la deuxième est la raison d'être de la table :

- **l'octet, pleinement** — le fichier revient identique, et ça veut dire
  quelque chose ;
- **l'octet, mais à vide** — LRC et TMPlayer. La lecture invente des fins,
  l'écriture ne rend que des débuts : **les octets reviendraient identiques quoi
  que la lecture ait inventé.** Le test passerait sans rien prouver. Ces deux
  formats portent donc, dans la table, les fins attendues **écrites à la main** ;
- **l'octet, sous condition déclarée** — MicroDVD. Il compte en images, le
  modèle compte en millisecondes ([ADR 0006](../../../../docs/adr/0006-positions-en-millisecondes.md)),
  et le fichier n'énonce aucune grille.

## Pourquoi pas dans `valides/`

Deux raisons, et la seconde est un piège vérifié.

**Sept des neuf ne s'ouvraient pas encore.** `valides/` est le corpus de ce qui
s'ouvre et revient octet pour octet ; un `.ass` déposé là avant qu'un lecteur
d'ASS existe aurait rendu la porte rouge pendant toute la phase. Les neuf
s'ouvrent aujourd'hui, mais deux ne peuvent toujours pas y aller : `valides/`
ne demande que les octets, et sur TMPlayer et LRC les octets ne prouvent rien.
C'est la table de promesses qui a ce qu'il faut.

**Et `valides/` ne tolère aucun fichier qui ne soit pas un sous-titre.** Depuis
#289 le répertoire *est* la liste : `validFiles()` retient tout fichier régulier
et le premier cas exige qu'il s'ouvre. Ce `LISEZMOI.md`, déposé là, ferait
échouer le test. Ici il est exempté nommément, ce qui se lit dans le harnais.

**Un format livré ne déménage pas pour autant.** Son rendu reste ici, où la
scène est ; ce qui change est la ligne de la table qui le concerne.
