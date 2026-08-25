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

**Cela s'était reproduit pour une mesure neuve, par une autre porte**, et cette
porte-là est refermée depuis #189. Une mesure sans histoire posait ses deux
extrêmes au premier relevé qui la portait, **même pris au-dessus du seuil** : il
n'y avait rien à comparer, donc rien à refuser, et la règle ci-dessus ne
s'appliquait jamais à ce qui venait de naître. Les quatre mesures de la version
`0.5.13` sont nées ainsi, sous une charge de 5,73 — puis le relevé a été rejoué
à 2,75, remplaçant la section de version sans toucher à la table. Les extrêmes
citaient alors, pour une version présente au journal, des chiffres qu'elle n'y
montrait plus : un maximum de 27,8 ms pour l'ouverture d'une vidéo là où le
relevé conservé en montre 10,9. La table a été corrigée à la main, comme elle
l'avait été pour `0.2.15`.

**Deux règles en sont sorties**, et `verify-gates.sh` les tient :

- **une mesure neuve n'entre dans la table que sur un relevé propre.** Prise sur
  une machine occupée, elle est consignée au relevé et laissée hors de
  l'enveloppe ; elle y entrera au premier relevé calme, et le script le dit ;
- **un extrême posé par le relevé qu'on remplace est repris du relevé courant**,
  meilleur ou pire. Sa section disparaît du fichier ; le laisser tel quel ferait
  citer des chiffres que cette version n'y montre plus. Quand le relevé courant
  n'est pas propre, l'extrême reste tel quel et le script signale qu'il n'est
  plus vérifiable.

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

## Le modèle de table scrute les anomalies, en 0.4.18

Trois mesures font un bond, et il est attendu plutôt que subi :

| Mesure | Avant | Après |
| :----- | ----: | ----: |
| construction du modèle sur 4000 sous-titres | 59 ns | ~8 µs |
| rafraîchir après un décalage de 4000 sous-titres | 26 ns | ~9 µs |
| édition d'une cellule de position | 579 ns | ~10 µs |

**La troisième ligne a été ajoutée à la relecture de fin de phase**, qui l'a
trouvée en confrontant les relevés : elle avait bondi d'un facteur dix-sept
entre la `0.4.17` et la `0.4.18` sans que rien ne le dise, et cette note en
annonçait deux.

L'issue #134 fait marquer par la table les sous-titres dont les positions ne
tiennent pas debout. Le calcul est `scanAnomalies`, qui parcourt le document
entier ; le modèle le refait à sa construction et après **chaque changement de
position** — c'est ce qui garantit qu'un marquage n'est jamais périmé, y compris
après une annulation.

**Le rapport est de cent, la somme est de huit microsecondes.** Le calcul a lieu
une fois par opération, jamais par cellule : ouvrir un fichier coûte 2,4 ms de
lecture, où ces 8 µs pèsent trois millièmes ; décaler quatre mille sous-titres
coûtait 7,7 µs et en coûte le double, ce qu'aucun œil ne distingue d'un geste.

**La troisième est la seule qui soit sur le chemin d'une frappe**, et elle
mérite d'être regardée pour elle-même : valider une position rescanne le
document entier, donc une cellule éditée dans un fichier de quatre mille
sous-titres coûte désormais dix microsecondes au lieu d'une demie. C'est
toujours mille fois moins que ce qu'un doigt peut sentir, et c'est le prix d'un
marquage qui n'est jamais périmé. L'éditer un texte, lui, n'a pas bougé : un
texte n'entre dans aucune anomalie, et rien n'est recalculé.

Ce qui aurait été économisé, et pourquoi ça ne l'a pas été : rendre le calcul
paresseux — marquer sale, recalculer au premier affichage — rendrait ces deux
mesures à leur valeur d'avant sans rien changer au programme, puisque
l'affichage suit toujours l'opération. Ce serait optimiser le banc d'essai.

Ce qui l'a été : `data()` cherche les anomalies d'une ligne par dichotomie et
non par balayage. Aucun benchmark ne le montre — leur fixture est saine, donc la
liste est vide — mais sur un fichier très abîmé, un balayage ferait payer à
chaque cellule le nombre d'anomalies du document.

## Réinitialiser coûte ce que rafraîchir coûte, en 0.4.20

Une mesure nouvelle, demandée par
[l'ADR 0019](../adr/0019-table-en-adaptateur-mince.md) avant que la phase 7 ne
construise dessus : **un changement de structure réinitialise le modèle**, et
l'ADR se donne pour déclencheur de réexamen le moment où un menu ajoutera les
lignes une par une — « une réinitialisation complète à chaque ligne ajoutée
serait alors ridicule ».

Le chiffre ne dit pas ce qu'on attendait :

| Mesure | Ordre de grandeur |
| :----- | ----------------: |
| réinitialisation du modèle après une ligne retirée | ~13 µs |
| rafraîchir après un décalage de 4000 sous-titres | ~12 µs |
| construction du modèle sur 4000 sous-titres | ~10 µs |

**Les trois sont le même chiffre**, et c'est le `scanAnomalies` qu'elles
partagent. Côté modèle, réinitialiser ne coûte rien de plus que rafraîchir : les
`beginResetModel` / `endResetModel` sont deux signaux, le reste est le balayage
du document que toute opération de position paie déjà.

**Ce que la mesure ne contient pas, et ne peut pas contenir : la part de la
vue.** Un `QTableView` demande une `QApplication`, et le binaire de benchmarks
n'en a pas — il prend le `main` de Catch2, et les modèles qu'il mesure sont des
`QObject`. Or c'est précisément la vue qui reconstruit tout, perd la sélection
et refait sa mise en page.

Le déclencheur de l'ADR ne trouvera donc pas son argument ici. La mesure ne
répond pas à la question : **elle la déplace, et dit où elle vit.** C'est ce
qu'on peut en attendre de plus honnête avant d'avoir un écran sous la main.

## Extrêmes

Le minimum et le maximum jamais relevés pour chaque mesure. Cette table n'est
jamais élaguée — c'est elle qui garde l'enveloppe quand les relevés s'effacent.
Une mesure renommée ou retirée y garde ses extrêmes indéfiniment : rien ici ne
distingue une entrée vivante d'une orpheline — élaguer les orphelines n'est
pas le sujet de ce ticket.

<!-- extrêmes -->
| Mesure | Minimum | Relevé le | Maximum | Relevé le |
| :----- | ------: | :-------- | ------: | :-------- |
| versionString | 29.2 ns | 0.5.9 — 2026-08-23 | 64.5 ns | 0.5.10 — 2026-08-23 |
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 42.8 ns | 0.5.3 — 2026-08-22 |
| format | 29.8 ns | 0.3.11 — 2026-08-15 | 47.9 ns | 0.4.14 — 2026-08-21 |
| position vers image | 6.48 ns | 0.3.11 — 2026-08-15 | 16.8 ns | 0.4.10 — 2026-08-19 |
| image vers position | 6.48 ns | 0.4.5 — 2026-08-17 | 12.1 ns | 0.3.9 — 2026-08-15 |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.2.14 — 2026-08-14 | 9.69 ns | 0.5.3 — 2026-08-22 |
| lecture de 4000 sous-titres | 2.08 ms | 0.5.9 — 2026-08-23 | 3.28 ms | 0.5.3 — 2026-08-22 |
| écriture de 4000 sous-titres | 488 µs | 0.3.10 — 2026-08-15 | 783 µs | 0.5.3 — 2026-08-22 |
| décalage de 4000 sous-titres | 6.85 µs | 0.4.9 — 2026-08-19 | 11.4 µs | 0.3.3 — 2026-08-15 |
| décalage puis annulation | 12.8 µs | 0.4.15 — 2026-08-21 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 71.7 µs | 0.3.13 — 2026-08-16 | 93.3 µs | 0.2.12 — 2026-08-14 |
| conversion de fréquence sur 4000 sous-titres | 67.5 µs | 0.5.10 — 2026-08-23 | 100 µs | 0.2.12 — 2026-08-14 |
| tri de 4000 sous-titres à l'envers | 196 µs | 0.3.2 — 2026-08-15 | 742 µs | 0.5.0 — 2026-08-22 |
| suppression d'un sous-titre sur deux | 138 µs | 0.4.14 — 2026-08-21 | 12.5 ms | 0.3.3 — 2026-08-15 |
| insertion de 100 sous-titres vides au milieu | 48.9 µs | 0.3.13 — 2026-08-16 | 96.2 µs | 0.5.3 — 2026-08-22 |
| modification d'un texte, à travers une session | 114 ns | 0.2.14 — 2026-08-14 | 223 ns | 0.5.3 — 2026-08-22 |
| suppression des mentions sur 4000 sous-titres | 850 µs | 0.4.9 — 2026-08-19 | 5.68 ms | 0.4.4 — 2026-08-17 |
| suppression puis annulation | 213 µs | 0.4.14 — 2026-08-21 | 354 µs | 0.5.3 — 2026-08-22 |
| construction du modèle sur 4000 sous-titres | 59.3 ns | 0.4.14 — 2026-08-21 | 13.2 µs | 0.5.3 — 2026-08-22 |
| une fenêtre de 40 lignes, cinq colonnes | 14.7 µs | 0.4.15 — 2026-08-21 | 18.3 µs | 0.5.3 — 2026-08-22 |
| rafraîchir après un décalage de 4000 sous-titres | 25.6 ns | 0.4.11 — 2026-08-20 | 10.8 µs | 0.5.9 — 2026-08-23 |
| édition d'une cellule de texte | 371 ns | 0.5.9 — 2026-08-23 | 899 ns | 0.4.12 — 2026-08-20 |
| édition d'une cellule de position | 484 ns | 0.4.15 — 2026-08-21 | 13.3 µs | 0.5.3 — 2026-08-22 |
| réinitialisation du modèle après une ligne retirée | 7.45 µs | 0.5.0 — 2026-08-22 | 13 µs | 0.4.20 — 2026-08-22 |
| la réplique en cours, sur 4000 sous-titres | 8.81 µs | 0.5.13 — 2026-08-24 | 8.81 µs | 0.5.13 — 2026-08-24 |
| composer une réplique de deux lignes | 188 ns | 0.5.13 — 2026-08-24 | 188 ns | 0.5.13 — 2026-08-24 |
| ouvrir une vidéo | 10.9 ms | 0.5.13 — 2026-08-24 | 10.9 ms | 0.5.13 — 2026-08-24 |
| chercher une position | 577 µs | 0.5.13 — 2026-08-24 | 577 µs | 0.5.13 — 2026-08-24 |

<!-- versionString min=29.1524 max=64.5489 -->
<!-- parse min=29.9 max=42.7539 -->
<!-- format min=29.8143 max=47.8971 -->
<!-- position vers image min=6.48462 max=16.831 -->
<!-- image vers position min=6.47637 max=12.0852 -->
<!-- mise à l'échelle par un rationnel exact min=6.71 max=9.69092 -->
<!-- lecture de 4000 sous-titres min=2082230.0 max=3276990.0 -->
<!-- écriture de 4000 sous-titres min=488279.0 max=782932.0 -->
<!-- décalage de 4000 sous-titres min=6850.43 max=11400.0 -->
<!-- décalage puis annulation min=12847.6 max=20700.0 -->
<!-- transformation de 4000 sous-titres min=71737.0 max=93300.0 -->
<!-- conversion de fréquence sur 4000 sous-titres min=67493.7 max=100000.0 -->
<!-- tri de 4000 sous-titres à l'envers min=196000.0 max=741836.0 -->
<!-- suppression d'un sous-titre sur deux min=137974.0 max=12500000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=48927.4 max=96227.8 -->
<!-- modification d'un texte, à travers une session min=114.0 max=222.951 -->
<!-- suppression des mentions sur 4000 sous-titres min=850257.0 max=5676890.0 -->
<!-- suppression puis annulation min=213280.0 max=353822.0 -->
<!-- construction du modèle sur 4000 sous-titres min=59.2521 max=13180.9 -->
<!-- une fenêtre de 40 lignes, cinq colonnes min=14699.7 max=18297.5 -->
<!-- rafraîchir après un décalage de 4000 sous-titres min=25.5559 max=10829.7 -->
<!-- édition d'une cellule de texte min=370.791 max=898.579 -->
<!-- édition d'une cellule de position min=484.262 max=13302.7 -->
<!-- réinitialisation du modèle après une ligne retirée min=7452.24 max=13047.7 -->
<!-- la réplique en cours, sur 4000 sous-titres min=8805.31 max=8805.31 -->
<!-- composer une réplique de deux lignes min=187.598 max=187.598 -->
<!-- ouvrir une vidéo min=10930700.0 max=10930700.0 -->
<!-- chercher une position min=577286.0 max=577286.0 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

### 0.5.16 — 2026-08-25 — Release — charge 3.74 — allure ×1.44

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 8.14 µs | 6.98 µs |
| composer une réplique de deux lignes | 221 ns | 27.7 ns |
| ouvrir une vidéo | 11.4 ms | 1.25 ms |
| chercher une position | 1.25 ms | 1.08 ms |
| construction du modèle sur 4000 sous-titres | 12.1 µs | 4.16 µs |
| une fenêtre de 40 lignes, cinq colonnes | 38.1 µs | 6.22 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.3 µs | 2.45 µs |
| réinitialisation du modèle après une ligne retirée | 11.1 µs | 3.32 µs |
| édition d'une cellule de texte | 688 ns | 342 ns |
| édition d'une cellule de position | 15 µs | 4.18 µs |
| versionString | 43.2 ns | 4.74 ns |
| parse | 54.3 ns | 13.3 ns |
| format | 75.4 ns | 7.25 ns |
| position vers image | 11.9 ns | 2.04 ns |
| image vers position | 8.35 ns | 0.528 ns |
| mise à l'échelle par un rationnel exact | 12.4 ns | 2.33 ns |
| lecture de 4000 sous-titres | 5.39 ms | 1.15 ms |
| écriture de 4000 sous-titres | 1.41 ms | 507 µs |
| décalage de 4000 sous-titres | 11.9 µs | 7.03 µs |
| décalage puis annulation | 26 µs | 17.3 µs |
| transformation de 4000 sous-titres | 193 µs | 102 µs |
| conversion de fréquence sur 4000 sous-titres | 124 µs | 42.3 µs |
| tri de 4000 sous-titres à l'envers | 433 µs | 130 µs |
| suppression d'un sous-titre sur deux | 392 µs | 274 µs |
| suppression puis annulation | 431 µs | 226 µs |
| insertion de 100 sous-titres vides au milieu | 159 µs | 133 µs |
| modification d'un texte, à travers une session | 232 ns | 52.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.66 ms | 441 µs |

### 0.5.15 — 2026-08-24 — Release — charge 7.49 — allure ×0.81

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 12.6 µs | 1.63 µs |
| composer une réplique de deux lignes | 544 ns | 79.6 ns |
| ouvrir une vidéo | 33.7 ms | 24.2 ms |
| chercher une position | 636 µs | 368 µs |
| construction du modèle sur 4000 sous-titres | 8.37 µs | 1.5 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.2 µs | 1.59 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.62 µs | 563 ns |
| réinitialisation du modèle après une ligne retirée | 8.69 µs | 1.27 µs |
| édition d'une cellule de texte | 409 ns | 100 ns |
| édition d'une cellule de position | 13.5 µs | 1.09 µs |
| versionString | 37.8 ns | 6.06 ns |
| parse | 40 ns | 5.34 ns |
| format | 40.9 ns | 4.39 ns |
| position vers image | 6.92 ns | 0.654 ns |
| image vers position | 7.42 ns | 0.0372 ns |
| mise à l'échelle par un rationnel exact | 7.89 ns | 0.781 ns |
| lecture de 4000 sous-titres | 2.79 ms | 321 µs |
| écriture de 4000 sous-titres | 558 µs | 21.4 µs |
| décalage de 4000 sous-titres | 7.8 µs | 4.61 µs |
| décalage puis annulation | 14 µs | 5.12 µs |
| transformation de 4000 sous-titres | 78.5 µs | 5.82 µs |
| conversion de fréquence sur 4000 sous-titres | 78.3 µs | 10.5 µs |
| tri de 4000 sous-titres à l'envers | 278 µs | 36.6 µs |
| suppression d'un sous-titre sur deux | 185 µs | 32.4 µs |
| suppression puis annulation | 272 µs | 53.3 µs |
| insertion de 100 sous-titres vides au milieu | 55.5 µs | 17.5 µs |
| modification d'un texte, à travers une session | 169 ns | 44.4 ns |
| suppression des mentions sur 4000 sous-titres | 989 µs | 65.6 µs |

### 0.5.14 — 2026-08-24 — Release — charge 6.83 — allure ×1.62

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 11.4 µs | 3.17 µs |
| composer une réplique de deux lignes | 519 ns | 275 ns |
| ouvrir une vidéo | 21.5 ms | 20.1 ms |
| chercher une position | 618 µs | 196 µs |
| construction du modèle sur 4000 sous-titres | 8.53 µs | 1.8 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18 µs | 1.62 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.92 µs | 2.11 µs |
| réinitialisation du modèle après une ligne retirée | 16.5 µs | 4.81 µs |
| édition d'une cellule de texte | 1.92 µs | 2.3 µs |
| édition d'une cellule de position | 36.5 µs | 25.8 µs |
| versionString | 72.1 ns | 19.3 ns |
| parse | 69.4 ns | 24.4 ns |
| format | 86.6 ns | 11 ns |
| position vers image | 15 ns | 0.925 ns |
| image vers position | 14.1 ns | 2.54 ns |
| mise à l'échelle par un rationnel exact | 23 ns | 75.1 ns |
| lecture de 4000 sous-titres | 8.09 ms | 2.09 ms |
| écriture de 4000 sous-titres | 2.24 ms | 952 µs |
| décalage de 4000 sous-titres | 98.3 µs | 75.9 µs |
| décalage puis annulation | 101 µs | 123 µs |
| transformation de 4000 sous-titres | 341 µs | 300 µs |
| conversion de fréquence sur 4000 sous-titres | 295 µs | 260 µs |
| tri de 4000 sous-titres à l'envers | 296 µs | 15 µs |
| suppression d'un sous-titre sur deux | 185 µs | 30.7 µs |
| suppression puis annulation | 270 µs | 31.6 µs |
| insertion de 100 sous-titres vides au milieu | 62.5 µs | 14.2 µs |
| modification d'un texte, à travers une session | 418 ns | 113 ns |
| suppression des mentions sur 4000 sous-titres | 2.22 ms | 520 µs |

### 0.5.13 — 2026-08-24 — Release — charge 3.67 — allure ×1.04

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 8.81 µs | 2.39 µs |
| composer une réplique de deux lignes | 188 ns | 13.5 ns |
| ouvrir une vidéo | 10.9 ms | 2.02 ms |
| chercher une position | 577 µs | 119 µs |
| construction du modèle sur 4000 sous-titres | 14.2 µs | 5.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.4 µs | 3.16 µs |
| rafraîchir après un décalage de 4000 sous-titres | 12.7 µs | 5.27 µs |
| réinitialisation du modèle après une ligne retirée | 12.4 µs | 4.99 µs |
| édition d'une cellule de texte | 465 ns | 193 ns |
| édition d'une cellule de position | 13.8 µs | 3.23 µs |
| versionString | 43.4 ns | 7.2 ns |
| parse | 39.8 ns | 3.22 ns |
| format | 43.8 ns | 1.49 ns |
| position vers image | 7.43 ns | 0.0599 ns |
| image vers position | 8.25 ns | 0.882 ns |
| mise à l'échelle par un rationnel exact | 9.58 ns | 0.272 ns |
| lecture de 4000 sous-titres | 2.81 ms | 577 µs |
| écriture de 4000 sous-titres | 607 µs | 36 µs |
| décalage de 4000 sous-titres | 8.85 µs | 2.76 µs |
| décalage puis annulation | 17.1 µs | 4 µs |
| transformation de 4000 sous-titres | 86.3 µs | 10.2 µs |
| conversion de fréquence sur 4000 sous-titres | 80.9 µs | 10.8 µs |
| tri de 4000 sous-titres à l'envers | 304 µs | 19.8 µs |
| suppression d'un sous-titre sur deux | 179 µs | 27.1 µs |
| suppression puis annulation | 549 µs | 71.3 µs |
| insertion de 100 sous-titres vides au milieu | 65.7 µs | 6.94 µs |
| modification d'un texte, à travers une session | 252 ns | 78.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 140 µs |

### 0.5.12 — 2026-08-23 — Release — charge 6.67 — allure ×1.48

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 26.6 µs | 31.8 µs |
| une fenêtre de 40 lignes, cinq colonnes | 40.4 µs | 5.8 µs |
| rafraîchir après un décalage de 4000 sous-titres | 16.8 µs | 6.9 µs |
| réinitialisation du modèle après une ligne retirée | 29.9 µs | 25.8 µs |
| édition d'une cellule de texte | 1.59 µs | 744 ns |
| édition d'une cellule de position | 32 µs | 32.6 µs |
| versionString | 91 ns | 53.1 ns |
| parse | 66.8 ns | 3.89 ns |
| format | 79.9 ns | 12.1 ns |
| position vers image | 29.2 ns | 142 ns |
| image vers position | 14.7 ns | 2.12 ns |
| mise à l'échelle par un rationnel exact | 23.4 ns | 52.1 ns |
| lecture de 4000 sous-titres | 9.36 ms | 4.12 ms |
| écriture de 4000 sous-titres | 682 µs | 180 µs |
| décalage de 4000 sous-titres | 8.18 µs | 4.59 µs |
| décalage puis annulation | 16.1 µs | 5.6 µs |
| transformation de 4000 sous-titres | 80.5 µs | 13.5 µs |
| conversion de fréquence sur 4000 sous-titres | 82.6 µs | 8.39 µs |
| tri de 4000 sous-titres à l'envers | 303 µs | 68.5 µs |
| suppression d'un sous-titre sur deux | 172 µs | 14.1 µs |
| suppression puis annulation | 282 µs | 42.4 µs |
| insertion de 100 sous-titres vides au milieu | 66.6 µs | 19.1 µs |
| modification d'un texte, à travers une session | 200 ns | 8.56 ns |
| suppression des mentions sur 4000 sous-titres | 1.06 ms | 73 µs |

### 0.5.11 — 2026-08-23 — Release — charge 2.07 — allure ×1.16

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 9.03 µs | 3.11 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.3 µs | 785 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.87 µs | 3.39 µs |
| réinitialisation du modèle après une ligne retirée | 9.69 µs | 9.64 µs |
| édition d'une cellule de texte | 433 ns | 66.8 ns |
| édition d'une cellule de position | 19.1 µs | 9.35 µs |
| versionString | 69.9 ns | 8.53 ns |
| parse | 90.5 ns | 8.24 ns |
| format | 58.6 ns | 5.38 ns |
| position vers image | 7.47 ns | 0.244 ns |
| image vers position | 8.98 ns | 2.38 ns |
| mise à l'échelle par un rationnel exact | 9.5 ns | 1.98 ns |
| lecture de 4000 sous-titres | 3.11 ms | 155 µs |
| écriture de 4000 sous-titres | 624 µs | 40.7 µs |
| décalage de 4000 sous-titres | 9.88 µs | 9.13 µs |
| décalage puis annulation | 19 µs | 8.82 µs |
| transformation de 4000 sous-titres | 87.3 µs | 23.3 µs |
| conversion de fréquence sur 4000 sous-titres | 79 µs | 13.8 µs |
| tri de 4000 sous-titres à l'envers | 286 µs | 45 µs |
| suppression d'un sous-titre sur deux | 170 µs | 43.2 µs |
| suppression puis annulation | 269 µs | 71.8 µs |
| insertion de 100 sous-titres vides au milieu | 58.5 µs | 26.2 µs |
| modification d'un texte, à travers une session | 341 ns | 144 ns |
| suppression des mentions sur 4000 sous-titres | 1.06 ms | 135 µs |

### 0.5.10 — 2026-08-23 — Release — charge 1.41 — allure ×0.83

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 11.1 µs | 6.39 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.2 µs | 602 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.8 µs | 3.75 µs |
| réinitialisation du modèle après une ligne retirée | 11.2 µs | 6.36 µs |
| édition d'une cellule de texte | 415 ns | 48.4 ns |
| édition d'une cellule de position | 13.1 µs | 4.07 µs |
| versionString | 64.5 ns | 15.4 ns |
| parse | 34.8 ns | 12.3 ns |
| format | 34.3 ns | 0.292 ns |
| position vers image | 6.49 ns | 0.067 ns |
| image vers position | 7.6 ns | 1.18 ns |
| mise à l'échelle par un rationnel exact | 8.1 ns | 0.113 ns |
| lecture de 4000 sous-titres | 2.12 ms | 154 µs |
| écriture de 4000 sous-titres | 515 µs | 40.3 µs |
| décalage de 4000 sous-titres | 6.9 µs | 3.58 µs |
| décalage puis annulation | 18 µs | 8.3 µs |
| transformation de 4000 sous-titres | 79.8 µs | 9.7 µs |
| conversion de fréquence sur 4000 sous-titres | 67.5 µs | 10 µs |
| tri de 4000 sous-titres à l'envers | 240 µs | 27.1 µs |
| suppression d'un sous-titre sur deux | 164 µs | 22.4 µs |
| suppression puis annulation | 255 µs | 40.8 µs |
| insertion de 100 sous-titres vides au milieu | 58.2 µs | 47.5 µs |
| modification d'un texte, à travers une session | 215 ns | 88.9 ns |
| suppression des mentions sur 4000 sous-titres | 862 µs | 61.4 µs |

### 0.5.9 — 2026-08-23 — Release — charge 1.38 — allure ×0.30

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 11.3 µs | 5.15 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15 µs | 344 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.8 µs | 3.8 µs |
| réinitialisation du modèle après une ligne retirée | 11.1 µs | 6.47 µs |
| édition d'une cellule de texte | 371 ns | 99.2 ns |
| édition d'une cellule de position | 11.8 µs | 1.33 µs |
| versionString | 29.2 ns | 0.253 ns |
| parse | 32.4 ns | 2.85 ns |
| format | 34.6 ns | 0.412 ns |
| position vers image | 6.51 ns | 0.173 ns |
| image vers position | 6.63 ns | 0.198 ns |
| mise à l'échelle par un rationnel exact | 7.06 ns | 0.0749 ns |
| lecture de 4000 sous-titres | 2.08 ms | 126 µs |
| écriture de 4000 sous-titres | 514 µs | 40.9 µs |
| décalage de 4000 sous-titres | 7.8 µs | 2.75 µs |
| décalage puis annulation | 15.1 µs | 3.13 µs |
| transformation de 4000 sous-titres | 73.9 µs | 7.42 µs |
| conversion de fréquence sur 4000 sous-titres | 70 µs | 8.26 µs |
| tri de 4000 sous-titres à l'envers | 241 µs | 23.5 µs |
| suppression d'un sous-titre sur deux | 142 µs | 17.2 µs |
| suppression puis annulation | 231 µs | 65.7 µs |
| insertion de 100 sous-titres vides au milieu | 53.8 µs | 22.2 µs |
| modification d'un texte, à travers une session | 158 ns | 16.8 ns |
| suppression des mentions sur 4000 sous-titres | 862 µs | 82.7 µs |

### 0.5.8 — 2026-08-23 — Release — charge 6.03 — allure ×0.69

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 8.41 µs | 2.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20 µs | 3.6 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.21 µs | 2.34 µs |
| réinitialisation du modèle après une ligne retirée | 8.73 µs | 1.66 µs |
| édition d'une cellule de texte | 404 ns | 38.1 ns |
| édition d'une cellule de position | 11 µs | 3.65 µs |
| versionString | 37.5 ns | 5.25 ns |
| parse | 39.1 ns | 9.17 ns |
| format | 41.4 ns | 1.41 ns |
| position vers image | 7.82 ns | 0.0977 ns |
| image vers position | 8.14 ns | 0.742 ns |
| mise à l'échelle par un rationnel exact | 8.5 ns | 2.01 ns |
| lecture de 4000 sous-titres | 2.97 ms | 733 µs |
| écriture de 4000 sous-titres | 688 µs | 131 µs |
| décalage de 4000 sous-titres | 7.87 µs | 1.61 µs |
| décalage puis annulation | 255 µs | 965 µs |
| transformation de 4000 sous-titres | 321 µs | 314 µs |
| conversion de fréquence sur 4000 sous-titres | 541 µs | 891 µs |
| tri de 4000 sous-titres à l'envers | 1.95 ms | 1.46 ms |
| suppression d'un sous-titre sur deux | 1.67 ms | 2.93 ms |
| suppression puis annulation | 1.93 ms | 1.4 ms |
| insertion de 100 sous-titres vides au milieu | 787 µs | 1.1 ms |
| modification d'un texte, à travers une session | 754 ns | 545 ns |
| suppression des mentions sur 4000 sous-titres | 3.79 ms | 1.66 ms |

### 0.5.7 — 2026-08-22 — Release — charge 12.53 — allure ×2.13

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 14.8 µs | 4.17 µs |
| une fenêtre de 40 lignes, cinq colonnes | 31.3 µs | 10.5 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13 µs | 15 µs |
| réinitialisation du modèle après une ligne retirée | 15.3 µs | 16.2 µs |
| édition d'une cellule de texte | 957 ns | 463 ns |
| édition d'une cellule de position | 38.5 µs | 53.1 µs |
| versionString | 55.8 ns | 7.94 ns |
| parse | 71.6 ns | 19.3 ns |
| format | 68 ns | 17.8 ns |
| position vers image | 9.09 ns | 0.575 ns |
| image vers position | 11.6 ns | 3.26 ns |
| mise à l'échelle par un rationnel exact | 10.6 ns | 0.746 ns |
| lecture de 4000 sous-titres | 8.33 ms | 2.61 ms |
| écriture de 4000 sous-titres | 1.98 ms | 877 µs |
| décalage de 4000 sous-titres | 77 µs | 90.3 µs |
| décalage puis annulation | 84.5 µs | 94.9 µs |
| transformation de 4000 sous-titres | 457 µs | 382 µs |
| conversion de fréquence sur 4000 sous-titres | 359 µs | 338 µs |
| tri de 4000 sous-titres à l'envers | 1.66 ms | 1.16 ms |
| suppression d'un sous-titre sur deux | 1.14 ms | 1.02 ms |
| suppression puis annulation | 2.05 ms | 1.37 ms |
| insertion de 100 sous-titres vides au milieu | 376 µs | 371 µs |
| modification d'un texte, à travers une session | 511 ns | 397 ns |
| suppression des mentions sur 4000 sous-titres | 3.17 ms | 969 µs |

### 0.5.6 — 2026-08-22 — Release — charge 12.73 — allure ×3.61

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 29.5 µs | 74.1 µs |
| une fenêtre de 40 lignes, cinq colonnes | 92.3 µs | 531 µs |
| rafraîchir après un décalage de 4000 sous-titres | 61 µs | 270 µs |
| réinitialisation du modèle après une ligne retirée | 25.2 µs | 33.5 µs |
| édition d'une cellule de texte | 2.24 µs | 1.71 µs |
| édition d'une cellule de position | 40.2 µs | 57 µs |
| versionString | 66.1 ns | 14.8 ns |
| parse | 96.6 ns | 51 ns |
| format | 75 ns | 12 ns |
| position vers image | 10.3 ns | 1.84 ns |
| image vers position | 16.4 ns | 7.23 ns |
| mise à l'échelle par un rationnel exact | 16.6 ns | 2.57 ns |
| lecture de 4000 sous-titres | 9.56 ms | 2.09 ms |
| écriture de 4000 sous-titres | 1.92 ms | 739 µs |
| décalage de 4000 sous-titres | 144 µs | 143 µs |
| décalage puis annulation | 197 µs | 217 µs |
| transformation de 4000 sous-titres | 522 µs | 493 µs |
| conversion de fréquence sur 4000 sous-titres | 527 µs | 509 µs |
| tri de 4000 sous-titres à l'envers | 2.98 ms | 1.71 ms |
| suppression d'un sous-titre sur deux | 1.45 ms | 1.15 ms |
| suppression puis annulation | 3.49 ms | 1.82 ms |
| insertion de 100 sous-titres vides au milieu | 1.06 ms | 577 µs |
| modification d'un texte, à travers une session | 541 ns | 329 ns |
| suppression des mentions sur 4000 sous-titres | 5.72 ms | 3.47 ms |

### 0.5.5 — 2026-08-22 — Release — charge 3.76 — allure ×1.10

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 15.9 µs | 13.5 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.7 µs | 1.58 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.4 µs | 6.5 µs |
| réinitialisation du modèle après une ligne retirée | 13.7 µs | 3.63 µs |
| édition d'une cellule de texte | 495 ns | 124 ns |
| édition d'une cellule de position | 14.3 µs | 5.83 µs |
| versionString | 35.9 ns | 2.12 ns |
| parse | 40.9 ns | 3.52 ns |
| format | 44.7 ns | 4.53 ns |
| position vers image | 7.84 ns | 0.17 ns |
| image vers position | 8.2 ns | 0.783 ns |
| mise à l'échelle par un rationnel exact | 9.87 ns | 0.958 ns |
| lecture de 4000 sous-titres | 3.43 ms | 491 µs |
| écriture de 4000 sous-titres | 838 µs | 257 µs |
| décalage de 4000 sous-titres | 9.26 µs | 3.14 µs |
| décalage puis annulation | 17.5 µs | 6.81 µs |
| transformation de 4000 sous-titres | 97.1 µs | 26.6 µs |
| conversion de fréquence sur 4000 sous-titres | 93.7 µs | 19.2 µs |
| tri de 4000 sous-titres à l'envers | 410 µs | 226 µs |
| suppression d'un sous-titre sur deux | 204 µs | 94.8 µs |
| suppression puis annulation | 373 µs | 206 µs |
| insertion de 100 sous-titres vides au milieu | 114 µs | 104 µs |
| modification d'un texte, à travers une session | 220 ns | 70.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.33 ms | 398 µs |

### 0.5.4 — 2026-08-22 — Release — charge 2.67

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 13.2 µs | 8.2 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.7 µs | 2.15 µs |
| rafraîchir après un décalage de 4000 sous-titres | 12.7 µs | 1.7 µs |
| réinitialisation du modèle après une ligne retirée | 12.7 µs | 5.06 µs |
| édition d'une cellule de texte | 430 ns | 68.5 ns |
| édition d'une cellule de position | 14 µs | 2.33 µs |
| versionString | 33.6 ns | 0.941 ns |
| parse | 42.4 ns | 11.9 ns |
| format | 45.6 ns | 2.7 ns |
| position vers image | 7.44 ns | 0.0818 ns |
| image vers position | 8.03 ns | 1.74 ns |
| mise à l'échelle par un rationnel exact | 9.09 ns | 0.0764 ns |
| lecture de 4000 sous-titres | 2.71 ms | 153 µs |
| écriture de 4000 sous-titres | 627 µs | 73.2 µs |
| décalage de 4000 sous-titres | 8.98 µs | 2.72 µs |
| décalage puis annulation | 17.2 µs | 1.75 µs |
| transformation de 4000 sous-titres | 83.7 µs | 10.1 µs |
| conversion de fréquence sur 4000 sous-titres | 81.7 µs | 13.3 µs |
| tri de 4000 sous-titres à l'envers | 291 µs | 41.7 µs |
| suppression d'un sous-titre sur deux | 159 µs | 14.4 µs |
| suppression puis annulation | 253 µs | 25.7 µs |
| insertion de 100 sous-titres vides au milieu | 55.7 µs | 18.5 µs |
| modification d'un texte, à travers une session | 200 ns | 22.5 ns |
| suppression des mentions sur 4000 sous-titres | 2.36 ms | 855 µs |

### 0.5.3 — 2026-08-22 — Release — charge 1.49

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 13.2 µs | 24.4 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.3 µs | 1.03 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.5 µs | 3.27 µs |
| réinitialisation du modèle après une ligne retirée | 10.8 µs | 9.02 µs |
| édition d'une cellule de texte | 527 ns | 377 ns |
| édition d'une cellule de position | 13.3 µs | 2.35 µs |
| versionString | 35.7 ns | 1.9 ns |
| parse | 42.8 ns | 7.19 ns |
| format | 44.7 ns | 5.36 ns |
| position vers image | 8.47 ns | 1.57 ns |
| image vers position | 7.85 ns | 0.297 ns |
| mise à l'échelle par un rationnel exact | 9.69 ns | 0.518 ns |
| lecture de 4000 sous-titres | 3.28 ms | 358 µs |
| écriture de 4000 sous-titres | 783 µs | 156 µs |
| décalage de 4000 sous-titres | 9.08 µs | 5.32 µs |
| décalage puis annulation | 17.3 µs | 5.76 µs |
| transformation de 4000 sous-titres | 89 µs | 16.5 µs |
| conversion de fréquence sur 4000 sous-titres | 92.7 µs | 25.6 µs |
| tri de 4000 sous-titres à l'envers | 378 µs | 123 µs |
| suppression d'un sous-titre sur deux | 211 µs | 75.7 µs |
| suppression puis annulation | 354 µs | 155 µs |
| insertion de 100 sous-titres vides au milieu | 96.2 µs | 82.1 µs |
| modification d'un texte, à travers une session | 223 ns | 63.3 ns |
| suppression des mentions sur 4000 sous-titres | 1.13 ms | 158 µs |

### 0.5.2 — 2026-08-22 — Release — charge 1.47

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 8.32 µs | 2.02 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15.4 µs | 1.57 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.41 µs | 3.99 µs |
| réinitialisation du modèle après une ligne retirée | 8.86 µs | 1.61 µs |
| édition d'une cellule de texte | 457 ns | 181 ns |
| édition d'une cellule de position | 9.25 µs | 650 ns |
| versionString | 31.5 ns | 9.32 ns |
| parse | 38.9 ns | 2.88 ns |
| format | 40.6 ns | 14.4 ns |
| position vers image | 7.46 ns | 0.139 ns |
| image vers position | 7.97 ns | 0.47 ns |
| mise à l'échelle par un rationnel exact | 9.5 ns | 1.8 ns |
| lecture de 4000 sous-titres | 2.78 ms | 392 µs |
| écriture de 4000 sous-titres | 663 µs | 136 µs |
| décalage de 4000 sous-titres | 7.67 µs | 3.16 µs |
| décalage puis annulation | 15.2 µs | 8.18 µs |
| transformation de 4000 sous-titres | 80.4 µs | 17.6 µs |
| conversion de fréquence sur 4000 sous-titres | 83.6 µs | 28.6 µs |
| tri de 4000 sous-titres à l'envers | 357 µs | 267 µs |
| suppression d'un sous-titre sur deux | 140 µs | 25 µs |
| suppression puis annulation | 303 µs | 125 µs |
| insertion de 100 sous-titres vides au milieu | 61.8 µs | 58.7 µs |
| modification d'un texte, à travers une session | 206 ns | 107 ns |
| suppression des mentions sur 4000 sous-titres | 1 ms | 140 µs |

### 0.5.1 — 2026-08-22 — Release — charge 6.23

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 12.8 µs | 4.81 µs |
| une fenêtre de 40 lignes, cinq colonnes | 26.2 µs | 8.55 µs |
| rafraîchir après un décalage de 4000 sous-titres | 14.7 µs | 11.6 µs |
| réinitialisation du modèle après une ligne retirée | 12.3 µs | 9.05 µs |
| édition d'une cellule de texte | 1.17 µs | 374 ns |
| édition d'une cellule de position | 13.8 µs | 3.76 µs |
| versionString | 60.5 ns | 8.68 ns |
| parse | 51.3 ns | 7.75 ns |
| format | 52.5 ns | 18.9 ns |
| position vers image | 15.2 ns | 2.3 ns |
| image vers position | 14.9 ns | 0.615 ns |
| mise à l'échelle par un rationnel exact | 14 ns | 2.11 ns |
| lecture de 4000 sous-titres | 5.72 ms | 1.08 ms |
| écriture de 4000 sous-titres | 1.64 ms | 239 µs |
| décalage de 4000 sous-titres | 12 µs | 6.11 µs |
| décalage puis annulation | 27.3 µs | 12.4 µs |
| transformation de 4000 sous-titres | 189 µs | 110 µs |
| conversion de fréquence sur 4000 sous-titres | 110 µs | 55.3 µs |
| tri de 4000 sous-titres à l'envers | 1.43 ms | 457 µs |
| suppression d'un sous-titre sur deux | 847 µs | 339 µs |
| suppression puis annulation | 1.28 ms | 777 µs |
| insertion de 100 sous-titres vides au milieu | 216 µs | 127 µs |
| modification d'un texte, à travers une session | 563 ns | 113 ns |
| suppression des mentions sur 4000 sous-titres | 1.59 ms | 402 µs |

### 0.5.0 — 2026-08-22 — Release — charge 1.45

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 7.77 µs | 3.23 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15 µs | 853 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.45 µs | 2.52 µs |
| réinitialisation du modèle après une ligne retirée | 7.45 µs | 3.76 µs |
| édition d'une cellule de texte | 433 ns | 41.7 ns |
| édition d'une cellule de position | 9.54 µs | 1.2 µs |
| versionString | 42.6 ns | 10.8 ns |
| parse | 34.4 ns | 2.81 ns |
| format | 36.6 ns | 0.658 ns |
| position vers image | 6.49 ns | 0.0685 ns |
| image vers position | 6.59 ns | 0.0998 ns |
| mise à l'échelle par un rationnel exact | 7.94 ns | 0.0603 ns |
| lecture de 4000 sous-titres | 2.36 ms | 95.5 µs |
| écriture de 4000 sous-titres | 565 µs | 26.3 µs |
| décalage de 4000 sous-titres | 7.05 µs | 3.49 µs |
| décalage puis annulation | 13.2 µs | 2.41 µs |
| transformation de 4000 sous-titres | 79.3 µs | 5.05 µs |
| conversion de fréquence sur 4000 sous-titres | 77.1 µs | 10.3 µs |
| tri de 4000 sous-titres à l'envers | 742 µs | 459 µs |
| suppression d'un sous-titre sur deux | 172 µs | 24.8 µs |
| suppression puis annulation | 243 µs | 52.9 µs |
| insertion de 100 sous-titres vides au milieu | 62.4 µs | 28.6 µs |
| modification d'un texte, à travers une session | 199 ns | 103 ns |
| suppression des mentions sur 4000 sous-titres | 1.03 ms | 107 µs |

### 0.4.21 — 2026-08-22 — Release — charge 1.42

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 8.38 µs | 3.6 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 3.72 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.25 µs | 3.28 µs |
| réinitialisation du modèle après une ligne retirée | 8.15 µs | 1.72 µs |
| édition d'une cellule de texte | 432 ns | 63.7 ns |
| édition d'une cellule de position | 8.98 µs | 644 ns |
| versionString | 33.6 ns | 0.427 ns |
| parse | 39 ns | 2.82 ns |
| format | 42.2 ns | 1.81 ns |
| position vers image | 7.83 ns | 4.08 ns |
| image vers position | 9.59 ns | 2.24 ns |
| mise à l'échelle par un rationnel exact | 8.78 ns | 2.98 ns |
| lecture de 4000 sous-titres | 2.65 ms | 531 µs |
| écriture de 4000 sous-titres | 586 µs | 35.3 µs |
| décalage de 4000 sous-titres | 8.46 µs | 7.35 µs |
| décalage puis annulation | 15.6 µs | 5.21 µs |
| transformation de 4000 sous-titres | 77.6 µs | 21.6 µs |
| conversion de fréquence sur 4000 sous-titres | 77.5 µs | 7.56 µs |
| tri de 4000 sous-titres à l'envers | 255 µs | 38.4 µs |
| suppression d'un sous-titre sur deux | 160 µs | 24 µs |
| suppression puis annulation | 221 µs | 59.9 µs |
| insertion de 100 sous-titres vides au milieu | 58.5 µs | 16.6 µs |
| modification d'un texte, à travers une session | 191 ns | 26.5 ns |
| suppression des mentions sur 4000 sous-titres | 853 µs | 74.3 µs |

### 0.4.20 — 2026-08-22 — Release — charge 3.87

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 10.5 µs | 6.84 µs |
| une fenêtre de 40 lignes, cinq colonnes | 35.3 µs | 3.52 µs |
| rafraîchir après un décalage de 4000 sous-titres | 12.5 µs | 6.71 µs |
| réinitialisation du modèle après une ligne retirée | 13 µs | 2.18 µs |
| édition d'une cellule de texte | 527 ns | 331 ns |
| édition d'une cellule de position | 14.5 µs | 4.67 µs |
| versionString | 48.2 ns | 11 ns |
| parse | 79.1 ns | 6.14 ns |
| format | 46.7 ns | 11.6 ns |
| position vers image | 8.06 ns | 0.956 ns |
| image vers position | 11.1 ns | 1.84 ns |
| mise à l'échelle par un rationnel exact | 12.6 ns | 2.62 ns |
| lecture de 4000 sous-titres | 3.53 ms | 787 µs |
| écriture de 4000 sous-titres | 1.11 ms | 285 µs |
| décalage de 4000 sous-titres | 8.3 µs | 3.37 µs |
| décalage puis annulation | 18.9 µs | 4.74 µs |
| transformation de 4000 sous-titres | 88.2 µs | 14.3 µs |
| conversion de fréquence sur 4000 sous-titres | 83 µs | 15.9 µs |
| tri de 4000 sous-titres à l'envers | 312 µs | 93.8 µs |
| suppression d'un sous-titre sur deux | 297 µs | 188 µs |
| suppression puis annulation | 468 µs | 209 µs |
| insertion de 100 sous-titres vides au milieu | 76.7 µs | 33.4 µs |
| modification d'un texte, à travers une session | 238 ns | 54.3 ns |
| suppression des mentions sur 4000 sous-titres | 1.73 ms | 407 µs |

### 0.4.19 — 2026-08-22 — Release — charge 3.13

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 9.15 µs | 2.23 µs |
| une fenêtre de 40 lignes, cinq colonnes | 19.1 µs | 2.1 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.7 µs | 3.67 µs |
| édition d'une cellule de texte | 437 ns | 132 ns |
| édition d'une cellule de position | 10.3 µs | 1.77 µs |
| versionString | 37.5 ns | 1.24 ns |
| parse | 42 ns | 11.5 ns |
| format | 63.3 ns | 12.6 ns |
| position vers image | 7.51 ns | 0.327 ns |
| image vers position | 7.83 ns | 0.169 ns |
| mise à l'échelle par un rationnel exact | 8.54 ns | 0.248 ns |
| lecture de 4000 sous-titres | 3.11 ms | 758 µs |
| écriture de 4000 sous-titres | 1.19 ms | 701 µs |
| décalage de 4000 sous-titres | 13.5 µs | 23.3 µs |
| décalage puis annulation | 15.3 µs | 3.03 µs |
| transformation de 4000 sous-titres | 87.9 µs | 14.2 µs |
| conversion de fréquence sur 4000 sous-titres | 103 µs | 66.7 µs |
| tri de 4000 sous-titres à l'envers | 380 µs | 322 µs |
| suppression d'un sous-titre sur deux | 419 µs | 547 µs |
| suppression puis annulation | 309 µs | 239 µs |
| insertion de 100 sous-titres vides au milieu | 66.4 µs | 13.8 µs |
| modification d'un texte, à travers une session | 194 ns | 16.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.67 ms | 859 µs |

### 0.4.18 — 2026-08-22 — Release — charge 6.57

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 8.69 µs | 2.5 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 351 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.81 µs | 2.39 µs |
| édition d'une cellule de texte | 412 ns | 35 ns |
| édition d'une cellule de position | 10 µs | 3.85 µs |
| versionString | 38.7 ns | 4.54 ns |
| parse | 40.4 ns | 2.12 ns |
| format | 95.9 ns | 22.3 ns |
| position vers image | 7.44 ns | 0.114 ns |
| image vers position | 7.6 ns | 1.38 ns |
| mise à l'échelle par un rationnel exact | 8.11 ns | 0.241 ns |
| lecture de 4000 sous-titres | 2.88 ms | 281 µs |
| écriture de 4000 sous-titres | 687 µs | 142 µs |
| décalage de 4000 sous-titres | 8.21 µs | 1.71 µs |
| décalage puis annulation | 16.4 µs | 9.03 µs |
| transformation de 4000 sous-titres | 82.1 µs | 10.5 µs |
| conversion de fréquence sur 4000 sous-titres | 80.5 µs | 16.2 µs |
| tri de 4000 sous-titres à l'envers | 312 µs | 87.3 µs |
| suppression d'un sous-titre sur deux | 168 µs | 37.5 µs |
| suppression puis annulation | 264 µs | 24.1 µs |
| insertion de 100 sous-titres vides au milieu | 58.8 µs | 16.6 µs |
| modification d'un texte, à travers une session | 204 ns | 80.8 ns |
| suppression des mentions sur 4000 sous-titres | 1.08 ms | 57.1 µs |

### 0.4.17 — 2026-08-22 — Release — charge 2.05

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 68.5 ns | 17 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.7 µs | 532 ns |
| rafraîchir après un décalage de 4000 sous-titres | 33.9 ns | 12.9 ns |
| édition d'une cellule de texte | 483 ns | 99.1 ns |
| édition d'une cellule de position | 579 ns | 99.2 ns |
| versionString | 41.1 ns | 1.56 ns |
| parse | 39.5 ns | 2.26 ns |
| format | 39.7 ns | 5.26 ns |
| position vers image | 7.82 ns | 2.23 ns |
| image vers position | 7.44 ns | 0.0835 ns |
| mise à l'échelle par un rationnel exact | 8.68 ns | 1.71 ns |
| lecture de 4000 sous-titres | 2.61 ms | 163 µs |
| écriture de 4000 sous-titres | 608 µs | 47.6 µs |
| décalage de 4000 sous-titres | 8.89 µs | 2.75 µs |
| décalage puis annulation | 16.9 µs | 2.77 µs |
| transformation de 4000 sous-titres | 86.2 µs | 5.89 µs |
| conversion de fréquence sur 4000 sous-titres | 87.6 µs | 13.7 µs |
| tri de 4000 sous-titres à l'envers | 297 µs | 30.5 µs |
| suppression d'un sous-titre sur deux | 161 µs | 15.6 µs |
| suppression puis annulation | 271 µs | 23.8 µs |
| insertion de 100 sous-titres vides au milieu | 60.4 µs | 16.6 µs |
| modification d'un texte, à travers une session | 207 ns | 24.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.04 ms | 58.2 µs |

### 0.4.16 — 2026-08-22 — Release — charge 2.03

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 68 ns | 8.43 ns |
| une fenêtre de 40 lignes, cinq colonnes | 33.4 µs | 3.55 µs |
| rafraîchir après un décalage de 4000 sous-titres | 31.6 ns | 0.782 ns |
| édition d'une cellule de texte | 468 ns | 76.6 ns |
| édition d'une cellule de position | 614 ns | 125 ns |
| versionString | 34.4 ns | 0.531 ns |
| parse | 48.2 ns | 16 ns |
| format | 43.2 ns | 0.712 ns |
| position vers image | 8.12 ns | 1.94 ns |
| image vers position | 7.57 ns | 1.18 ns |
| mise à l'échelle par un rationnel exact | 7.71 ns | 0.123 ns |
| lecture de 4000 sous-titres | 2.86 ms | 429 µs |
| écriture de 4000 sous-titres | 584 µs | 36.7 µs |
| décalage de 4000 sous-titres | 7.86 µs | 5.34 µs |
| décalage puis annulation | 14.6 µs | 2.38 µs |
| transformation de 4000 sous-titres | 78.4 µs | 5.97 µs |
| conversion de fréquence sur 4000 sous-titres | 79 µs | 13 µs |
| tri de 4000 sous-titres à l'envers | 356 µs | 98.1 µs |
| suppression d'un sous-titre sur deux | 162 µs | 18.8 µs |
| suppression puis annulation | 254 µs | 35.8 µs |
| insertion de 100 sous-titres vides au milieu | 66.5 µs | 39.6 µs |
| modification d'un texte, à travers une session | 202 ns | 42.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.07 ms | 121 µs |

### 0.4.15 — 2026-08-21 — Release — charge 1.48

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 59.5 ns | 21.7 ns |
| une fenêtre de 40 lignes, cinq colonnes | 14.7 µs | 462 ns |
| rafraîchir après un décalage de 4000 sous-titres | 26.4 ns | 0.54 ns |
| édition d'une cellule de texte | 379 ns | 198 ns |
| édition d'une cellule de position | 484 ns | 144 ns |
| versionString | 36.5 ns | 0.445 ns |
| parse | 39.9 ns | 2.23 ns |
| format | 32.1 ns | 0.715 ns |
| position vers image | 6.49 ns | 0.0682 ns |
| image vers position | 6.89 ns | 1.83 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0683 ns |
| lecture de 4000 sous-titres | 2.36 ms | 76.1 µs |
| écriture de 4000 sous-titres | 560 µs | 16.4 µs |
| décalage de 4000 sous-titres | 7.85 µs | 4.15 µs |
| décalage puis annulation | 12.8 µs | 3.11 µs |
| transformation de 4000 sous-titres | 78.3 µs | 7.94 µs |
| conversion de fréquence sur 4000 sous-titres | 68.4 µs | 9.83 µs |
| tri de 4000 sous-titres à l'envers | 235 µs | 26.3 µs |
| suppression d'un sous-titre sur deux | 158 µs | 12.5 µs |
| suppression puis annulation | 255 µs | 31.3 µs |
| insertion de 100 sous-titres vides au milieu | 49.6 µs | 21.6 µs |
| modification d'un texte, à travers une session | 165 ns | 26 ns |
| suppression des mentions sur 4000 sous-titres | 938 µs | 58.2 µs |

### 0.4.14 — 2026-08-21 — Release — charge 1.48

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 59.3 ns | 22.1 ns |
| une fenêtre de 40 lignes, cinq colonnes | 15.1 µs | 600 ns |
| rafraîchir après un décalage de 4000 sous-titres | 28.7 ns | 0.6 ns |
| édition d'une cellule de texte | 414 ns | 133 ns |
| édition d'une cellule de position | 575 ns | 184 ns |
| versionString | 30.1 ns | 0.32 ns |
| parse | 37.8 ns | 11.5 ns |
| format | 47.9 ns | 20.7 ns |
| position vers image | 6.49 ns | 0.0647 ns |
| image vers position | 6.49 ns | 0.0691 ns |
| mise à l'échelle par un rationnel exact | 7.05 ns | 0.0541 ns |
| lecture de 4000 sous-titres | 2.36 ms | 297 µs |
| écriture de 4000 sous-titres | 556 µs | 115 µs |
| décalage de 4000 sous-titres | 7.72 µs | 2.77 µs |
| décalage puis annulation | 15.3 µs | 5.24 µs |
| transformation de 4000 sous-titres | 71.8 µs | 7.84 µs |
| conversion de fréquence sur 4000 sous-titres | 77.5 µs | 27.8 µs |
| tri de 4000 sous-titres à l'envers | 241 µs | 16 µs |
| suppression d'un sous-titre sur deux | 138 µs | 16.2 µs |
| suppression puis annulation | 213 µs | 12.6 µs |
| insertion de 100 sous-titres vides au milieu | 54.1 µs | 12.3 µs |
| modification d'un texte, à travers une session | 182 ns | 121 ns |
| suppression des mentions sur 4000 sous-titres | 858 µs | 44.6 µs |

### 0.4.13 — 2026-08-21 — Release — charge 1.64

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 64.5 ns | 4.93 ns |
| une fenêtre de 40 lignes, cinq colonnes | 16.8 µs | 525 ns |
| rafraîchir après un décalage de 4000 sous-titres | 33.7 ns | 5.91 ns |
| édition d'une cellule de texte | 441 ns | 78.7 ns |
| édition d'une cellule de position | 559 ns | 99.7 ns |
| versionString | 41 ns | 1.2 ns |
| parse | 39.7 ns | 10.5 ns |
| format | 37.8 ns | 13.9 ns |
| position vers image | 7.46 ns | 0.181 ns |
| image vers position | 7.43 ns | 0.0846 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0533 ns |
| lecture de 4000 sous-titres | 2.63 ms | 205 µs |
| écriture de 4000 sous-titres | 627 µs | 62.4 µs |
| décalage de 4000 sous-titres | 7.65 µs | 3.42 µs |
| décalage puis annulation | 15 µs | 3.81 µs |
| transformation de 4000 sous-titres | 82.8 µs | 13.8 µs |
| conversion de fréquence sur 4000 sous-titres | 81.7 µs | 16.3 µs |
| tri de 4000 sous-titres à l'envers | 292 µs | 66.3 µs |
| suppression d'un sous-titre sur deux | 179 µs | 20.5 µs |
| suppression puis annulation | 251 µs | 21.2 µs |
| insertion de 100 sous-titres vides au milieu | 59.9 µs | 10.7 µs |
| modification d'un texte, à travers une session | 184 ns | 23.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.02 ms | 91.5 µs |

### 0.4.12 — 2026-08-20 — Release — charge 2.50

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 66.2 ns | 8.35 ns |
| une fenêtre de 40 lignes, cinq colonnes | 18.4 µs | 1.3 µs |
| rafraîchir après un décalage de 4000 sous-titres | 27.9 ns | 3.73 ns |
| édition d'une cellule de texte | 463 ns | 153 ns |
| édition d'une cellule de position | 580 ns | 122 ns |
| versionString | 36.4 ns | 1.36 ns |
| parse | 39 ns | 2.67 ns |
| format | 38.6 ns | 1.95 ns |
| position vers image | 8.05 ns | 0.7 ns |
| image vers position | 11.4 ns | 0.696 ns |
| mise à l'échelle par un rationnel exact | 8.13 ns | 0.231 ns |
| lecture de 4000 sous-titres | 3.49 ms | 876 µs |
| écriture de 4000 sous-titres | 633 µs | 52.7 µs |
| décalage de 4000 sous-titres | 8.19 µs | 9.66 µs |
| décalage puis annulation | 16.2 µs | 5.85 µs |
| transformation de 4000 sous-titres | 86 µs | 15.9 µs |
| conversion de fréquence sur 4000 sous-titres | 86.9 µs | 28 µs |
| tri de 4000 sous-titres à l'envers | 541 µs | 235 µs |
| suppression d'un sous-titre sur deux | 204 µs | 59.1 µs |
| suppression puis annulation | 427 µs | 140 µs |
| insertion de 100 sous-titres vides au milieu | 236 µs | 138 µs |
| modification d'un texte, à travers une session | 248 ns | 78.8 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 162 µs |

### 0.4.11 — 2026-08-20 — Release — charge 2.65

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| construction du modèle sur 4000 sous-titres | 64.5 ns | 4.33 ns |
| une fenêtre de 40 lignes, cinq colonnes | 18.3 µs | 827 ns |
| rafraîchir après un décalage de 4000 sous-titres | 25.6 ns | 3.59 ns |
| versionString | 69.9 ns | 7.77 ns |
| parse | 40 ns | 5.24 ns |
| format | 45.5 ns | 12.2 ns |
| position vers image | 8.11 ns | 1.12 ns |
| image vers position | 7.81 ns | 0.117 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0884 ns |
| lecture de 4000 sous-titres | 2.73 ms | 241 µs |
| écriture de 4000 sous-titres | 596 µs | 29.3 µs |
| décalage de 4000 sous-titres | 9.22 µs | 5.22 µs |
| décalage puis annulation | 17.2 µs | 2.68 µs |
| transformation de 4000 sous-titres | 84.2 µs | 11.5 µs |
| conversion de fréquence sur 4000 sous-titres | 81.1 µs | 9.98 µs |
| tri de 4000 sous-titres à l'envers | 266 µs | 15.7 µs |
| suppression d'un sous-titre sur deux | 165 µs | 32.2 µs |
| suppression puis annulation | 253 µs | 39.2 µs |
| insertion de 100 sous-titres vides au milieu | 58.9 µs | 28.8 µs |
| modification d'un texte, à travers une session | 175 ns | 10.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.11 ms | 151 µs |

### 0.4.10 — 2026-08-19 — Release — charge 1.43

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 37.4 ns | 1.14 ns |
| parse | 37.8 ns | 4 ns |
| format | 39.7 ns | 0.92 ns |
| position vers image | 16.8 ns | 4.5 ns |
| image vers position | 7.44 ns | 0.087 ns |
| mise à l'échelle par un rationnel exact | 7.24 ns | 1.17 ns |
| lecture de 4000 sous-titres | 2.4 ms | 475 µs |
| écriture de 4000 sous-titres | 521 µs | 55.5 µs |
| décalage de 4000 sous-titres | 7.68 µs | 2.4 µs |
| décalage puis annulation | 15.1 µs | 3.29 µs |
| transformation de 4000 sous-titres | 75.1 µs | 18.3 µs |
| conversion de fréquence sur 4000 sous-titres | 73.1 µs | 7.13 µs |
| tri de 4000 sous-titres à l'envers | 244 µs | 28.4 µs |
| suppression d'un sous-titre sur deux | 155 µs | 15 µs |
| suppression puis annulation | 275 µs | 76.4 µs |
| insertion de 100 sous-titres vides au milieu | 59.4 µs | 13.8 µs |
| modification d'un texte, à travers une session | 153 ns | 113 ns |
| suppression des mentions sur 4000 sous-titres | 1.17 ms | 277 µs |

### 0.4.9 — 2026-08-19 — Release — charge 1.40

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 35.5 ns | 2.66 ns |
| parse | 39.9 ns | 1.64 ns |
| format | 47.8 ns | 22.8 ns |
| position vers image | 7.55 ns | 0.115 ns |
| image vers position | 7.42 ns | 0.0634 ns |
| mise à l'échelle par un rationnel exact | 8.31 ns | 0.673 ns |
| lecture de 4000 sous-titres | 2.74 ms | 508 µs |
| écriture de 4000 sous-titres | 582 µs | 65.7 µs |
| décalage de 4000 sous-titres | 6.85 µs | 3.48 µs |
| décalage puis annulation | 13.2 µs | 3.64 µs |
| transformation de 4000 sous-titres | 84.5 µs | 17.3 µs |
| conversion de fréquence sur 4000 sous-titres | 83.7 µs | 7.22 µs |
| tri de 4000 sous-titres à l'envers | 272 µs | 24.7 µs |
| suppression d'un sous-titre sur deux | 161 µs | 16.6 µs |
| suppression puis annulation | 258 µs | 36.4 µs |
| insertion de 100 sous-titres vides au milieu | 61.6 µs | 12.9 µs |
| modification d'un texte, à travers une session | 140 ns | 88.8 ns |
| suppression des mentions sur 4000 sous-titres | 850 µs | 126 µs |

### 0.4.8 — 2026-08-18 — Release — charge 3.25

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 73.8 ns | 7.55 ns |
| parse | 99.9 ns | 8.68 ns |
| format | 107 ns | 4.07 ns |
| position vers image | 15.1 ns | 0.409 ns |
| image vers position | 15.8 ns | 1.75 ns |
| mise à l'échelle par un rationnel exact | 43.1 ns | 231 ns |
| lecture de 4000 sous-titres | 7.47 ms | 1.56 ms |
| écriture de 4000 sous-titres | 1.89 ms | 877 µs |
| décalage de 4000 sous-titres | 23.5 µs | 24.5 µs |
| décalage puis annulation | 41.2 µs | 45.3 µs |
| transformation de 4000 sous-titres | 247 µs | 342 µs |
| conversion de fréquence sur 4000 sous-titres | 187 µs | 94.4 µs |
| tri de 4000 sous-titres à l'envers | 845 µs | 616 µs |
| suppression d'un sous-titre sur deux | 406 µs | 230 µs |
| suppression puis annulation | 786 µs | 453 µs |
| insertion de 100 sous-titres vides au milieu | 70.5 µs | 43.5 µs |
| modification d'un texte, à travers une session | 269 ns | 112 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 119 µs |

### 0.4.7 — 2026-08-18 — Release — charge 3.93

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 45 ns | 6.01 ns |
| parse | 44.3 ns | 3.52 ns |
| format | 51.2 ns | 9.56 ns |
| position vers image | 9.44 ns | 0.782 ns |
| image vers position | 8.79 ns | 0.517 ns |
| mise à l'échelle par un rationnel exact | 14 ns | 4.82 ns |
| lecture de 4000 sous-titres | 5.44 ms | 1.35 ms |
| écriture de 4000 sous-titres | 1.2 ms | 465 µs |
| décalage de 4000 sous-titres | 16.2 µs | 9.41 µs |
| décalage puis annulation | 18.8 µs | 11.3 µs |
| transformation de 4000 sous-titres | 120 µs | 76 µs |
| conversion de fréquence sur 4000 sous-titres | 111 µs | 30.6 µs |
| tri de 4000 sous-titres à l'envers | 444 µs | 122 µs |
| suppression d'un sous-titre sur deux | 15.4 ms | 3.27 ms |
| insertion de 100 sous-titres vides au milieu | 338 µs | 279 µs |
| modification d'un texte, à travers une session | 208 ns | 61.1 ns |
| suppression des mentions sur 4000 sous-titres | 9.36 ms | 2.3 ms |

### 0.4.6 — 2026-08-18 — Release — charge 2.27

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| versionString | 39.1 ns | 1.5 ns |
| parse | 39.6 ns | 4.04 ns |
| format | 38 ns | 1.51 ns |
| position vers image | 7.54 ns | 0.11 ns |
| image vers position | 7.81 ns | 0.0662 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0591 ns |
| lecture de 4000 sous-titres | 2.44 ms | 190 µs |
| écriture de 4000 sous-titres | 556 µs | 27.6 µs |
| décalage de 4000 sous-titres | 10.5 µs | 5.01 µs |
| décalage puis annulation | 16.6 µs | 996 ns |
| transformation de 4000 sous-titres | 89.5 µs | 3.59 µs |
| conversion de fréquence sur 4000 sous-titres | 95.3 µs | 12.7 µs |
| tri de 4000 sous-titres à l'envers | 362 µs | 98 µs |
| suppression d'un sous-titre sur deux | 10.7 ms | 960 µs |
| insertion de 100 sous-titres vides au milieu | 64.9 µs | 12.8 µs |
| modification d'un texte, à travers une session | 173 ns | 50.7 ns |
| suppression des mentions sur 4000 sous-titres | 5.82 ms | 550 µs |

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
