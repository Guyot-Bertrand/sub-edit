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
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 37.3 ns | 0.2.3 — 2026-08-12 |
| format | 32.6 ns | 0.2.7 — 2026-08-14 | 44 ns | 0.2.2 — 2026-08-12 |
| position vers image | 6.61 ns | 0.2.4 — 2026-08-13 | 8.78 ns | 0.2.3 — 2026-08-12 |
| image vers position | 6.48 ns | 0.2.4 — 2026-08-13 | 8.79 ns | 0.2.2 — 2026-08-12 |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.2.4 — 2026-08-13 | 9.07 ns | 0.2.2 — 2026-08-12 |
| lecture de 4000 sous-titres | 2.2 ms | 0.2.4 — 2026-08-13 | 3.08 ms | 0.2.7 — 2026-08-14 |
| écriture de 4000 sous-titres | 510 µs | 0.2.4 — 2026-08-13 | 641 µs | 0.2.7 — 2026-08-14 |
| décalage de 4000 sous-titres | 9.75 µs | 0.2.5 — 2026-08-13 | 11.2 µs | 0.2.7 — 2026-08-14 |
| décalage puis annulation | 15.3 µs | 0.2.5 — 2026-08-13 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 78.3 µs | 0.2.2 — 2026-08-12 | 87.1 µs | 0.2.7 — 2026-08-14 |
| conversion de fréquence sur 4000 sous-titres | 75.6 µs | 0.2.4 — 2026-08-13 | 89.5 µs | 0.2.3 — 2026-08-12 |
| tri de 4000 sous-titres à l'envers | 207 µs | 0.2.4 — 2026-08-13 | 274 µs | 0.2.6 — 2026-08-13 |
| suppression d'un sous-titre sur deux | 9.92 ms | 0.2.4 — 2026-08-13 | 12.5 ms | 0.2.3 — 2026-08-12 |
| insertion de 100 sous-titres vides au milieu | 53.2 µs | 0.2.4 — 2026-08-13 | 65.8 µs | 0.2.2 — 2026-08-12 |
| modification d'un texte, à travers une session | 117 ns | 0.2.4 — 2026-08-13 | 161 ns | 0.2.2 — 2026-08-12 |

<!-- versionString min=32.3214 max=52.7577 -->
<!-- parse min=29.9043 max=37.3426 -->
<!-- format min=32.5647 max=43.9743 -->
<!-- position vers image min=6.61311 max=8.77599 -->
<!-- image vers position min=6.47742 max=8.79029 -->
<!-- mise à l'échelle par un rationnel exact min=6.71826 max=9.06914 -->
<!-- lecture de 4000 sous-titres min=2198390.0 max=3078630.0 -->
<!-- écriture de 4000 sous-titres min=509544.0 max=640976.0 -->
<!-- décalage de 4000 sous-titres min=9753.23 max=11244.6 -->
<!-- décalage puis annulation min=15303.0 max=20725.3 -->
<!-- transformation de 4000 sous-titres min=78316.4 max=87141.3 -->
<!-- conversion de fréquence sur 4000 sous-titres min=75577.7 max=89464.1 -->
<!-- tri de 4000 sous-titres à l'envers min=207024.0 max=273761.0 -->
<!-- suppression d'un sous-titre sur deux min=9921060.0 max=12451000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=53231.0 max=65814.7 -->
<!-- modification d'un texte, à travers une session min=116.874 max=161.356 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

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
