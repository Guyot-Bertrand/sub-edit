# 0026 — Laisser le graphe d'inclusions tel quel, et pourquoi

**Date :** 2026-08-31
**Statut :** acceptée

## Contexte

L'issue [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) a rendu
l'analyse statique incrémentale : clang-tidy est accroché à la règle de
compilation de chaque source, et ce qui décide de réanalyser un fichier est le
graphe de dépendances que le compilateur a écrit. Une propriété jusque-là
invisible est devenue mesurable, et chère : **toucher un en-tête coûte
exactement le nombre d'unités de traduction qui l'atteignent.**

Mesuré aussitôt, sur douze cœurs :

```
rien n'a changé            0,4 s
un .cpp touché             1 unité
un en-tête profond touché  129 unités
```

D'où la question, qui se pose naturellement une fois le chiffre sous les yeux :
**peut-on déclarer plus de types par anticipation, pour réduire le rayon d'une
modification d'en-tête ? Et pour quel gain ?**

Elle mérite une réponse mesurée, parce que la réponse par défaut du métier —
« oui, toujours, les déclarations anticipées sont une bonne pratique » — est
celle qu'on applique sans vérifier.

### Ce qui a été mesuré, et comment

Le graphe vient de `ninja -C build/tidy -t deps`, c'est-à-dire des fichiers de
dépendances que le compilateur a produits en compilant. Pas d'un `grep` sur les
lignes `#include` : la fermeture transitive réelle, celle qui gouverne les
réanalyses. 337 unités de traduction, 125 en-têtes du projet, 3062 arêtes.

## Décision

**Le graphe d'inclusions reste tel quel.** On ne généralise pas les déclarations
anticipées, et on n'écrit pas de règle qui les encourage.

Une seule coupe est retenue comme envisageable, et elle n'est pas une
déclaration anticipée : déplacer les deux fonctions-passerelles entre `Frame` et
`Timestamp`. Elle n'est pas faite ici — voir « Conséquences ».

## Alternatives écartées

### Généraliser les déclarations anticipées — écartée, structurellement impossible

Les en-têtes au plus gros rayon sont **tous des types-valeurs**, et c'est
exactement ce qu'une déclaration anticipée ne sait pas servir.

| En-tête | Unités atteintes | sur 337 |
| :------ | ---------------: | ------: |
| `core/time/ratio.hpp` | 153 | 45 % |
| `core/time/frame_rate.hpp` | 149 | 44 % |
| `core/time/duration.hpp` | 145 | 43 % |
| `core/time/frame.hpp` | 130 | 39 % |
| `core/time/timestamp.hpp` | 129 | 38 % |
| `core/model/subtitle_format.hpp` | 126 | 37 % |
| `core/model/source_file.hpp` | 123 | 36 % |

`Timestamp` porte `Duration`, `Frame`, `FrameRate` et `Ratio` **par valeur, dans
des corps `constexpr` écrits en ligne** : il appelle `rate.millisecondsPerFrame()`,
`frame.number()`, `factor.scale(…)`. `Subtitle` porte `Timestamp start` et
`FormatExtras extras` comme membres. `Project` expose `subtitleAt()` en ligne sur
un `std::vector<Subtitle>`.

Le compilateur a besoin du type complet dans chacun de ces cas. Ce n'est pas
« peu commode », c'est **impossible** : une fonction `constexpr` se définit dans
l'en-tête, et son corps ne compile pas sur un type incomplet.

**Et c'est notre propre conception qui l'impose.** Les
[principes de conception](../principes-de-conception.md) demandent des modèles
typés et des valeurs plutôt que des indirections ; l'ADR
[0013](0013-mise-a-l-echelle-exacte-des-positions.md) fait de `Ratio` un type
exact manipulé par valeur. Un graphe d'inclusions dense est la contrepartie de
ce choix, pas un défaut d'hygiène.

### Couper les arêtes les plus lourdes — écartée, le plafond est trop bas

Le gain de chaque coupe a été calculé exactement, arête par arête, en retirant
l'arête du graphe et en recomptant la fermeture transitive. Indépendamment de
toute faisabilité :

| Gain | Arête coupée |
| ---: | :----------- |
| **6,2 %** | `model/project.hpp` ⇏ `model/subtitle.hpp` |
| 4,0 % | `time/timestamp.hpp` ⇏ `time/frame.hpp` |
| 3,4 % | `model/subtitle.hpp` ⇏ `model/boundary.hpp` |
| 3,2 % | `model/subtitle.hpp` ⇏ `model/format_extras.hpp` |
| 3,0 % | `model/subtitle.hpp` ⇏ `time/timestamp.hpp` |

**La meilleure coupe imaginable vaut 6,2 % du graphe, et elle est infaisable :**
`Project` expose `subtitles()`, `setSubtitles()` et `subtitleAt()` en ligne sur
son `std::vector<Subtitle>`. Les sortir hors ligne mettrait un appel de fonction
sur `subtitleAt`, appelé une fois par ligne visible à chaque repeint de la
table. C'est un prix payé à chaque image pour économiser des secondes d'analyse
une fois par trimestre.

### Traquer les inclusions mortes — écartée, il n'y en a pas

`misc-include-cleaner` — désactivé dans `.clang-tidy`, avec sa justification —
a été relancé comme instrument de mesure sur deux unités du noyau : **une seule
inclusion inutile.** Il n'y a pas de gisement.

## Conséquences

### Ce que la mesure a retourné : le rayon ne coûte que quand l'en-tête change

C'est le résultat que la question n'attendait pas, et il vaut plus que la
réponse elle-même. En pondérant le rayon par le nombre de commits qui ont touché
chaque fichier depuis le début du projet :

| Coût | Rayon | Commits | En-tête |
| ---: | ----: | ------: | :------ |
| **783** | 87 | **9** | `core/model/project.hpp` |
| 438 | 146 | 3 | `core/time/frame_rate.hpp` |
| 300 | 150 | **2** | `core/time/ratio.hpp` |
| 300 | **15** | **20** | `gui/main_window.hpp` |
| 284 | 71 | 4 | `core/io/file_system.hpp` |
| — | 130 | **1** | `core/time/frame.hpp` |

**`ratio.hpp` touche 45 % du dépôt et a changé deux fois en deux ans.
`frame.hpp` en touche 39 % et a changé une fois.** Ce sont des types-valeurs
finis : ils ont le plus gros rayon parce qu'ils sont fondamentaux, et ils ne
bougent pas *parce qu'*ils sont fondamentaux. Les deux propriétés ont la même
cause, ce qui est précisément ce qui rend l'optimisation vaine.

Le vrai foyer de coût est ailleurs, et il est double : `project.hpp`, dont le
rayon est moyen mais qui bouge souvent ; et `gui/main_window.hpp`, dont le rayon
est petit et le churn énorme.

### En secondes, puisque c'est la seule unité qui compte

Mesuré à douze cœurs, sur l'arbre `build/tidy` à jour :

| En-tête touché | Unités | Temps |
| :------------- | -----: | ----: |
| `core/model/project.hpp` | 90 | **331 s** |
| `gui/main_window.hpp` | 16 | **115 s** |

Une unité d'interface coûte trois fois une unité de noyau — 7,2 s contre 3,7 —
parce qu'elle traîne les en-têtes de Qt. **Le rayon seul est donc un mauvais
estimateur du coût :** seize unités d'interface valent trente unités de noyau.

### Ce qui reste ouvert, et ce qui ne l'est pas

**La passerelle `Frame` ↔ `Timestamp` est la seule coupe nette.**
`timestamp.hpp` inclut `frame.hpp` pour deux fonctions — `fromFrame(Frame,
FrameRate)` et `toFrame(FrameRate)` — qui ont **douze utilisateurs réels** :
quatre dans la bibliothèque, huit dans les tests. Elles imposent `frame.hpp` à
130 unités.

Les déplacer en fonctions libres dans un en-tête de conversion ferait tomber ce
rayon de 130 à 12, sans rien sortir hors ligne et sans toucher au modèle. Ce
n'est pas une déclaration anticipée mais un déplacement de fonction, et c'est ce
qui le rend possible : la dépendance vient d'une commodité posée sur `Timestamp`
plutôt que d'une propriété du type.

**Elle n'est pas faite, et pas parce qu'elle est difficile.** `frame.hpp` a
changé une fois en deux ans : le gain espéré est de quelques minutes sur la
durée de vie du projet. La coupe se fera le jour où l'on rouvrira ce fichier
pour une autre raison, pas pour elle-même.

### Ce qui rouvrirait la question

Trois choses, et aucune n'est une opinion sur les déclarations anticipées :

- **`project.hpp` continue de bouger.** Neuf commits l'ont touché, chacun coûtant
  331 s d'analyse. Si son interface ne se stabilise pas, le levier n'est pas
  l'inclusion mais la stabilité — sortir de son en-tête ce qui bouge, et non ce
  qu'il inclut.
- **Une unité d'interface coûte trois fois une unité de noyau.** Si `src/` devient
  majoritairement du Qt, le calcul ci-dessus change de conclusion sans qu'aucun
  chiffre du graphe ait bougé.
- **Un type-valeur du noyau se met à changer souvent.** Ce serait le signe que
  quelque chose lui a été ajouté qui n'en relève pas, et la réponse serait de le
  lui retirer — pas de le déclarer par anticipation.

### Ce que cette décision coûterait à défaire

Rien : elle n'écrit aucun code. Elle écrit une mesure et un raisonnement, pour
qu'on ne les refasse pas. Le coût de ne pas l'avoir écrite serait de rouvrir la
question tous les six mois et d'y répondre chaque fois par l'intuition du
métier, qui donne ici la mauvaise réponse.
