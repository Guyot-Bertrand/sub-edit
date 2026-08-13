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
| versionString | 32.3 ns | 0.2.4 — 2026-08-13 | 44.6 ns | 0.2.2 — 2026-08-12 |
| parse | 34.1 ns | 0.2.2 — 2026-08-12 | 37.3 ns | 0.2.3 — 2026-08-12 |
| format | 34.4 ns | 0.2.4 — 2026-08-13 | 44 ns | 0.2.2 — 2026-08-12 |
| position vers image | 6.61 ns | 0.2.4 — 2026-08-13 | 8.78 ns | 0.2.3 — 2026-08-12 |
| image vers position | 6.48 ns | 0.2.4 — 2026-08-13 | 8.79 ns | 0.2.2 — 2026-08-12 |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.2.4 — 2026-08-13 | 9.07 ns | 0.2.2 — 2026-08-12 |
| lecture de 4000 sous-titres | 2.2 ms | 0.2.4 — 2026-08-13 | 2.52 ms | 0.2.2 — 2026-08-12 |
| écriture de 4000 sous-titres | 510 µs | 0.2.4 — 2026-08-13 | 618 µs | 0.2.3 — 2026-08-12 |
| décalage de 4000 sous-titres | 9.83 µs | 0.2.2 — 2026-08-12 | 10.8 µs | 0.2.2 — 2026-08-12 |
| décalage puis annulation | 17.1 µs | 0.2.2 — 2026-08-12 | 19.1 µs | 0.2.2 — 2026-08-12 |
| transformation de 4000 sous-titres | 78.3 µs | 0.2.2 — 2026-08-12 | 86.1 µs | 0.2.4 — 2026-08-13 |
| conversion de fréquence sur 4000 sous-titres | 75.6 µs | 0.2.4 — 2026-08-13 | 89.5 µs | 0.2.3 — 2026-08-12 |
| tri de 4000 sous-titres à l'envers | 207 µs | 0.2.4 — 2026-08-13 | 258 µs | 0.2.3 — 2026-08-12 |
| suppression d'un sous-titre sur deux | 9.92 ms | 0.2.4 — 2026-08-13 | 12.5 ms | 0.2.3 — 2026-08-12 |
| insertion de 100 sous-titres vides au milieu | 53.2 µs | 0.2.4 — 2026-08-13 | 65.8 µs | 0.2.2 — 2026-08-12 |
| modification d'un texte, à travers une session | 117 ns | 0.2.4 — 2026-08-13 | 161 ns | 0.2.2 — 2026-08-12 |

<!-- versionString min=32.3214 max=44.575 -->
<!-- parse min=34.0756 max=37.3426 -->
<!-- format min=34.3724 max=43.9743 -->
<!-- position vers image min=6.61311 max=8.77599 -->
<!-- image vers position min=6.47742 max=8.79029 -->
<!-- mise à l'échelle par un rationnel exact min=6.71826 max=9.06914 -->
<!-- lecture de 4000 sous-titres min=2198390.0 max=2517570.0 -->
<!-- écriture de 4000 sous-titres min=509544.0 max=618286.0 -->
<!-- décalage de 4000 sous-titres min=9828.04 max=10838.2 -->
<!-- décalage puis annulation min=17138.5 max=19113.1 -->
<!-- transformation de 4000 sous-titres min=78316.4 max=86133.4 -->
<!-- conversion de fréquence sur 4000 sous-titres min=75577.7 max=89464.1 -->
<!-- tri de 4000 sous-titres à l'envers min=207024.0 max=257593.0 -->
<!-- suppression d'un sous-titre sur deux min=9921060.0 max=12451000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=53231.0 max=65814.7 -->
<!-- modification d'un texte, à travers une session min=116.874 max=161.356 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

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
