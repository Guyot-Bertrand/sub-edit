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

    aller-retour intacts : 10/12

Relevé sur la version 0.9.13, le 2026-09-07.

| Départ \ Arrivée | `srt` | `vtt` |
| :--- | ---: | ---: |
| `srt` | — | 7/8 |
| `vtt` | 3/4 | — |

| Fichier | Passage par | Première ligne qui ne revient pas |
| :------ | :---------- | :-------------------------------- |
| `valides/complet.vtt` | `srt` | `WEBVTT - Dialogue` |
| `valides/coordonnees.srt` | `vtt` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |

7 fichier(s) du corpus ne s'ouvrent pas encore et n'entrent dans aucune mesure.

<!-- fin du relevé -->

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

**Deux bornes que le tableau ne montre pas.** LRC écrit `mm:ss.cc` : il n'a pas
de champ d'heures, donc rien au-delà de 99 minutes 59, et rien avant le début.
Et la précision ci-dessus n'est pas mesurée : **la scène de #338 pose toutes ses
positions sur la seconde entière**, exactement pour qu'elle soit la même scène
chez le plus pauvre des neuf. Une perte de précision ne se verra donc que le
jour où le corpus portera un fichier qui l'exerce.

## Ce qui n'entre pas encore dans la mesure

Sept des neuf rendus de la scène ne s'ouvrent pas : la phase 9 n'a pas encore
écrit leurs lecteurs. Le relevé les compte à part plutôt que de les taire, et
leur nombre descend d'un à chaque format livré.
