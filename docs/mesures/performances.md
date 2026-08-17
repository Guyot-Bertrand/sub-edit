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

Ce fichier est écrit par `make bench` : ne pas l'éditer à la main — sauf cette
préface, que le script recopie telle quelle.

## Le texte de la fixture d'édition a changé une fois, en 0.3.9

Les mesures de `core/edit` portent sur un document engendré de quatre mille
sous-titres. **Ses positions n'ont jamais bougé et ne bougeront pas** — c'est ce
qui rend chaque chiffre comparable à son propre passé. Son **texte**, lui, a
changé une fois : jusqu'à la version `0.3.8` les quatre mille répliques étaient
la même chaîne de cinquante-trois octets ; depuis la `0.3.9`, le texte varie et
un sous-titre sur cinq porte une mention pour malentendants, dans les
proportions que donnent de vrais fichiers. Le pourquoi et les chiffres sont dans
`src/test/bench/full_length_project.hpp` ; ce qui compte ici est que le document
pèse désormais 122 Ko de texte au lieu de 212 Ko.

**L'effet sur les mesures qui ne portent pas sur le texte est sous le bruit.**
Les deux fixtures ont été mesurées le même jour, à une heure d'intervalle, sur
une machine à 0,6 de charge : la lecture de 4000 sous-titres, dont la fixture n'a
pas bougé d'un octet, s'est écartée de 14 % entre les deux exécutions — davantage
que toute mesure d'édition. Le décalage, mesuré à 11,3 puis 8,67 µs, est revenu à
9,01 au relevé de la `0.3.9` : à 0,02 µs de son minimum historique.

**Une mesure fait exception, et c'est la seule qui recopie du texte.** La
suppression d'un sous-titre sur deux garde ce qu'elle retire, pour pouvoir
l'annuler : `Project::remove` copie chacun des deux mille sous-titres ôtés, donc
leurs chaînes. Deux fois moins d'octets à copier se voient — son minimum est
passé de 9,92 à 8,95 ms. Ce n'est pas une amélioration du code, c'est une fixture
plus légère : pour cette mesure, les relevés antérieurs à la `0.3.9` et les
suivants ne se comparent qu'à un dixième près.

Rien n'a été élagué de la table pour autant : l'enveloppe d'avant reste vraie de
ce qu'elle mesurait.

## Extrêmes

Le minimum et le maximum jamais relevés pour chaque mesure. Cette table n'est
jamais élaguée — c'est elle qui garde l'enveloppe quand les relevés s'effacent.
Une mesure renommée ou retirée y garde ses extrêmes indéfiniment : rien ici ne
distingue une entrée vivante d'une orpheline — élaguer les orphelines n'est
pas le sujet de ce ticket.

<!-- extrêmes -->
| Mesure | Minimum | Relevé le | Maximum | Relevé le |
| :----- | ------: | :-------- | ------: | :-------- |
| versionString | 32.3 ns | 0.2.4 — 2026-08-13 | 55.1 ns | 0.4.4 — 2026-08-17 |
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 41.2 ns | 0.4.4 — 2026-08-17 |
| format | 29.8 ns | 0.3.11 — 2026-08-15 | 46.2 ns | 0.4.0 — 2026-08-16 |
| position vers image | 6.48 ns | 0.3.11 — 2026-08-15 | 8.78 ns | 0.2.3 — 2026-08-12 |
| image vers position | 6.48 ns | 0.4.5 — 2026-08-17 | 12.1 ns | 0.3.9 — 2026-08-15 |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.2.14 — 2026-08-14 | 9.09 ns | 0.2.13 — 2026-08-14 |
| lecture de 4000 sous-titres | 2.17 ms | 0.3.9 — 2026-08-15 | 3.17 ms | 0.2.13 — 2026-08-14 |
| écriture de 4000 sous-titres | 488 µs | 0.3.10 — 2026-08-15 | 669 µs | 0.4.0 — 2026-08-16 |
| décalage de 4000 sous-titres | 8.3 µs | 0.3.13 — 2026-08-16 | 11.4 µs | 0.3.3 — 2026-08-15 |
| décalage puis annulation | 14.9 µs | 0.3.9 — 2026-08-15 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 71.7 µs | 0.3.13 — 2026-08-16 | 93.3 µs | 0.2.12 — 2026-08-14 |
| conversion de fréquence sur 4000 sous-titres | 72.4 µs | 0.3.14 — 2026-08-16 | 100 µs | 0.2.12 — 2026-08-14 |
| tri de 4000 sous-titres à l'envers | 196 µs | 0.3.2 — 2026-08-15 | 274 µs | 0.2.6 — 2026-08-13 |
| suppression d'un sous-titre sur deux | 8.3 ms | 0.4.3 — 2026-08-17 | 12.5 ms | 0.3.3 — 2026-08-15 |
| insertion de 100 sous-titres vides au milieu | 48.9 µs | 0.3.13 — 2026-08-16 | 67 µs | 0.2.13 — 2026-08-14 |
| modification d'un texte, à travers une session | 114 ns | 0.2.14 — 2026-08-14 | 180 ns | 0.3.9 — 2026-08-15 |
| suppression des mentions sur 4000 sous-titres | 4.93 ms | 0.3.15 — 2026-08-16 | 5.68 ms | 0.4.4 — 2026-08-17 |

<!-- versionString min=32.3 max=55.096 -->
<!-- parse min=29.9 max=41.1824 -->
<!-- format min=29.8143 max=46.2179 -->
<!-- position vers image min=6.48462 max=8.78 -->
<!-- image vers position min=6.47637 max=12.0852 -->
<!-- mise à l'échelle par un rationnel exact min=6.71 max=9.09 -->
<!-- lecture de 4000 sous-titres min=2165410.0 max=3170000.0 -->
<!-- écriture de 4000 sous-titres min=488279.0 max=668622.0 -->
<!-- décalage de 4000 sous-titres min=8303.85 max=11400.0 -->
<!-- décalage puis annulation min=14851.9 max=20700.0 -->
<!-- transformation de 4000 sous-titres min=71737.0 max=93300.0 -->
<!-- conversion de fréquence sur 4000 sous-titres min=72416.4 max=100000.0 -->
<!-- tri de 4000 sous-titres à l'envers min=196000.0 max=274000.0 -->
<!-- suppression d'un sous-titre sur deux min=8295920.0 max=12500000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=48927.4 max=67000.0 -->
<!-- modification d'un texte, à travers une session min=114.0 max=179.799 -->
<!-- suppression des mentions sur 4000 sous-titres min=4926150.0 max=5676890.0 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

### 0.4.5 — 2026-08-17 — Release — charge 1.42

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 32.3 ns | 0.329 ns |
| parse | 37.4 ns | 2.54 ns |
| format | 31.9 ns | 5.16 ns |
| position vers image | 6.71 ns | 1 ns |
| image vers position | 6.48 ns | 0.0533 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0926 ns |
| lecture de 4000 sous-titres | 2.18 ms | 99.8 µs |
| écriture de 4000 sous-titres | 554 µs | 29.9 µs |
| décalage de 4000 sous-titres | 10.2 µs | 1.49 µs |
| décalage puis annulation | 16.4 µs | 1.8 µs |
| transformation de 4000 sous-titres | 75.3 µs | 6.18 µs |
| conversion de fréquence sur 4000 sous-titres | 74.5 µs | 2.53 µs |
| tri de 4000 sous-titres à l'envers | 221 µs | 23.3 µs |
| suppression d'un sous-titre sur deux | 8.65 ms | 502 µs |
| insertion de 100 sous-titres vides au milieu | 52.3 µs | 16.3 µs |
| modification d'un texte, à travers une session | 149 ns | 16.9 ns |
| suppression des mentions sur 4000 sous-titres | 5.22 ms | 331 µs |

### 0.4.4 — 2026-08-17 — Release — charge 1.44

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 55.1 ns | 4.03 ns |
| parse | 41.2 ns | 3.45 ns |
| format | 39.5 ns | 9.72 ns |
| position vers image | 7.65 ns | 1.38 ns |
| image vers position | 7.81 ns | 0.105 ns |
| mise à l'échelle par un rationnel exact | 8.69 ns | 2.62 ns |
| lecture de 4000 sous-titres | 2.59 ms | 130 µs |
| écriture de 4000 sous-titres | 575 µs | 68.1 µs |
| décalage de 4000 sous-titres | 9.95 µs | 1.15 µs |
| décalage puis annulation | 18.3 µs | 2.01 µs |
| transformation de 4000 sous-titres | 88.4 µs | 12 µs |
| conversion de fréquence sur 4000 sous-titres | 85.9 µs | 7.31 µs |
| tri de 4000 sous-titres à l'envers | 259 µs | 40.1 µs |
| suppression d'un sous-titre sur deux | 9.96 ms | 387 µs |
| insertion de 100 sous-titres vides au milieu | 59.4 µs | 20.8 µs |
| modification d'un texte, à travers une session | 151 ns | 20.3 ns |
| suppression des mentions sur 4000 sous-titres | 5.68 ms | 196 µs |

### 0.4.3 — 2026-08-17 — Release — charge 1.38

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 32.8 ns | 4.43 ns |
| parse | 34.2 ns | 4.77 ns |
| format | 33.9 ns | 1.23 ns |
| position vers image | 6.59 ns | 0.106 ns |
| image vers position | 6.48 ns | 0.0715 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0733 ns |
| lecture de 4000 sous-titres | 2.45 ms | 328 µs |
| écriture de 4000 sous-titres | 499 µs | 42.6 µs |
| décalage de 4000 sous-titres | 8.34 µs | 1.78 µs |
| décalage puis annulation | 17.7 µs | 1.38 µs |
| transformation de 4000 sous-titres | 71.8 µs | 5.64 µs |
| conversion de fréquence sur 4000 sous-titres | 77.5 µs | 17.6 µs |
| tri de 4000 sous-titres à l'envers | 209 µs | 18.6 µs |
| suppression d'un sous-titre sur deux | 8.3 ms | 437 µs |
| insertion de 100 sous-titres vides au milieu | 55.7 µs | 14.9 µs |
| modification d'un texte, à travers une session | 122 ns | 12.8 ns |
| suppression des mentions sur 4000 sous-titres | 5.28 ms | 490 µs |

### 0.4.2 — 2026-08-17 — Release — charge 6.61

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 85.4 ns | 100 ns |
| parse | 90.9 ns | 14.9 ns |
| format | 79.8 ns | 10.2 ns |
| position vers image | 18.9 ns | 30.9 ns |
| image vers position | 15.9 ns | 6.75 ns |
| mise à l'échelle par un rationnel exact | 15.2 ns | 1.75 ns |
| lecture de 4000 sous-titres | 8.56 ms | 2.8 ms |
| écriture de 4000 sous-titres | 836 µs | 211 µs |
| décalage de 4000 sous-titres | 9.65 µs | 2.99 µs |
| décalage puis annulation | 18.7 µs | 2.23 µs |
| transformation de 4000 sous-titres | 81 µs | 3.38 µs |
| conversion de fréquence sur 4000 sous-titres | 123 µs | 22.8 µs |
| tri de 4000 sous-titres à l'envers | 432 µs | 365 µs |
| suppression d'un sous-titre sur deux | 10.8 ms | 1.17 ms |
| insertion de 100 sous-titres vides au milieu | 115 µs | 148 µs |
| modification d'un texte, à travers une session | 134 ns | 20.4 ns |
| suppression des mentions sur 4000 sous-titres | 6.02 ms | 840 µs |

### 0.4.1 — 2026-08-16 — Release — charge 2.65

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37 ns | 0.552 ns |
| parse | 37.1 ns | 1.97 ns |
| format | 38.6 ns | 1.39 ns |
| position vers image | 7.51 ns | 0.139 ns |
| image vers position | 7.85 ns | 1.14 ns |
| mise à l'échelle par un rationnel exact | 7.71 ns | 0.135 ns |
| lecture de 4000 sous-titres | 2.68 ms | 329 µs |
| écriture de 4000 sous-titres | 550 µs | 20.1 µs |
| décalage de 4000 sous-titres | 10.4 µs | 1.77 µs |
| décalage puis annulation | 19.4 µs | 3.08 µs |
| transformation de 4000 sous-titres | 87.1 µs | 7.82 µs |
| conversion de fréquence sur 4000 sous-titres | 85.4 µs | 5.52 µs |
| tri de 4000 sous-titres à l'envers | 245 µs | 18.8 µs |
| suppression d'un sous-titre sur deux | 9.86 ms | 157 µs |
| insertion de 100 sous-titres vides au milieu | 59.1 µs | 13.3 µs |
| modification d'un texte, à travers une session | 148 ns | 20.7 ns |
| suppression des mentions sur 4000 sous-titres | 5.78 ms | 300 µs |

### 0.4.0 — 2026-08-16 — Release — charge 0.99

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.6 ns | 4.5 ns |
| parse | 39.3 ns | 2.2 ns |
| format | 46.2 ns | 22.3 ns |
| position vers image | 6.56 ns | 0.0946 ns |
| image vers position | 7.43 ns | 0.0807 ns |
| mise à l'échelle par un rationnel exact | 7.98 ns | 1.04 ns |
| lecture de 4000 sous-titres | 2.6 ms | 241 µs |
| écriture de 4000 sous-titres | 669 µs | 154 µs |
| décalage de 4000 sous-titres | 9.37 µs | 1.07 µs |
| décalage puis annulation | 15.3 µs | 1.84 µs |
| transformation de 4000 sous-titres | 82.4 µs | 9.1 µs |
| conversion de fréquence sur 4000 sous-titres | 74.1 µs | 13.8 µs |
| tri de 4000 sous-titres à l'envers | 239 µs | 26.7 µs |
| suppression d'un sous-titre sur deux | 9.12 ms | 716 µs |
| insertion de 100 sous-titres vides au milieu | 54.8 µs | 16.1 µs |
| modification d'un texte, à travers une session | 127 ns | 21.8 ns |
| suppression des mentions sur 4000 sous-titres | 5.07 ms | 248 µs |

### 0.3.15 — 2026-08-16 — Release — charge 1.44

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 36.4 ns | 0.349 ns |
| parse | 36.5 ns | 10.4 ns |
| format | 33.8 ns | 0.73 ns |
| position vers image | 6.81 ns | 1.25 ns |
| image vers position | 7.05 ns | 2.16 ns |
| mise à l'échelle par un rationnel exact | 7.06 ns | 2.03 ns |
| lecture de 4000 sous-titres | 2.19 ms | 76.8 µs |
| écriture de 4000 sous-titres | 496 µs | 31.1 µs |
| décalage de 4000 sous-titres | 10.7 µs | 5.41 µs |
| décalage puis annulation | 15.6 µs | 1.72 µs |
| transformation de 4000 sous-titres | 73.9 µs | 10 µs |
| conversion de fréquence sur 4000 sous-titres | 76.2 µs | 13 µs |
| tri de 4000 sous-titres à l'envers | 211 µs | 31.8 µs |
| suppression d'un sous-titre sur deux | 8.48 ms | 236 µs |
| insertion de 100 sous-titres vides au milieu | 49.5 µs | 18.4 µs |
| modification d'un texte, à travers une session | 125 ns | 12.5 ns |
| suppression des mentions sur 4000 sous-titres | 4.93 ms | 195 µs |

### 0.3.14 — 2026-08-16 — Release — charge 1.42

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 41.9 ns | 1.37 ns |
| parse | 32.4 ns | 3.23 ns |
| format | 39.5 ns | 1.88 ns |
| position vers image | 7.6 ns | 0.141 ns |
| image vers position | 7.49 ns | 0.477 ns |
| mise à l'échelle par un rationnel exact | 7.74 ns | 0.266 ns |
| lecture de 4000 sous-titres | 2.66 ms | 370 µs |
| écriture de 4000 sous-titres | 573 µs | 13.9 µs |
| décalage de 4000 sous-titres | 9.25 µs | 1.42 µs |
| décalage puis annulation | 17.8 µs | 2.95 µs |
| transformation de 4000 sous-titres | 81.7 µs | 4.87 µs |
| conversion de fréquence sur 4000 sous-titres | 72.4 µs | 9.63 µs |
| tri de 4000 sous-titres à l'envers | 237 µs | 16.6 µs |
| suppression d'un sous-titre sur deux | 9.65 ms | 761 µs |
| insertion de 100 sous-titres vides au milieu | 55.6 µs | 26 µs |
| modification d'un texte, à travers une session | 132 ns | 27.8 ns |
| suppression des mentions sur 4000 sous-titres | 5.17 ms | 366 µs |

### 0.3.13 — 2026-08-16 — Release — charge 1.49

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 41.6 ns | 0.601 ns |
| parse | 32.3 ns | 2.27 ns |
| format | 33.2 ns | 0.959 ns |
| position vers image | 7.86 ns | 1.84 ns |
| image vers position | 6.48 ns | 0.0606 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0635 ns |
| lecture de 4000 sous-titres | 2.27 ms | 145 µs |
| écriture de 4000 sous-titres | 507 µs | 37.5 µs |
| décalage de 4000 sous-titres | 8.3 µs | 1.83 µs |
| décalage puis annulation | 15.5 µs | 954 ns |
| transformation de 4000 sous-titres | 71.7 µs | 4.63 µs |
| conversion de fréquence sur 4000 sous-titres | 81.3 µs | 5.22 µs |
| tri de 4000 sous-titres à l'envers | 238 µs | 15.9 µs |
| suppression d'un sous-titre sur deux | 9.41 ms | 807 µs |
| insertion de 100 sous-titres vides au milieu | 48.9 µs | 9.6 µs |
| modification d'un texte, à travers une session | 149 ns | 29.8 ns |
| suppression des mentions sur 4000 sous-titres | 5.28 ms | 334 µs |

### 0.3.12 — 2026-08-15 — Release — charge 1.13

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 42 ns | 0.558 ns |
| parse | 37.1 ns | 2.54 ns |
| format | 36 ns | 1.42 ns |
| position vers image | 7.44 ns | 0.0822 ns |
| image vers position | 7.44 ns | 0.0866 ns |
| mise à l'échelle par un rationnel exact | 8.09 ns | 0.0939 ns |
| lecture de 4000 sous-titres | 2.51 ms | 390 µs |
| écriture de 4000 sous-titres | 556 µs | 33.3 µs |
| décalage de 4000 sous-titres | 9.89 µs | 1.71 µs |
| décalage puis annulation | 16.9 µs | 1.74 µs |
| transformation de 4000 sous-titres | 80.8 µs | 2.87 µs |
| conversion de fréquence sur 4000 sous-titres | 73 µs | 10.7 µs |
| tri de 4000 sous-titres à l'envers | 234 µs | 17.1 µs |
| suppression d'un sous-titre sur deux | 9.29 ms | 264 µs |
| insertion de 100 sous-titres vides au milieu | 54.7 µs | 13.2 µs |
| modification d'un texte, à travers une session | 137 ns | 8.36 ns |

### 0.3.11 — 2026-08-15 — Release — charge 1.42

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 36.5 ns | 0.387 ns |
| parse | 34.1 ns | 9.09 ns |
| format | 29.8 ns | 0.793 ns |
| position vers image | 6.48 ns | 0.0491 ns |
| image vers position | 7.44 ns | 0.0887 ns |
| mise à l'échelle par un rationnel exact | 7.06 ns | 0.0819 ns |
| lecture de 4000 sous-titres | 2.33 ms | 111 µs |
| écriture de 4000 sous-titres | 584 µs | 25.5 µs |
| décalage de 4000 sous-titres | 9.76 µs | 9.4 µs |
| décalage puis annulation | 15.9 µs | 5.16 µs |
| transformation de 4000 sous-titres | 78.6 µs | 9.5 µs |
| conversion de fréquence sur 4000 sous-titres | 72.6 µs | 8.28 µs |
| tri de 4000 sous-titres à l'envers | 214 µs | 18.9 µs |
| suppression d'un sous-titre sur deux | 8.84 ms | 218 µs |
| insertion de 100 sous-titres vides au milieu | 49.6 µs | 19.3 µs |
| modification d'un texte, à travers une session | 122 ns | 5.74 ns |

### 0.3.10 — 2026-08-15 — Release — charge 1.48

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 36.3 ns | 0.566 ns |
| parse | 37.3 ns | 3.8 ns |
| format | 33.7 ns | 6.4 ns |
| position vers image | 6.48 ns | 0.0559 ns |
| image vers position | 6.87 ns | 1.86 ns |
| mise à l'échelle par un rationnel exact | 7.05 ns | 0.0534 ns |
| lecture de 4000 sous-titres | 2.36 ms | 215 µs |
| écriture de 4000 sous-titres | 488 µs | 35 µs |
| décalage de 4000 sous-titres | 10.7 µs | 3.23 µs |
| décalage puis annulation | 18.8 µs | 3.14 µs |
| transformation de 4000 sous-titres | 91.5 µs | 8.06 µs |
| conversion de fréquence sur 4000 sous-titres | 74.2 µs | 3.22 µs |
| tri de 4000 sous-titres à l'envers | 213 µs | 20.4 µs |
| suppression d'un sous-titre sur deux | 9.41 ms | 532 µs |
| insertion de 100 sous-titres vides au milieu | 51.7 µs | 20.3 µs |
| modification d'un texte, à travers une session | 124 ns | 20.4 ns |

### 0.3.9 — 2026-08-15 — Release — charge 1.26

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 32.4 ns | 0.358 ns |
| parse | 32.3 ns | 2.62 ns |
| format | 32.4 ns | 6.36 ns |
| position vers image | 6.49 ns | 0.0943 ns |
| image vers position | 12.1 ns | 0.572 ns |
| mise à l'échelle par un rationnel exact | 7.06 ns | 0.0668 ns |
| lecture de 4000 sous-titres | 2.17 ms | 61.2 µs |
| écriture de 4000 sous-titres | 507 µs | 83.4 µs |
| décalage de 4000 sous-titres | 9.01 µs | 2.4 µs |
| décalage puis annulation | 14.9 µs | 1.87 µs |
| transformation de 4000 sous-titres | 72.8 µs | 3.91 µs |
| conversion de fréquence sur 4000 sous-titres | 73 µs | 5.07 µs |
| tri de 4000 sous-titres à l'envers | 215 µs | 21.5 µs |
| suppression d'un sous-titre sur deux | 8.95 ms | 264 µs |
| insertion de 100 sous-titres vides au milieu | 50.3 µs | 20.1 µs |
| modification d'un texte, à travers une session | 180 ns | 242 ns |

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
