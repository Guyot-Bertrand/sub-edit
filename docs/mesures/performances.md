# Journal des performances

Ce que les benchmarks ont mesuré, version par version. Rejoués en `Release`,
le mode livré — c'est le seul dont les chiffres veulent dire quelque chose pour
un utilisateur.

**Aucun seuil d'alerte, délibérément.** La variance naturelle des mesures n'est
pas connue : le même tri a rendu 240 µs puis 456 µs à une heure d'intervalle,
sur la même machine et le même binaire. Poser un cliquet sur des chiffres
pareils, ce serait se garantir de fausses alertes et l'habitude de les ignorer.
La table des extrêmes ci-dessous existe pour que cette variance devienne
connue ; le seuil viendra quand elle le sera. Voir
[l'ADR 0015](../adr/0015-memoire-des-mesures.md).

## La charge de la machine, et pourquoi elle est écrite

Une mesure prise pendant qu'un navigateur compile du JavaScript ne dit rien du
code : elle dit l'état de la machine. Chaque relevé porte donc la **charge
moyenne d'une minute** relevée juste avant, dans son en-tête, et la règle qui en
découle est mécanique :

> **Un relevé pris au-dessus de `BENCH_MAX_LOAD` — une et demie, par défaut —
> entre au journal mais ne fixe aucun extrême.**

`make bench` attend d'abord que la charge redescende, trois minutes au plus : la
cause la plus fréquente est la cible qui précède, et la moyenne d'une minute met
une minute à l'oublier. Passé ce délai il mesure quand même, le dit, et le
relevé reste hors de la table ci-dessous.

**Il n'attend que ce qui vient.** Si la charge ne baisse pas — une demi-minute
sans progrès suffit à le dire — il renonce aussitôt : une machine partagée avec
un autre travail de longue haleine ne redeviendra pas calme dans le délai, et
l'attendre coûterait trois minutes à chaque exécution. Notre propre build, lui,
décroît franchement ; c'est ce qui distingue les deux cas, et non le niveau de
la charge, que les deux poussent aussi haut.

Sans cette règle, un maximum posé par du bruit est **définitif** — la table
n'est jamais élaguée — et rend la mesure aveugle à toute régression plus petite
que ce bruit.

**Cela s'est produit.** Le relevé de la version `0.2.15` a été pris sous une
charge de 7,2 et a posé treize maxima d'un coup, dont un à 3,4 fois sa valeur
habituelle, pour un ticket qui ne touchait aucun chemin mesuré. La table a été
recalculée sans lui, depuis les relevés conservés — tous les extrêmes qu'elle
citait en venaient, donc rien n'a été perdu qu'un peu de précision : les relevés
n'affichent que trois chiffres significatifs.

**Conséquence pratique : l'enveloppe ne grandit que sur une machine libre.** Un
relevé pris pendant qu'on travaille ailleurs sur la même machine est consigné,
daté, comparable — mais il ne touche pas la table. Ce n'est pas un échec, et
`make check-local` ne s'en émeut pas. Pour nourrir l'enveloppe, lancer
`make bench` quand on ne se sert pas de la machine.

**Le seuil lui-même est une heuristique**, et la colonne de charge existe pour
l'affiner. Il a d'ailleurs été abaissé de deux à une et demie le jour où il a été
posé : le relevé de la version `0.3.5`, pris à 1,88 et donc admis, a fixé un
maximum de 853 µs pour l'écriture de 4000 sous-titres là où les dix-sept relevés
précédents allaient de 492 à 641. Ceux pris sous 1,4 n'ont posé que des minima.

Ce fichier est écrit par `make bench` : ne pas l'éditer à la main.

## Extrêmes

Le minimum et le maximum jamais relevés pour chaque mesure. Cette table n'est
jamais élaguée — c'est elle qui garde l'enveloppe quand les relevés s'effacent.
Une mesure renommée ou retirée y garde ses extrêmes indéfiniment : rien ici ne
distingue une entrée vivante d'une orpheline — élaguer les orphelines n'est
pas le sujet de ce ticket.

<!-- extrêmes -->
| Mesure | Minimum | Relevé le | Maximum | Relevé le |
| :----- | ------: | :-------- | ------: | :-------- |
| versionString | 32.3 ns | 0.2.4 — 2026-08-13 | 52.8 ns | 0.2.6 — 2026-08-13 |
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 39.6 ns | 0.2.8 — 2026-08-14 |
| format | 32.6 ns | 0.2.10 — 2026-08-14 | 44 ns | 0.3.3 — 2026-08-15 |
| position vers image | 6.49 ns | 0.2.9 — 2026-08-14 | 8.78 ns | 0.2.3 — 2026-08-12 |
| image vers position | 6.48 ns | 0.2.10 — 2026-08-14 | 10.3 ns | 0.3.4 — 2026-08-15 |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.2.14 — 2026-08-14 | 9.09 ns | 0.2.13 — 2026-08-14 |
| lecture de 4000 sous-titres | 2.2 ms | 0.2.4 — 2026-08-13 | 3.17 ms | 0.2.13 — 2026-08-14 |
| écriture de 4000 sous-titres | 492 µs | 0.3.2 — 2026-08-15 | 641 µs | 0.2.7 — 2026-08-14 |
| décalage de 4000 sous-titres | 9.03 µs | 0.2.10 — 2026-08-14 | 11.4 µs | 0.3.3 — 2026-08-15 |
| décalage puis annulation | 15.3 µs | 0.2.10 — 2026-08-14 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 74.5 µs | 0.2.10 — 2026-08-14 | 93.3 µs | 0.2.12 — 2026-08-14 |
| conversion de fréquence sur 4000 sous-titres | 73.5 µs | 0.3.1 — 2026-08-14 | 100 µs | 0.2.12 — 2026-08-14 |
| tri de 4000 sous-titres à l'envers | 196 µs | 0.3.2 — 2026-08-15 | 274 µs | 0.2.6 — 2026-08-13 |
| suppression d'un sous-titre sur deux | 9.92 ms | 0.2.4 — 2026-08-13 | 12.5 ms | 0.3.3 — 2026-08-15 |
| insertion de 100 sous-titres vides au milieu | 50.3 µs | 0.2.11 — 2026-08-14 | 67 µs | 0.2.13 — 2026-08-14 |
| modification d'un texte, à travers une session | 114 ns | 0.2.14 — 2026-08-14 | 149 ns | 0.2.5 — 2026-08-13 |

<!-- versionString min=32.3 max=52.8 -->
<!-- parse min=29.9 max=39.6 -->
<!-- format min=32.6 max=44.0 -->
<!-- position vers image min=6.49 max=8.78 -->
<!-- image vers position min=6.48 max=10.3 -->
<!-- mise à l'échelle par un rationnel exact min=6.71 max=9.09 -->
<!-- lecture de 4000 sous-titres min=2200000.0 max=3170000.0 -->
<!-- écriture de 4000 sous-titres min=492000.0 max=641000.0 -->
<!-- décalage de 4000 sous-titres min=9030.0 max=11400.0 -->
<!-- décalage puis annulation min=15300.0 max=20700.0 -->
<!-- transformation de 4000 sous-titres min=74500.0 max=93300.0 -->
<!-- conversion de fréquence sur 4000 sous-titres min=73500.0 max=100000.0 -->
<!-- tri de 4000 sous-titres à l'envers min=196000.0 max=274000.0 -->
<!-- suppression d'un sous-titre sur deux min=9920000.0 max=12500000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=50300.0 max=67000.0 -->
<!-- modification d'un texte, à travers une session min=114.0 max=149.0 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

### 0.3.8 — 2026-08-15 — Release — charge 2.23

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 40.8 ns | 4.33 ns |
| parse | 40.7 ns | 7.81 ns |
| format | 40.8 ns | 5.03 ns |
| position vers image | 12.8 ns | 1.38 ns |
| image vers position | 7.97 ns | 0.671 ns |
| mise à l'échelle par un rationnel exact | 8.52 ns | 1.42 ns |
| lecture de 4000 sous-titres | 3.02 ms | 399 µs |
| écriture de 4000 sous-titres | 975 µs | 308 µs |
| décalage de 4000 sous-titres | 54.3 µs | 42.9 µs |
| décalage puis annulation | 40.2 µs | 23.7 µs |
| transformation de 4000 sous-titres | 98 µs | 9.65 µs |
| conversion de fréquence sur 4000 sous-titres | 91 µs | 8.56 µs |
| tri de 4000 sous-titres à l'envers | 325 µs | 126 µs |
| suppression d'un sous-titre sur deux | 12.7 ms | 1.32 ms |
| insertion de 100 sous-titres vides au milieu | 102 µs | 162 µs |
| modification d'un texte, à travers une session | 146 ns | 46.5 ns |

### 0.3.6 — 2026-08-15 — Release — charge 3.36

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 78.3 ns | 7.74 ns |
| parse | 43.8 ns | 3.72 ns |
| format | 38.5 ns | 1.24 ns |
| position vers image | 7.86 ns | 0.2 ns |
| image vers position | 7.76 ns | 1.93 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0727 ns |
| lecture de 4000 sous-titres | 3.84 ms | 957 µs |
| écriture de 4000 sous-titres | 716 µs | 202 µs |
| décalage de 4000 sous-titres | 12.1 µs | 3.41 µs |
| décalage puis annulation | 17.7 µs | 3.58 µs |
| transformation de 4000 sous-titres | 86.7 µs | 11 µs |
| conversion de fréquence sur 4000 sous-titres | 91 µs | 11.9 µs |
| tri de 4000 sous-titres à l'envers | 984 µs | 586 µs |
| suppression d'un sous-titre sur deux | 20.7 ms | 13.1 ms |
| insertion de 100 sous-titres vides au milieu | 96.6 µs | 45.1 µs |
| modification d'un texte, à travers une session | 158 ns | 52.3 ns |

### 0.3.5 — 2026-08-15 — Release — charge 30.05

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 308 ns | 2.17 µs |
| parse | 78.6 ns | 8.91 ns |
| format | 257 ns | 1.63 µs |
| position vers image | 15.5 ns | 3.94 ns |
| image vers position | 74.9 ns | 491 ns |
| mise à l'échelle par un rationnel exact | 18.7 ns | 10.1 ns |
| lecture de 4000 sous-titres | 6.62 ms | 1.47 ms |
| écriture de 4000 sous-titres | 1.46 ms | 407 µs |
| décalage de 4000 sous-titres | 13.5 µs | 2.92 µs |
| décalage puis annulation | 19 µs | 1.77 µs |
| transformation de 4000 sous-titres | 96.4 µs | 17.8 µs |
| conversion de fréquence sur 4000 sous-titres | 96.2 µs | 16.5 µs |
| tri de 4000 sous-titres à l'envers | 246 µs | 20.8 µs |
| suppression d'un sous-titre sur deux | 13.5 ms | 3.05 ms |
| insertion de 100 sous-titres vides au milieu | 62.2 µs | 6.45 µs |
| modification d'un texte, à travers une session | 144 ns | 49.1 ns |

### 0.3.4 — 2026-08-15 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 39 ns | 0.681 ns |
| parse | 37 ns | 2.24 ns |
| format | 38.7 ns | 2.2 ns |
| position vers image | 7.52 ns | 0.388 ns |
| image vers position | 10.3 ns | 2.36 ns |
| mise à l'échelle par un rationnel exact | 7.52 ns | 1.73 ns |
| lecture de 4000 sous-titres | 2.59 ms | 93.9 µs |
| écriture de 4000 sous-titres | 551 µs | 28.8 µs |
| décalage de 4000 sous-titres | 11 µs | 3 µs |
| décalage puis annulation | 16.6 µs | 4.54 µs |
| transformation de 4000 sous-titres | 85.5 µs | 5.75 µs |
| conversion de fréquence sur 4000 sous-titres | 85.3 µs | 5.38 µs |
| tri de 4000 sous-titres à l'envers | 214 µs | 34.9 µs |
| suppression d'un sous-titre sur deux | 10.9 ms | 1 ms |
| insertion de 100 sous-titres vides au milieu | 65.9 µs | 37.6 µs |
| modification d'un texte, à travers une session | 127 ns | 20 ns |

### 0.3.3 — 2026-08-15 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 44.5 ns | 7.76 ns |
| parse | 36.7 ns | 2.79 ns |
| format | 44 ns | 33.4 ns |
| position vers image | 7.83 ns | 0.117 ns |
| image vers position | 7.91 ns | 0.948 ns |
| mise à l'échelle par un rationnel exact | 8.12 ns | 0.205 ns |
| lecture de 4000 sous-titres | 2.92 ms | 275 µs |
| écriture de 4000 sous-titres | 610 µs | 56.1 µs |
| décalage de 4000 sous-titres | 11.4 µs | 1.35 µs |
| décalage puis annulation | 20.1 µs | 2.68 µs |
| transformation de 4000 sous-titres | 92 µs | 11.2 µs |
| conversion de fréquence sur 4000 sous-titres | 92.5 µs | 11.6 µs |
| tri de 4000 sous-titres à l'envers | 243 µs | 33.6 µs |
| suppression d'un sous-titre sur deux | 12.5 ms | 1.98 ms |
| insertion de 100 sous-titres vides au milieu | 61.6 µs | 13.1 µs |
| modification d'un texte, à travers une session | 141 ns | 31.5 ns |

### 0.3.2 — 2026-08-15 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.1 ns | 0.51 ns |
| parse | 30.1 ns | 2.57 ns |
| format | 35.2 ns | 8.57 ns |
| position vers image | 8.56 ns | 3.98 ns |
| image vers position | 6.48 ns | 0.0943 ns |
| mise à l'échelle par un rationnel exact | 8.15 ns | 2.61 ns |
| lecture de 4000 sous-titres | 2.48 ms | 327 µs |
| écriture de 4000 sous-titres | 492 µs | 22.9 µs |
| décalage de 4000 sous-titres | 9.2 µs | 2.08 µs |
| décalage puis annulation | 18.3 µs | 1.81 µs |
| transformation de 4000 sous-titres | 77.6 µs | 8.58 µs |
| conversion de fréquence sur 4000 sous-titres | 75.4 µs | 13.7 µs |
| tri de 4000 sous-titres à l'envers | 196 µs | 27.4 µs |
| suppression d'un sous-titre sur deux | 10.1 ms | 760 µs |
| insertion de 100 sous-titres vides au milieu | 61.5 µs | 12.1 µs |
| modification d'un texte, à travers une session | 118 ns | 34.2 ns |

### 0.3.1 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.1 ns | 0.388 ns |
| parse | 34 ns | 8.59 ns |
| format | 38 ns | 2.81 ns |
| position vers image | 7.87 ns | 0.404 ns |
| image vers position | 6.93 ns | 2.33 ns |
| mise à l'échelle par un rationnel exact | 7.69 ns | 0.0586 ns |
| lecture de 4000 sous-titres | 2.45 ms | 71.1 µs |
| écriture de 4000 sous-titres | 557 µs | 15.1 µs |
| décalage de 4000 sous-titres | 9.29 µs | 1.51 µs |
| décalage puis annulation | 17.9 µs | 3.24 µs |
| transformation de 4000 sous-titres | 75.2 µs | 7.97 µs |
| conversion de fréquence sur 4000 sous-titres | 73.5 µs | 8.38 µs |
| tri de 4000 sous-titres à l'envers | 204 µs | 15.1 µs |
| suppression d'un sous-titre sur deux | 10.4 ms | 331 µs |
| insertion de 100 sous-titres vides au milieu | 54.2 µs | 23.5 µs |
| modification d'un texte, à travers une session | 116 ns | 6.55 ns |

### 0.2.15 — 2026-08-14 — Release — charge 7.2

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 47.5 ns | 1.68 ns |
| parse | 63.9 ns | 14.2 ns |
| format | 53.4 ns | 6.87 ns |
| position vers image | 12.5 ns | 0.638 ns |
| image vers position | 8.87 ns | 1.1 ns |
| mise à l'échelle par un rationnel exact | 8.86 ns | 2.55 ns |
| lecture de 4000 sous-titres | 3.62 ms | 1.01 ms |
| écriture de 4000 sous-titres | 669 µs | 87.5 µs |
| décalage de 4000 sous-titres | 37.7 µs | 26.8 µs |
| décalage puis annulation | 22.9 µs | 3.61 µs |
| transformation de 4000 sous-titres | 117 µs | 24.9 µs |
| conversion de fréquence sur 4000 sous-titres | 105 µs | 12.9 µs |
| tri de 4000 sous-titres à l'envers | 316 µs | 93.6 µs |
| suppression d'un sous-titre sur deux | 16.2 ms | 3.52 ms |
| insertion de 100 sous-titres vides au milieu | 74.2 µs | 18.2 µs |
| modification d'un texte, à travers une session | 172 ns | 45.3 ns |

### 0.2.14 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 42 ns | 1.56 ns |
| parse | 37.6 ns | 8.4 ns |
| format | 40.1 ns | 1.44 ns |
| position vers image | 7.45 ns | 0.0904 ns |
| image vers position | 7.81 ns | 0.0814 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0559 ns |
| lecture de 4000 sous-titres | 2.72 ms | 200 µs |
| écriture de 4000 sous-titres | 623 µs | 72.7 µs |
| décalage de 4000 sous-titres | 10.6 µs | 6.54 µs |
| décalage puis annulation | 17.6 µs | 2.17 µs |
| transformation de 4000 sous-titres | 84.7 µs | 3.15 µs |
| conversion de fréquence sur 4000 sous-titres | 85.5 µs | 11 µs |
| tri de 4000 sous-titres à l'envers | 221 µs | 20.9 µs |
| suppression d'un sous-titre sur deux | 11.4 ms | 1.82 ms |
| insertion de 100 sous-titres vides au milieu | 63.1 µs | 15.7 µs |
| modification d'un texte, à travers une session | 114 ns | 22.8 ns |

### 0.2.13 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 44.3 ns | 0.899 ns |
| parse | 36.1 ns | 2.02 ns |
| format | 42.1 ns | 14.4 ns |
| position vers image | 7.47 ns | 0.23 ns |
| image vers position | 8.45 ns | 2.13 ns |
| mise à l'échelle par un rationnel exact | 9.09 ns | 3.63 ns |
| lecture de 4000 sous-titres | 3.17 ms | 651 µs |
| écriture de 4000 sous-titres | 619 µs | 72.5 µs |
| décalage de 4000 sous-titres | 10.8 µs | 982 ns |
| décalage puis annulation | 18.3 µs | 1.78 µs |
| transformation de 4000 sous-titres | 83.3 µs | 9.57 µs |
| conversion de fréquence sur 4000 sous-titres | 85.6 µs | 11.6 µs |
| tri de 4000 sous-titres à l'envers | 233 µs | 32.1 µs |
| suppression d'un sous-titre sur deux | 11.6 ms | 635 µs |
| insertion de 100 sous-titres vides au milieu | 67 µs | 21.1 µs |
| modification d'un texte, à travers une session | 142 ns | 56.8 ns |

### 0.2.12 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 50.1 ns | 11.5 ns |
| parse | 37.6 ns | 9.1 ns |
| format | 38.3 ns | 1.76 ns |
| position vers image | 7.67 ns | 1.53 ns |
| image vers position | 7.48 ns | 0.318 ns |
| mise à l'échelle par un rationnel exact | 7.73 ns | 0.238 ns |
| lecture de 4000 sous-titres | 3.09 ms | 816 µs |
| écriture de 4000 sous-titres | 576 µs | 27.3 µs |
| décalage de 4000 sous-titres | 10.7 µs | 4.03 µs |
| décalage puis annulation | 18.6 µs | 2.46 µs |
| transformation de 4000 sous-titres | 93.3 µs | 18.5 µs |
| conversion de fréquence sur 4000 sous-titres | 100 µs | 20.8 µs |
| tri de 4000 sous-titres à l'envers | 213 µs | 40 µs |
| suppression d'un sous-titre sur deux | 11 ms | 793 µs |
| insertion de 100 sous-titres vides au milieu | 56.7 µs | 16.4 µs |
| modification d'un texte, à travers une session | 126 ns | 31.2 ns |

### 0.2.11 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 41.8 ns | 1.06 ns |
| parse | 34.5 ns | 2.59 ns |
| format | 37.3 ns | 1.64 ns |
| position vers image | 7.71 ns | 1.49 ns |
| image vers position | 7.43 ns | 0.0749 ns |
| mise à l'échelle par un rationnel exact | 7.62 ns | 3.02 ns |
| lecture de 4000 sous-titres | 2.32 ms | 243 µs |
| écriture de 4000 sous-titres | 550 µs | 24 µs |
| décalage de 4000 sous-titres | 9.08 µs | 3.28 µs |
| décalage puis annulation | 15.5 µs | 3.11 µs |
| transformation de 4000 sous-titres | 85.4 µs | 12.8 µs |
| conversion de fréquence sur 4000 sous-titres | 75.5 µs | 13.1 µs |
| tri de 4000 sous-titres à l'envers | 227 µs | 24.4 µs |
| suppression d'un sous-titre sur deux | 9.94 ms | 620 µs |
| insertion de 100 sous-titres vides au milieu | 50.3 µs | 18.6 µs |
| modification d'un texte, à travers une session | 124 ns | 97.3 ns |

### 0.2.10 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 36.7 ns | 0.498 ns |
| parse | 32.6 ns | 9.13 ns |
| format | 32.6 ns | 1.17 ns |
| position vers image | 6.66 ns | 1.07 ns |
| image vers position | 6.48 ns | 0.0516 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.073 ns |
| lecture de 4000 sous-titres | 2.4 ms | 63.5 µs |
| écriture de 4000 sous-titres | 570 µs | 57.8 µs |
| décalage de 4000 sous-titres | 9.03 µs | 2.67 µs |
| décalage puis annulation | 15.3 µs | 1.69 µs |
| transformation de 4000 sous-titres | 74.5 µs | 9.2 µs |
| conversion de fréquence sur 4000 sous-titres | 74.4 µs | 11.7 µs |
| tri de 4000 sous-titres à l'envers | 212 µs | 41 µs |
| suppression d'un sous-titre sur deux | 10.1 ms | 304 µs |
| insertion de 100 sous-titres vides au milieu | 50.6 µs | 19 µs |
| modification d'un texte, à travers une session | 132 ns | 176 ns |

### 0.2.9 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.1 ns | 0.456 ns |
| parse | 30.1 ns | 3.07 ns |
| format | 42.4 ns | 7.84 ns |
| position vers image | 6.49 ns | 0.0588 ns |
| image vers position | 6.48 ns | 0.0758 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0767 ns |
| lecture de 4000 sous-titres | 2.62 ms | 213 µs |
| écriture de 4000 sous-titres | 505 µs | 38.2 µs |
| décalage de 4000 sous-titres | 10.8 µs | 2.04 µs |
| décalage puis annulation | 19.5 µs | 5.09 µs |
| transformation de 4000 sous-titres | 85 µs | 2.49 µs |
| conversion de fréquence sur 4000 sous-titres | 84.9 µs | 5.75 µs |
| tri de 4000 sous-titres à l'envers | 255 µs | 43.9 µs |
| suppression d'un sous-titre sur deux | 11.7 ms | 1.73 ms |
| insertion de 100 sous-titres vides au milieu | 65.3 µs | 21.5 µs |
| modification d'un texte, à travers une session | 127 ns | 16.8 ns |

### 0.2.8 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 32.3 ns | 0.342 ns |
| parse | 39.6 ns | 19.6 ns |
| format | 38.4 ns | 0.62 ns |
| position vers image | 7.53 ns | 0.19 ns |
| image vers position | 6.64 ns | 1.57 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0667 ns |
| lecture de 4000 sous-titres | 2.5 ms | 204 µs |
| écriture de 4000 sous-titres | 559 µs | 21.2 µs |
| décalage de 4000 sous-titres | 11 µs | 5.31 µs |
| décalage puis annulation | 18.8 µs | 2.84 µs |
| transformation de 4000 sous-titres | 86.8 µs | 9.56 µs |
| conversion de fréquence sur 4000 sous-titres | 87.4 µs | 19.3 µs |
| tri de 4000 sous-titres à l'envers | 229 µs | 21.4 µs |
| suppression d'un sous-titre sur deux | 10.3 ms | 558 µs |
| insertion de 100 sous-titres vides au milieu | 54.3 µs | 20.3 µs |
| modification d'un texte, à travers une session | 127 ns | 26.1 ns |

### 0.2.7 — 2026-08-14 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 39.3 ns | 1.88 ns |
| parse | 36 ns | 2.27 ns |
| format | 32.6 ns | 6.04 ns |
| position vers image | 7.86 ns | 0.354 ns |
| image vers position | 7.43 ns | 0.0622 ns |
| mise à l'échelle par un rationnel exact | 8.87 ns | 1.76 ns |
| lecture de 4000 sous-titres | 3.08 ms | 1.03 ms |
| écriture de 4000 sous-titres | 641 µs | 141 µs |
| décalage de 4000 sous-titres | 11.2 µs | 3.93 µs |
| décalage puis annulation | 18 µs | 3.1 µs |
| transformation de 4000 sous-titres | 87.1 µs | 14.8 µs |
| conversion de fréquence sur 4000 sous-titres | 89.4 µs | 12.6 µs |
| tri de 4000 sous-titres à l'envers | 235 µs | 49.4 µs |
| suppression d'un sous-titre sur deux | 10 ms | 878 µs |
| insertion de 100 sous-titres vides au milieu | 55.8 µs | 62.7 µs |
| modification d'un texte, à travers une session | 136 ns | 20.7 ns |

### 0.2.6 — 2026-08-13 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 52.8 ns | 19.9 ns |
| parse | 29.9 ns | 2.69 ns |
| format | 41.3 ns | 1.16 ns |
| position vers image | 7.72 ns | 2.52 ns |
| image vers position | 7.64 ns | 1.23 ns |
| mise à l'échelle par un rationnel exact | 7.73 ns | 0.209 ns |
| lecture de 4000 sous-titres | 2.47 ms | 156 µs |
| écriture de 4000 sous-titres | 595 µs | 63.3 µs |
| décalage de 4000 sous-titres | 11 µs | 2.55 µs |
| décalage puis annulation | 20.7 µs | 6.46 µs |
| transformation de 4000 sous-titres | 82 µs | 9.29 µs |
| conversion de fréquence sur 4000 sous-titres | 86.4 µs | 10 µs |
| tri de 4000 sous-titres à l'envers | 274 µs | 67.7 µs |
| suppression d'un sous-titre sur deux | 10.9 ms | 342 µs |
| insertion de 100 sous-titres vides au milieu | 60.7 µs | 20.8 µs |
| modification d'un texte, à travers une session | 134 ns | 25.5 ns |

### 0.2.5 — 2026-08-13 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.1 ns | 0.583 ns |
| parse | 34.5 ns | 3.31 ns |
| format | 39.3 ns | 0.505 ns |
| position vers image | 7.62 ns | 1.69 ns |
| image vers position | 7.83 ns | 0.2 ns |
| mise à l'échelle par un rationnel exact | 8.12 ns | 0.212 ns |
| lecture de 4000 sous-titres | 2.45 ms | 134 µs |
| écriture de 4000 sous-titres | 560 µs | 18.3 µs |
| décalage de 4000 sous-titres | 9.75 µs | 1.14 µs |
| décalage puis annulation | 15.3 µs | 1.21 µs |
| transformation de 4000 sous-titres | 81.5 µs | 6.74 µs |
| conversion de fréquence sur 4000 sous-titres | 80.3 µs | 2.88 µs |
| tri de 4000 sous-titres à l'envers | 220 µs | 19.2 µs |
| suppression d'un sous-titre sur deux | 11.4 ms | 1.4 ms |
| insertion de 100 sous-titres vides au milieu | 56.1 µs | 18.8 µs |
| modification d'un texte, à travers une session | 149 ns | 30.6 ns |

### 0.2.4 — 2026-08-13 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 32.3 ns | 0.409 ns |
| parse | 34.2 ns | 3.03 ns |
| format | 34.4 ns | 1.06 ns |
| position vers image | 6.61 ns | 0.18 ns |
| image vers position | 6.48 ns | 0.0499 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.081 ns |
| lecture de 4000 sous-titres | 2.2 ms | 93.4 µs |
| écriture de 4000 sous-titres | 510 µs | 31 µs |
| décalage de 4000 sous-titres | 10.2 µs | 4.2 µs |
| décalage puis annulation | 19 µs | 4.38 µs |
| transformation de 4000 sous-titres | 86.1 µs | 8.37 µs |
| conversion de fréquence sur 4000 sous-titres | 75.6 µs | 5.21 µs |
| tri de 4000 sous-titres à l'envers | 207 µs | 25.9 µs |
| suppression d'un sous-titre sur deux | 9.92 ms | 247 µs |
| insertion de 100 sous-titres vides au milieu | 53.2 µs | 14.8 µs |
| modification d'un texte, à travers une session | 117 ns | 21.6 ns |

### 0.2.3 — 2026-08-12 — Release

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.1 ns | 0.505 ns |
| parse | 37.3 ns | 3.53 ns |
| format | 40.6 ns | 1.63 ns |
| position vers image | 8.78 ns | 1.86 ns |
| image vers position | 7.82 ns | 0.143 ns |
| mise à l'échelle par un rationnel exact | 7.93 ns | 1.55 ns |
| lecture de 4000 sous-titres | 2.5 ms | 136 µs |
| écriture de 4000 sous-titres | 618 µs | 98 µs |
| décalage de 4000 sous-titres | 10.8 µs | 3.2 µs |
| décalage puis annulation | 18.5 µs | 2.24 µs |
| transformation de 4000 sous-titres | 82.6 µs | 6.28 µs |
| conversion de fréquence sur 4000 sous-titres | 89.5 µs | 13.7 µs |
| tri de 4000 sous-titres à l'envers | 258 µs | 58.1 µs |
| suppression d'un sous-titre sur deux | 12.5 ms | 783 µs |
| insertion de 100 sous-titres vides au milieu | 64.9 µs | 27.2 µs |
| modification d'un texte, à travers une session | 138 ns | 35.3 ns |
