# Conversion entre formats

Ce qu'une conversion perd, **mesuré** paire par paire, et ce que chaque format
oblige à perdre, **déclaré** à côté.

**Une perte, et non un résultat.** L'[ADR 0009](../adr/0009-texte-en-chaine-brute.md)
annonce depuis la phase 1 qu'un modèle structuré sert de pivot « uniquement lors
d'une conversion entre formats, où la perte est inévitable et **assumée** ».
*Assumée* suppose qu'on la connaisse ; c'est ce que l'issue #339 a inscrit dans
`measure-conversion-loss.py`, sur le modèle de #290 pour la détection
d'encodage.

## Comment le rejouer

```console
$ make conversion
```

C'est une étape de `check-local` : la mesure se rejoue à chaque pull request, et
**une divergence d'avec le relevé ci-dessous se voit**. Une perte qui s'aggrave
échoue, une perte qui se réduit invite à `make conversion-record`, une perte
inchangée se tait. Ce n'est pas un seuil : le nombre comparé est celui de la
dernière mesure, jamais une barre qu'on aurait posée.

Pour voir ce qui ne revient pas, fichier par fichier :

```console
$ ./src/scripts/measure-conversion-loss.py --diff
```

**Le binaire est un argument**, comme le détecteur l'est pour le score
d'encodage : la mesure porte sur la sous-commande `convert` que l'utilisateur
tape, et deux binaires se comparent sur le même corpus sans qu'on touche à
l'outil.

```console
$ ./src/scripts/measure-conversion-loss.py --binary ./build/release/bin/subedit-cli
```

## Ce qui est mesuré

**Un aller-retour, et le format de départ est la toise.** Une conversion ne se
regarde pas de l'extérieur : le fichier d'arrivée est dans un autre format, et
il n'y a rien à quoi le comparer. Un fichier part en `A`, passe par `B`, revient
en `A` — ce qui ne revient pas est ce que `B` n'a pas su porter.

La mesure n'est exacte que grâce à une propriété que le harnais de #338 tient
par ailleurs : **un fichier du corpus est déjà ce que notre écriture produit**,
octet pour octet. Sans elle, un écart pourrait venir de la mise en forme plutôt
que du passage.

Le corpus est celui de `src/test/data/` — `valides/`, dix fichiers qui ne se
ressemblent pas, et `formats/`, la même scène écrite neuf fois. Le corpus privé
n'est pas lu : absent d'une machine sur deux, il ferait dire deux choses à la
même porte.

<!-- relevé engendré : ne pas modifier à la main -->

    aller-retour intacts : 60/126

Relevé sur la version 0.9.21, le 2026-09-09.

| Départ \ Arrivée | `ass` | `lrc` | `microdvd` | `mpl2` | `srt` | `ssa` | `subviewer2` | `tmplayer` | `vtt` |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `ass` | — | 0/1 | · | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| `lrc` | 1/1 | — | · | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 |
| `microdvd` | · | · | — | · | · | · | · | · | · |
| `mpl2` | 1/1 | 0/1 | · | — | 1/1 | 1/1 | 1/1 | 0/1 | 1/1 |
| `srt` | 5/8 | 0/8 | · | 5/8 | — | 5/8 | 5/8 | 0/8 | 7/8 |
| `ssa` | 0/1 | 0/1 | · | 0/1 | 0/1 | — | 0/1 | 0/1 | 0/1 |
| `subviewer2` | 0/1 | 0/1 | · | 0/1 | 0/1 | 0/1 | — | 0/1 | 0/1 |
| `tmplayer` | 1/1 | 0/1 | · | 1/1 | 1/1 | 1/1 | 1/1 | — | 1/1 |
| `vtt` | 3/4 | 0/4 | · | 3/4 | 3/4 | 3/4 | 3/4 | 0/4 | — |

| Fichier | Passage par | Première ligne qui ne revient pas |
| :------ | :---------- | :-------------------------------- |
| `valides/balises.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/balises.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/cadence.srt` | `subviewer2` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `ssa` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `ass` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `mpl2` | `00:00:01,010 --> 00:00:02,020` |
| `valides/cadence.srt` | `tmplayer` | `00:00:01,010 --> 00:00:02,020` |
| `valides/cadence.srt` | `lrc` | `00:00:01,010 --> 00:00:02,020` |
| `valides/complet.vtt` | `srt` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `subviewer2` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `ssa` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `ass` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `mpl2` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `tmplayer` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `lrc` | `WEBVTT - Dialogue` |
| `valides/coordonnees.srt` | `vtt` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `subviewer2` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `ssa` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `ass` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `mpl2` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/crlf-bom.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/crlf-bom.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/heures.vtt` | `tmplayer` | `00:00:01.000 --> 00:00:03.000` |
| `valides/heures.vtt` | `lrc` | `00:00:01.000 --> 00:00:03.000` |
| `valides/mentions.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/mentions.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/minimal.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,500` |
| `valides/minimal.srt` | `lrc` | `00:00:01,000 --> 00:00:03,500` |
| `valides/minimal.vtt` | `tmplayer` | `00:01.000 --> 00:03.500` |
| `valides/minimal.vtt` | `lrc` | `00:01.000 --> 00:03.500` |
| `valides/trois.srt` | `subviewer2` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `ssa` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `ass` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `mpl2` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/trois.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.ass` | `srt` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `vtt` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `subviewer2` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `ssa` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `mpl2` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `tmplayer` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `lrc` | `Title: The scene, in nine formats` |
| `formats/scene.mpl2.txt` | `tmplayer` | `[10][30]The harbour wakes before\|the town does.` |
| `formats/scene.mpl2.txt` | `lrc` | `[10][30]The harbour wakes before\|the town does.` |
| `formats/scene.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.ssa` | `srt` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `vtt` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `subviewer2` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `ass` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `mpl2` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `tmplayer` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `lrc` | `Title: The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `srt` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `vtt` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `ssa` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `ass` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `mpl2` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `tmplayer` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `lrc` | `[TITLE]The scene, in nine formats` |
| `formats/scene.tmplayer.txt` | `lrc` | `00:00:01:The harbour wakes before\|the town does.` |
| `formats/scene.vtt` | `tmplayer` | `00:01.000 --> 00:03.000` |
| `formats/scene.vtt` | `lrc` | `00:01.000 --> 00:03.000` |

**26 paire(s) que la conversion refuse de faire** — un fichier temporel vers un format en images, sans grille et sans fréquence donnée. Un refus n'est pas une perte, et n'entre donc pas dans le compte ci-dessus.

<!-- fin du relevé -->

## La colonne qui ne se remplit pas, et ce qu'elle dit

**On ne revient pas à MicroDVD.** Sa colonne est vide, et sa ligne aussi, parce
qu'un aller-retour qui le quitte ne peut pas y rentrer : le fichier
intermédiaire a perdu la fréquence, et la conversion refuse d'en inventer une —
`convert --to microdvd` demande alors `--frame-rate`.

C'est **la perte déclarée la plus nette des neuf formats**, et la seule qui ne
se compte pas en pertes. Un en-tête perdu laisse un fichier ; une fréquence
perdue laisse une conversion impossible.

Ces paires sont donc comptées à part. **Un refus n'est pas une perte** : les
compter comme telles dirait le contraire de ce qu'ils sont, et gonflerait le
dénominateur d'un chiffre qui ne mesure plus rien.

## Ce que le nombre vaut, et ce qu'il ne vaut pas

**Il mélange sciemment deux choses** : ce qu'un format ne *peut* pas porter, et
ce que notre conversion perdrait sans y être obligée. Les séparer demanderait de
déclarer les soixante-douze cases à la main — et une déclaration écrite à la
main se périme en silence, ce que #289 a corrigé pour le corpus et que rien
n'obligerait à recommencer ici.

Ce qu'il garantit est plus étroit, et se tient tout seul : **une perte ne
s'aggrave pas sans qu'on le voie.** Ce qu'elle vaut en droit se lit ci-dessous,
en prose — et c'est là, pas dans le nombre, que la politique de dégradation se
décide.

**Et il baisse une fois, à l'arrivée de TMPlayer et de LRC** : 47/80 devient
60/126, soit une part qui recule de trois cinquièmes à moins de la moitié. Rien
n'a régressé. Deux formats qui ne portent pas de fin sont entrés dans la
matrice, et **toute paire qui les traverse perd les siennes par construction** —
seize colonnes et deux lignes de plus, presque toutes perdantes d'avance. Le
relevé a donc été réenregistré sciemment, ce que le contrôle demande de faire
plutôt que de subir.

C'est aussi ce qui montre la limite du chiffre unique, et pourquoi la matrice
est là : **une ligne se lit, un total ne se lit pas.** Les lignes `lrc` et
`tmplayer` sont pleines — on part de ces formats sans rien perdre, puisqu'ils
n'ont rien de plus à perdre. Ce sont leurs colonnes qui sont vides.

## La perte déclarée

Ce qui suit est **une propriété des formats, pas de notre code**. Elle s'écrit
avant la première ligne de lecteur, et c'est ce sur quoi le cadrage de la phase 9
(#337) tranche sa politique de dégradation.

### Ce qui ne traverse jamais une frontière de format

**Les données propres à un sous-titre** — les coordonnées de SubRip, l'identifiant
et les réglages d'une cellule WebVTT, le style nommé, les marges et l'effet
d'une réplique SSA. Chaque format a les siennes, et elles ne se traduisent pas :
`align:start position:10%` n'a pas d'équivalent SubRip, `X1:040 X2:600` n'en a
pas en WebVTT.

**L'en-tête**, pour la même raison. `[Script Info]` n'est pas une entête WebVTT,
et le texte libre qui suit `WEBVTT` n'est pas du SSA.

Ce sont les deux seules pertes que la mesure trouve aujourd'hui, et elles sont
toutes deux de ce côté-ci : `valides/coordonnees.srt` perd ses coordonnées en
passant par WebVTT, `valides/complet.vtt` perd son en-tête, son bloc `STYLE`, sa
note, l'identifiant de sa première cellule et ses réglages en passant par SubRip.

### Ce qui traverse, sauf là où le format d'arrivée ne sait pas le porter

| Format | La fin d'une réplique | Les sauts de ligne | L'italique | Précision |
| :----- | :-------------------- | :----------------- | :--------- | :-------- |
| SubRip | portée | portés | `<i>…</i>` | milliseconde |
| WebVTT | portée | portés | `<i>…</i>` | milliseconde |
| SubViewer 2 | portée | `[br]` | `<i>…</i>` | centième |
| Sub Station Alpha | portée | `\N` | `{\i1}…{\i0}` | centième |
| Advanced SSA | portée | `\N` | `{\i1}…{\i0}` | centième |
| MicroDVD | portée, **en images** | `\|` | `{Y:i}` | image |
| MPL2 | portée | `\|` | `/` en tête de ligne | dixième |
| TMPlayer | **absente** | `\|` | **aucun vocabulaire** | seconde |
| LRC | **absente** | **absents** | **aucun vocabulaire** | centième |

**Deux formats n'ont pas de fin.** LRC et TMPlayer ne portent qu'une position
par ligne ; une lecture invente les fins, et la fin inventée est ce qui ressort
d'un aller-retour. C'est la promesse « à vide » de la table de #338, vue de
l'autre côté.

**LRC ne porte pas de saut de ligne** : une réplique de deux lignes en revient
sur une seule, et les mots sont recollés par une espace.

**MicroDVD compte en images**, et n'énonce aucune grille. Une conversion vers
lui suppose une fréquence que le fichier ne dira pas à la lecture suivante — la
promesse conditionnelle de #338, et la question que le cadrage doit trancher.

**Une borne que le tableau ne montre pas, et où nous allons plus loin que
Gaupol.** LRC écrit `mm:ss.cc` et n'a pas de champ d'heures ; ce sont donc les
minutes qui les portent, et `[62:03.00]` est une heure et deux minutes. Le motif
de Gaupol prend exactement deux chiffres de minutes : au-delà de 99 minutes 59
il ne relit plus le fichier, et son écriture, qui découpe la chaîne `HH:MM:SS`
après les heures, écrit `02:03.00` pour la même position — une réplique déplacée
d'une heure, en silence.

Notre lecture prend les minutes pour ce qu'elles sont, aussi longues qu'elles
soient, et notre écriture les rend entières. L'aller-retour est alors exact quel
que soit l'endroit du film, ce qui est la seule chose que le format demandait.
Le signe est accepté des deux côtés, comme chez Gaupol.

**La précision, en revanche, se mesure — et pas là où on l'attendait.** La scène
de #338 pose toutes ses positions sur la seconde entière, exactement pour
qu'elle soit la même scène chez le plus pauvre des neuf ; elle ne pouvait donc
rien en dire. C'est `valides/` qui l'exerce, sans avoir été écrit pour :
`cadence.srt` porte une position à `01:00:00,017`, et elle revient à `,020` d'un
passage par SubViewer 2. Le corpus qui ne ressemble à rien trouve ce que le
corpus régulier ne peut pas trouver, et c'est la raison de garder les deux.

## Ce qui n'entrait pas dans la mesure

Sept des neuf rendus de la scène ne s'ouvraient pas au début de la phase 9 : les
lecteurs n'étaient pas écrits. Le relevé les comptait à part plutôt que de les
taire, et leur nombre est descendu d'un à chaque format livré, jusqu'à zéro. La
ligne disparaît du relevé quand il n'y a rien à compter ; le compteur, lui,
reste, parce que la question se reposera au premier corpus qu'on ajoutera.
