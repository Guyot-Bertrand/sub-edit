# 0013 — Mettre les positions à l'échelle par un rationnel exact, arrondi une fois

**Date :** 2026-08-08
**Statut :** acceptée

## Contexte

Deux des trois opérations de positions du MVP multiplient une position par un
facteur : la transformation affine par deux repères, et la conversion de
fréquence d'image. Aucune n'est exprimable avec les types de la phase 1, qui
n'offrent que l'addition d'une durée et la conversion vers une image.

La conversion de fréquence semble pourtant se ramener à ce qui existe :
`fromFrame(toFrame(t, entrée), sortie)`. C'est faux, et la spec de la phase 1
le dit déjà sans qu'on en tire la conséquence — `ms → image → ms` n'est **pas**
l'identité. Ce chemin quantifie chaque position sur la grille d'images
d'entrée. Mesuré de 25 vers 24000/1001 :

| position | par les images | exact | écart |
| -------: | -------------: | ----: | ----: |
| 1010 ms | 1043 | 1053 | 10 ms |
| 1020 ms | 1084 | 1064 | 20 ms |
| 3600017 ms | 3753750 | 3753768 | 18 ms |

Écart maximal relevé sur 200 000 positions : **21 ms**, soit une demi-image.
Gaupol ne commet pas cette erreur : il calcule en secondes et n'arrondit qu'une
fois. Le benchmark de conversion livré à l'issue #9 la commet, lui, et mesure
donc une opération qui ne sera pas implémentée.

## Décision

Les positions se mettent à l'échelle par un **rationnel exact**, `Ratio`, avec
**un seul arrondi**, celui de la phase 1 : à l'entier le plus proche, la moitié
s'éloignant de zéro.

```cpp
Timestamp Timestamp::scaledBy(Ratio) const;
Duration  Duration::scaledBy(Ratio) const;
```

Une méthode nommée et non un opérateur : mettre une position à l'échelle est une
décision d'arrondi, et [0006](0006-positions-en-millisecondes.md) veut qu'aucune
ne se prenne implicitement.

La conversion de fréquence s'écrit `t × (entrée / sortie)`, sans passer par les
images.

La transformation s'écrit **`(t − x₁) × r + y₁`** et non `r × t + constante`,
qui est la forme de Gaupol. Outre l'arrondi unique, cette écriture fait tomber
les deux repères **exactement** sur les positions demandées : `(x₂ − x₁) × r`
vaut `y₂ − y₁` exactement, par construction de `r`. La forme de Gaupol ne le
garantit pas.

## Alternatives écartées

- **L'aller-retour par les images.** Séduisant parce qu'il réutilise l'existant,
  et faux pour la raison mesurée ci-dessus. C'est l'alternative qu'il fallait
  nommer : elle se serait glissée dans le code sans qu'on la remarque.
- **Un facteur en virgule flottante**, ce que fait Gaupol. Écarté pour la raison
  qui a déjà écarté la fréquence en `double` en [0006](0006-positions-en-millisecondes.md) :
  `25 / (24000/1001)` n'est pas représentable exactement, et l'erreur se propage
  à chaque position. Un rationnel supprime la question.
- **Arrondir séparément le coefficient et la constante**, forme la plus directe
  de la transformation. Écarté : deux arrondis là où un suffit, et les repères
  ne tombent plus sur leur cible.

## Conséquences

Toute position calculée l'est à la milliseconde près du résultat exact, et non à
la demi-image près.

Les calculs intermédiaires tiennent dans un `std::int64_t` : une position
plausible ne dépasse pas `3,6 × 10⁸` ms, et un rationnel réduit garde un
numérateur du même ordre — le produit reste loin des bornes.

`Ratio` s'ajoute aux types du module `time`. C'est un cinquième type là où
[0006](0006-positions-en-millisecondes.md) en nommait trois et
[0011](0011-numero-d-image-en-type-fort.md) un quatrième ; il ne représente pas
une grandeur mais un **facteur**, ce qui justifie qu'il ne soit ni une position
ni une durée.

Les opérations qui arrondissent ne peuvent pas s'inverser par un second calcul :
transformation et conversion doivent retenir les positions antérieures, là où le
décalage se contente d'un `Duration`. C'est ce que
[0010](0010-annulation-par-commandes.md) appelle capturer le strict nécessaire —
qui n'est pas la même chose selon l'opération.

Défaire cette décision changerait silencieusement toutes les positions
calculées : aucun test ne verrait la différence sans être réécrit, et les
fichiers déjà enregistrés seraient incohérents avec les nouveaux. Le déclencheur
d'un réexamen serait une mesure montrant que le coût du rationnel pèse sur un
fichier réel — les benchmarks de la phase 2 sont là pour le dire.
