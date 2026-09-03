# Détection d'encodage

Ce que la détection du projet répond, sur les deux corpus qui peuvent lui être
soumis. Relevé à la main plutôt qu'engendré : il ne bouge qu'aux issues qui
touchent la détection, là où les performances bougent à chaque version.

**Un taux, et non « la détection marche ».** Une détection est un classifieur :
elle a un taux de succès, pas un résultat juste ou faux — c'est ce que l'issue
#290 a inscrit dans `score-encoding-detection.py`, avant qu'il y ait une
détection à nous. Il y en a une depuis l'issue #296, et voici son chiffre.

## Comment le rejouer

```console
$ cmake --build build/dev --target subedit_detect_encoding
$ ./src/scripts/score-encoding-detection.py \
      --detector './build/dev/bin/subedit_detect_encoding {}' --prive
```

`--prive` ajoute le corpus privé, absent de toute machine qui ne l'a pas — le
relevé se lit sans lui, en moins complet.

## Corpus étiqueté — 9 sur 9

`src/test/data/encodages/`, fabriqué par `encoding-fixtures.py` : la table qui
les écrit dit dans quel encodage chacune l'a été, donc l'étiquette vient avec le
fichier.

| Version | Fixtures reconnues | Confusions |
| :------ | -----------------: | :--------- |
| 0.8.16 — 2026-09-03 | **9/9** | aucune |

C'est le score qu'ICU avait rendu seule au banc d'essai de l'[ADR
0027](../adr/0027-icu-pour-les-encodages.md), et que `uchardet` rend aussi. Ce
n'est donc pas ICU qui est mesurée ici : c'est la détection du projet, qui
l'emploie après avoir tranché deux cas elle-même.

**Ce que le corpus ne prouve pas.** Neuf fixtures fabriquées sont le cas facile,
et l'une d'elles — `latin1.srt` — n'a pas de bonne réponse démontrable : sans
octet dans la plage `0x80–0x9f`, Latin-1 et CP1252 sont le même encodage. La
compter juste est une convention, pas une preuve.

## Corpus privé — 71 fichiers, sans étiquettes

`src/data/`, des fichiers réels. **Il ne donne aucun taux** : il n'y a pas de
vérité à lui confronter, et s'en inventer une serait pire que de n'en pas avoir.
Ce qu'il donne est ce qui se vérifie sans étiquette.

| | notre détection | `uchardet`, témoin |
| :--- | :-------------- | :----------------- |
| fichiers qui se décodent entièrement sous la réponse | **71/71** | 71/71 |
| réponses UTF-8 | 57 | 52 |
| réponses de la famille latine | 14 | 14, dont 5 dites `ascii` |

**Les deux se contredisent sur 15 fichiers sur 71**, et c'est le chiffre le plus
instructif de la table. Aucun de ces désaccords ne porte sur autre chose que la
famille latine : lire de l'ASCII comme du Latin-1, ou du CP1252 comme du
Latin-1, ne change aucun caractère du fichier tant qu'il n'est pas réécrit.

Que 71 réponses sur 71 décodent le fichier de bout en bout **ne prouve à peu
près rien** : un encodage mono-octet décode presque toute suite d'octets, et
c'est précisément d'où vient le mojibake. Le chiffre est nécessaire, il n'est
pas suffisant.

## Les deux règles que la détection tranche elle-même

Elles sont dans `core/text/encoding.cpp`, et ce sont elles qui séparent notre
réponse de celle d'ICU.

**Le BOM d'abord, toujours.** C'est la seule chose qu'un fichier de sous-titres
déclare de son encodage. Une heuristique qui passerait avant répondrait à une
question déjà tranchée.

**Des octets qui se décodent en UTF-8 sont de l'UTF-8, et ICU n'est pas
consultée.** Mesuré, et l'écart est la raison de la règle : sur
`valides/minimal.srt` — deux accents en cent quarante octets — le détecteur
d'ICU classe ISO-8859-1 à 81 et UTF-8 à 80. Un point de statistiques de lettres
contre une propriété structurelle, et le fichier revenait avec `Ã©` là où il
avait ses accents.
