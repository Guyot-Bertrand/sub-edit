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
> n'entre pas au journal.** Les benchmarks tournent, leurs chiffres s'affichent,
> et rien n'est écrit.

> **Et une mesure plus dispersée que d'habitude ne fixe aucun extrême**, même
> sur une machine calme : au-delà de **1,25 fois sa dispersion médiane**, elle
> entre au relevé et reste hors de la table.

### Deux critères, parce qu'un seul ne lit pas la mesure — #202

Le seuil de charge lit **la machine**. Il ne lit pas **la mesure**, qui est
pourtant la seule chose que le banc sache dire de sa propre fiabilité — et elle
est là, dans la colonne « Écart-type » de chaque relevé.

Un relevé pris à charge 1,44 — donc admis — a posé un maximum définitif sur un
ticket qui ne changeait aucun `.cpp` : 3,4 ms pour la lecture de 4000
sous-titres, contre 3,28. Son écart-type valait **26 % de sa moyenne**, là où
cette mesure-là se tient autour de 10 %. La mesure disait elle-même qu'elle ne
valait rien.

**Le critère est relatif à chaque mesure, et il le fallait.** La dispersion
médiane du journal est de 17 %, et elle va de 3,8 % pour la mise à l'échelle
d'un rationnel à 43,6 % pour un décalage de quatre mille sous-titres. Un seuil
unique à 10 % refuserait plus d'une mesure sur deux. Ce qui compte n'est pas
qu'une mesure soit dispersée, c'est qu'elle le soit **plus que d'habitude**.

**Le facteur vaut un quart au-dessus de la médiane**, et c'est là que la courbe
tourne. Mesuré sur les 87 relevés du journal, en comptant les maxima posés et
ceux que les cinq relevés suivants démentent :

| Critère | Maxima posés | Démentis |
| :------ | -----------: | -------: |
| charge seule — la règle d'avant | 134 | **11** (8,2 %) |
| charge, et dispersion ≤ 1,0 × sa médiane | 119 | 4 |
| **charge, et dispersion ≤ 1,25 × sa médiane** | **126** | **4** (3,2 %) |
| charge, et dispersion ≤ 1,5 × sa médiane | 121 | 5 |
| charge, et dispersion ≤ 2,0 × sa médiane | 131 | 6 |

**La pointe n'était pas la bonne lunette.** L'instrument comptait ce qui dépasse
d'une moitié ses voisines ; un maximum se pose en dépassant le précédent d'un
cheveu, et il est définitif. Le relevé de #202 valait ×1,15 : jamais une pointe,
et pourtant une enveloppe faussée pour toujours. `analyse-bench-journal.py`
compte désormais les deux.

**Le second critère s'ajoute au premier, il ne le remplace pas.** La dispersion
seule est nettement moins bonne — 24 pointes admises contre 5 — et la charge se
lit **avant** de mesurer, ce qui permet de renoncer ; la dispersion ne se connaît
qu'après.

**En deçà de cinq relevés, une mesure n'a pas de médiane** et le critère ne
s'applique pas : la charge reste seule juge, et la règle de #189 tient déjà les
mesures neuves hors de la table jusqu'au premier relevé calme.

### Ce que la règle a réparé derrière elle

Une règle qui se resserre ne répare pas le passé toute seule : **un maximum ne
tombe que devant un maximum plus haut.** Vingt-quatre des soixante extrêmes de
la table avaient été posés par un relevé que les deux critères refusent, et ils
ont été recalculés — `src/scripts/recompute-bench-extremes.py`, qui ne touche
qu'un extrême dont **le relevé d'origine** est refusé, et laisse ceux dont la
source a été élaguée.

Sept sont dans ce cas : ni jugeables ni remplaçables, laissés tels quels. Le
dire vaut mieux que de recalculer sur une histoire tronquée.

`make bench` attend d'abord que la charge redescende, **trente secondes au
plus** : la cause la plus fréquente est la cible qui précède, et la moyenne
d'une minute met une minute à l'oublier. Passé ce délai il mesure quand même —
les chiffres restent utiles à qui vient de lancer la commande — mais il ne verse
rien, et il le dit.

### Pourquoi trente secondes, et pourquoi ne rien écrire — #270

La phase 7 a donné treize relevés dont **six au-dessus du seuil**. La question
posée était : le seuil est-il tenable dans le déroulé prescrit ?

**Ce qui a été mesuré, et ce que ça a corrigé.** Première idée, fausse : que la
charge vienne des deux étapes qui précèdent `bench` dans `check-local`. Relevée
à la sortie d'`install-check`, toutes les cinq secondes, elle culmine à 1,78
cinq secondes après la fin — la moyenne d'une minute retarde sur ce qui vient de
finir — et repasse sous le seuil en une vingtaine de secondes. `e2e` et
`install-check` sont courts ; ils ne chargent presque rien.

**Ce qui charge est ce qui précède le tout.** Le déroulé prescrit enchaîne
`make check` puis `make check-local` : un quart d'heure de clang-tidy, de tests
sous sanitizers et de couverture, immédiatement avant. C'est cet héritage-là que
`bench` trouve, et non celui de ses deux voisines. Quatre exécutions réelles, le
même jour et sur la même machine :

| Contexte de l'exécution | Charge à `bench` | Sort |
| :---------------------- | ---------------: | :--- |
| `check-local` dans la foulée de `make check` | 2,78 | refusé |
| idem, une seconde fois | 1,84 | refusé |
| `check-local` seul, machine au repos | 1,38 | **inscrit** |
| `check-local` pendant une session de bureau active | 2,88 | refusé |
| idem, plus chargée | 6,56 | refusé |

Le relevé de la version `0.8.6` est celui de la troisième ligne, et il a survécu
aux deux exécutions refusées qui l'ont suivi : **un refus n'écrit pas, donc il
n'efface pas non plus** la mesure propre déjà en place.

Les deux dernières ne sont pas des cas du projet : les processus en tête de `ps`
n'appartenaient pas à ce dépôt — un navigateur, un environnement de
développement, deux serveurs — et la charge est retombée à 0,64 dès que la
session s'est calmée, sans que rien du projet n'ait changé.

**Le seuil est donc tenable, et ce n'est pas lui qu'il fallait regarder.** Une
mesure prise hors de la traîne de `make check`, sur une machine qui n'est qu'à
nous, passe. Les six échecs de la phase 7 étaient l'une ou l'autre des deux
causes ci-dessus, et aucune ne se soigne en attendant plus longtemps : la traîne
d'un quart d'heure de compilation et la session de bureau durent toutes deux
plus que n'importe quel délai qu'on accepterait de payer à chaque exécution.

**D'où trente secondes.** Ce qui se rattrape se rattrape en vingt ; ce qui ne se
rattrape pas ne se rattrape pas en trois minutes non plus. Attendre au-delà,
c'était payer le délai pour les seuls cas où il ne pouvait rien.

**Et l'inscription sans droit d'extrême était une demi-mesure au sens propre.**
Le journal a une section par version : une section qu'on ne peut comparer à rien
— puisqu'on ignore ce qui, dedans, vient du code — occupe la place de celle qui
manque, et **cache** le fait qu'il n'y a pas eu de mesure. Une version sans
section le dit. Presque un relevé sur deux de la phase 7 était dans ce cas.

### La place du banc dans le déroulé, tranchée — #307

#270 laissait cette question ouverte, en disant qu'elle le dépassait : le banc
est enchaîné par `check-local`, donc juste derrière la seule étape qui chauffe
vraiment la machine. **La phase 8 a mesuré ce que cela coûte**, et la relecture
de fin de phase l'a tranché.

| Phase | Relevés versés | Refusés pour charge |
| :---- | -------------: | ------------------: |
| 7 | 7 sur 13 | 6 |
| 8 | 3 sur 6 | 3 — charges 1,59 · 2,08 · 10,64 |

Les deux premières charges de la phase 8 sont la traîne de `make check`, celle
que #270 avait mesurée à 2,78 puis 1,84 dans les mêmes conditions. La troisième
est une session de bureau, que rien du projet ne peut soigner.

**Première décision : l'échange des deux cibles**, `make check-local` d'abord,
`make check` ensuite — le banc trouve une machine que rien n'a chauffée, et
c'est aussi l'ordre que le principe « du moins cher au plus cher » voulait
déjà.

**Elle ne suffit pas, et sa première exécution l'a dit.** Machine partie de
0,63, nouvel ordre : le banc a lu **2,21** et n'a rien inscrit. La traîne de
`make check` n'était donc pas toute la cause.

### Le garde lisait la charge après l'avoir créée

Deux exécutions à une minute d'intervalle, le même jour et sur la même machine :

| Exécution | Charge lue | Sort |
| :-------- | ---------: | :--- |
| `check-local` complet, avec reconstruction Release | 2,21 | refusé |
| `make bench` seul, arbre Release déjà à jour | **1,32** | **inscrit** |

**La seconde partait d'une machine plus chargée, et s'est inscrite.** Ce qui les
sépare est la construction Release que l'étape du banc lance elle-même, juste
avant de demander si la machine est libre.

`/proc/loadavg` donne une moyenne d'**une minute**, c'est-à-dire le passé. Lue
après une compilation, elle répond sur cette compilation. **Le seuil n'a jamais
eu tort, ni le délai : on posait la question au seul moment où le garde ne
pouvait pas répondre.**

La lecture passe donc avant la construction. Le garde répond désormais « la
machine était-elle à nous quand on a commencé », ce qui est la bonne question :
une charge concurrente — navigateur, environnement de développement, la session
de bureau à 10,64 — est là avant comme pendant. Ce qu'il cesse de voir est une
charge qui démarrerait pendant la construction, et c'est le critère de
dispersion qui la voit, après coup.

**Ni le seuil ni le délai ne bougent.** Aucune mesure ne les met en cause. Voir
[l'ADR 0015](../adr/0015-memoire-des-mesures.md) et `CLAUDE.md`.

La règle de la table des extrêmes, elle, ne bouge pas : elle vaut toujours pour
le seul relevé qui peut encore arriver avec une charge non qualifiée, celui
d'une machine sans `/proc/loadavg`.

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

## Deux conversions qui ne faisaient rien, en 0.9.5

Les issues #314 et #318, du même geste : ICU convertissait dans deux cas où il
n'y avait rien à convertir.

**À l'écriture, quand la cible est de l'UTF-8**, les deux convertisseurs sont le
même : le texte faisait l'aller-retour par UTF-16 pour revenir tel qu'il était
parti. C'est le cas ordinaire — la grande majorité des fichiers écrits.

| | avant | après |
| :--- | ----: | ----: |
| écriture de 4000 sous-titres | 852 µs | **~610 µs** |

**Rien n'est perdu à sauter ce passage**, et il fallait le vérifier plutôt que
le supposer : l'aller-retour n'était pas un filet. Le rappel d'arrêt est posé
sur le convertisseur de sortie, qui refuse un caractère que la cible ne sait pas
écrire ; rien n'était posé sur celui d'entrée, si bien qu'un UTF-8 malformé
entrant était remplacé par des U+FFFD et écrit — en silence. Le recopier n'est
pas pire, et le modèle ne porte que de l'UTF-8 de toute façon.

**À la lecture, le fichier était décodé deux fois** : une fois par la détection,
qui ne peut répondre sans lire, et une fois par la lecture elle-même. La
détection rend désormais le texte qu'elle a produit.

Ce gain-là **ne se voit pas au banc**, et c'est ce qu'il faut savoir de lui : il
vaut un décodage du fichier entier, mesuré à part parce que la dispersion du
relevé de lecture est plus grande que lui à cette taille.

| Taille | Un décodage |
| :----- | ----------: |
| 4 000 sous-titres, 359 Ko | **212 µs** sur 2,7 ms de lecture, soit 8 % |
| 40 000 sous-titres, 3,6 Mo | **2 195 µs** |

Il est donc proportionnel au fichier, et c'est la seule chose qui le rende digne
d'intérêt : à quatre mille répliques il disparaît dans le bruit, à quarante
mille il vaut deux millisecondes. #314 demandait précisément que la question
soit rouverte par une mesure plutôt que par le souvenir ; la voici.

## Extrêmes

Le minimum et le maximum jamais relevés pour chaque mesure. Cette table n'est
jamais élaguée — c'est elle qui garde l'enveloppe quand les relevés s'effacent.
Une mesure renommée ou retirée y garde ses extrêmes indéfiniment : rien ici ne
distingue une entrée vivante d'une orpheline — élaguer les orphelines n'est
pas le sujet de ce ticket.

<!-- extrêmes -->
| Mesure | Minimum | Relevé le | Maximum | Relevé le |
| :----- | ------: | :-------- | ------: | :-------- |
| versionString | 29.2 ns | 0.5.9 — 2026-08-23 | 55.1 ns | 0.4.4 — 2026-08-17 |
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 67.3 ns | 0.9.18 — 2026-09-08 |
| format | 29.8 ns | 0.3.11 — 2026-08-15 | 68 ns | 0.9.17 — 2026-09-08 |
| position vers image | 6.32 ns | 0.12.23 — 2026-09-30 | 8.03 ns | 0.10.1 — 2026-09-11 |
| image vers position | 6.32 ns | 0.13.25 — 2026-10-05 | 12.7 ns | 0.10.14 — 2026-09-15 |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.9.19 — 2026-09-08 | 8.3 ns | 0.10.14 — 2026-09-15 |
| lecture de 4000 sous-titres | 2.01 ms | 0.13.33 — 2026-10-06 | 3.28 ms | 0.5.3 — 2026-08-22 |
| écriture de 4000 sous-titres | 488 µs | 0.3.10 — 2026-08-15 | 825 µs | 0.9.3 — 2026-09-05 |
| décalage de 4000 sous-titres | 6.56 µs | 0.13.25 — 2026-10-05 | 11 µs | 0.9.18 — 2026-09-08 |
| décalage puis annulation | 12.7 µs | 0.13.25 — 2026-10-05 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 67.5 µs | 0.13.13 — 2026-10-02 | 94.5 µs | 0.9.18 — 2026-09-08 |
| conversion de fréquence sur 4000 sous-titres | 66.2 µs | 0.11.7 — 2026-09-21 | 100 µs | 0.2.12 — 2026-08-14 |
| tri de 4000 sous-titres à l'envers | 196 µs | 0.3.2 — 2026-08-15 | 352 µs | 0.9.18 — 2026-09-08 |
| suppression d'un sous-titre sur deux | 138 µs | 0.4.14 — 2026-08-21 | 12.5 ms | 0.3.3 — 2026-08-15 |
| insertion de 100 sous-titres vides au milieu | 48.2 µs | 0.7.5 — 2026-08-29 | 97.6 µs | 0.8.13 — 2026-09-02 |
| modification d'un texte, à travers une session | 114 ns | 0.2.14 — 2026-08-14 | 228 ns | 0.8.6 — 2026-08-31 |
| suppression des mentions sur 4000 sous-titres | 853 µs | 0.4.21 — 2026-08-22 | 5.68 ms | 0.4.4 — 2026-08-17 |
| suppression puis annulation | 213 µs | 0.4.14 — 2026-08-21 | 314 µs | 0.10.15 — 2026-09-16 |
| construction du modèle sur 4000 sous-titres | 59.3 ns | 0.4.14 — 2026-08-21 | 16.4 µs | 0.9.13 — 2026-09-07 |
| une fenêtre de 40 lignes, cinq colonnes | 14.7 µs | 0.4.15 — 2026-08-21 | 28.9 µs | 0.6.8 — 2026-08-26 |
| rafraîchir après un décalage de 4000 sous-titres | 26.4 ns | 0.4.15 — 2026-08-21 | 13.2 µs | 0.11.25 — 2026-09-25 |
| édition d'une cellule de texte | 371 ns | 0.5.9 — 2026-08-23 | 496 ns | 0.9.22 — 2026-09-10 |
| édition d'une cellule de position | 8.09 µs | 0.12.7 — 2026-09-27 | 14.8 µs | 0.8.8 — 2026-09-01 |
| réinitialisation du modèle après une ligne retirée | 7.23 µs | 0.9.20 — 2026-09-08 | 13.6 µs | 0.11.25 — 2026-09-25 |
| la réplique en cours, sur 4000 sous-titres | 4.71 µs | 0.8.13 — 2026-09-02 | 14.1 µs | 0.11.25 — 2026-09-25 |
| composer une réplique de deux lignes | 162 ns | 0.13.0 — 2026-10-01 | 284 ns | 0.13.33 — 2026-10-06 |
| ouvrir une vidéo | 8.82 ms | 0.13.33 — 2026-10-06 | 218 ms | 0.14.7 — 2026-10-07 |
| chercher une position | 538 µs | 0.6.13 — 2026-08-27 | 5.42 ms | 0.14.5 — 2026-10-06 |
| déduction de fréquence sur 4000 sous-titres | 366 µs | 0.7.15 — 2026-08-30 | 455 µs | 0.9.18 — 2026-09-08 |
| alignement sur 4000 sous-titres | 123 µs | 0.13.25 — 2026-10-05 | 156 µs | 0.10.1 — 2026-09-11 |
| mise en italique de 4000 sous-titres | 885 µs | 0.12.8 — 2026-09-27 | 1.39 ms | 0.10.10 — 2026-09-13 |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.85 ms | 0.10.17 — 2026-09-19 | 4.78 ms | 0.12.19 — 2026-09-30 |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.04 ms | 0.13.33 — 2026-10-06 | 5.13 ms | 0.14.5 — 2026-10-06 |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 0.13.33 — 2026-10-06 | 8.01 ms | 0.13.10 — 2026-10-01 |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.2 ms | 0.10.17 — 2026-09-19 | 1.45 ms | 0.13.27 — 2026-10-05 |
| casse de titre sur 4000 sous-titres | 17.8 ms | 0.13.25 — 2026-10-05 | 22 ms | 0.11.25 — 2026-09-25 |
| collage de 4000 textes | 964 µs | 0.10.17 — 2026-09-19 | 1.26 ms | 0.11.1 — 2026-09-19 |
| alignement d'une traduction de 4000 lignes, par position | 803 µs | 0.13.33 — 2026-10-06 | 1.1 ms | 0.11.25 — 2026-09-25 |
| alignement d'une traduction de 4000 lignes, par numéro | 782 µs | 0.12.7 — 2026-09-27 | 995 µs | 0.12.19 — 2026-09-30 |
| erreurs courantes du français sur 4000 sous-titres | 153 ms | 0.12.7 — 2026-09-27 | 187 ms | 0.12.19 — 2026-09-30 |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 0.13.33 — 2026-10-06 | 207 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 0.12.8 — 2026-09-27 | 132 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, avec cache de longueurs | 111 ms | 0.13.33 — 2026-10-06 | 138 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 177 ms | 0.12.9 — 2026-09-28 | 212 ms | 0.12.19 — 2026-09-30 |
| walk of 200 files in a tree of depth 3 | 550 µs | 0.13.25 — 2026-10-05 | 659 µs | 0.14.3 — 2026-10-06 |
| shift of 200 files of 20 subtitles | 32.3 ms | 0.13.25 — 2026-10-05 | 38.5 ms | 0.13.10 — 2026-10-01 |
| shift of one file of 4000 subtitles | 2.89 ms | 0.13.33 — 2026-10-06 | 3.6 ms | 0.13.10 — 2026-10-01 |
| rendre une image de 640×360 | 208 µs | 0.14.5 — 2026-10-06 | 216 µs | 0.14.7 — 2026-10-07 |
| rendre une image de 1280×720 | 724 µs | 0.14.7 — 2026-10-07 | 738 µs | 0.14.5 — 2026-10-06 |
| rendre une image de 1920×1080 | 1.41 ms | 0.14.7 — 2026-10-07 | 1.5 ms | 0.14.5 — 2026-10-06 |
| chercher l'image 0 (vidéo 720p) | 6.65 ms | 0.14.7 — 2026-10-07 | 7.02 ms | 0.14.5 — 2026-10-06 |
| chercher l'image 100 (vidéo 720p) | 38.6 ms | 0.14.7 — 2026-10-07 | 39.1 ms | 0.14.5 — 2026-10-06 |
| chercher l'image 249 (vidéo 720p) | 82.5 ms | 0.14.5 — 2026-10-06 | 85.7 ms | 0.14.7 — 2026-10-07 |
| pas avant vers l'image 100 (vidéo 720p) | 40 ms | 0.14.7 — 2026-10-07 | 40.8 ms | 0.14.5 — 2026-10-06 |
| pas avant vers l'image 249 (vidéo 720p) | 83.3 ms | 0.14.5 — 2026-10-06 | 87.6 ms | 0.14.7 — 2026-10-07 |
| pas arrière vers l'image 99 (vidéo 720p) | 39 ms | 0.14.5 — 2026-10-06 | 40.7 ms | 0.14.7 — 2026-10-07 |
| pas arrière vers l'image 248 (vidéo 720p) | 82.4 ms | 0.14.5 — 2026-10-06 | 86.5 ms | 0.14.7 — 2026-10-07 |

<!-- versionString min=29.1524 max=55.1 -->
<!-- parse min=29.9 max=67.348 -->
<!-- format min=29.8143 max=68.0218 -->
<!-- position vers image min=6.31843 max=8.0301 -->
<!-- image vers position min=6.31692 max=12.7175 -->
<!-- mise à l'échelle par un rationnel exact min=6.70967 max=8.3022 -->
<!-- lecture de 4000 sous-titres min=2014490.0 max=3280000.0 -->
<!-- écriture de 4000 sous-titres min=488279.0 max=825490.0 -->
<!-- décalage de 4000 sous-titres min=6556.44 max=11029.2 -->
<!-- décalage puis annulation min=12695.6 max=20700.0 -->
<!-- transformation de 4000 sous-titres min=67454.9 max=94496.9 -->
<!-- conversion de fréquence sur 4000 sous-titres min=66238.2 max=100000.0 -->
<!-- tri de 4000 sous-titres à l'envers min=196000.0 max=351534.0 -->
<!-- suppression d'un sous-titre sur deux min=137974.0 max=12500000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=48185.9 max=97640.4 -->
<!-- modification d'un texte, à travers une session min=114.0 max=227.528 -->
<!-- suppression des mentions sur 4000 sous-titres min=853000.0 max=5676890.0 -->
<!-- suppression puis annulation min=213280.0 max=314434.0 -->
<!-- construction du modèle sur 4000 sous-titres min=59.2521 max=16365.4 -->
<!-- une fenêtre de 40 lignes, cinq colonnes min=14699.7 max=28872.4 -->
<!-- rafraîchir après un décalage de 4000 sous-titres min=26.4 max=13151.9 -->
<!-- édition d'une cellule de texte min=370.791 max=496.387 -->
<!-- édition d'une cellule de position min=8090.8 max=14758.2 -->
<!-- réinitialisation du modèle après une ligne retirée min=7229.3 max=13625.8 -->
<!-- la réplique en cours, sur 4000 sous-titres min=4714.13 max=14131.7 -->
<!-- composer une réplique de deux lignes min=161.721 max=284.466 -->
<!-- ouvrir une vidéo min=8822490.0 max=218275000.0 -->
<!-- chercher une position min=537819.0 max=5417380.0 -->
<!-- déduction de fréquence sur 4000 sous-titres min=366479.0 max=455304.0 -->
<!-- alignement sur 4000 sous-titres min=122556.0 max=156274.0 -->
<!-- mise en italique de 4000 sous-titres min=885492.0 max=1386260.0 -->
<!-- remplacement d'un mot fréquent sur 4000 sous-titres, texte simple min=3852960.0 max=4776230.0 -->
<!-- remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière min=4040880.0 max=5128860.0 -->
<!-- recherche sans résultat sur 4000 sous-titres min=2617430.0 max=8011860.0 -->
<!-- ajustement des durées de 4000 sous-titres, quatre contraintes min=1200330.0 max=1448050.0 -->
<!-- casse de titre sur 4000 sous-titres min=17770800.0 max=21973100.0 -->
<!-- collage de 4000 textes min=964212.0 max=1259920.0 -->
<!-- alignement d'une traduction de 4000 lignes, par position min=802764.0 max=1096980.0 -->
<!-- alignement d'une traduction de 4000 lignes, par numéro min=781546.0 max=995207.0 -->
<!-- erreurs courantes du français sur 4000 sous-titres min=153145000.0 max=186776000.0 -->
<!-- erreurs courantes de l'anglais sur 4000 sous-titres min=170763000.0 max=206664000.0 -->
<!-- découpage de 4000 sous-titres, sans cache de longueurs min=109364000.0 max=131622000.0 -->
<!-- découpage de 4000 sous-titres, avec cache de longueurs min=111468000.0 max=138172000.0 -->
<!-- découpage de 4000 sous-titres, en ems, avec cache de longueurs min=177182000.0 max=212308000.0 -->
<!-- walk of 200 files in a tree of depth 3 min=550048.0 max=658528.0 -->
<!-- shift of 200 files of 20 subtitles min=32254900.0 max=38465800.0 -->
<!-- shift of one file of 4000 subtitles min=2892610.0 max=3596300.0 -->
<!-- rendre une image de 640×360 min=208485.0 max=216400.0 -->
<!-- rendre une image de 1280×720 min=723665.0 max=738399.0 -->
<!-- rendre une image de 1920×1080 min=1406670.0 max=1502450.0 -->
<!-- chercher l'image 0 (vidéo 720p) min=6654830.0 max=7018160.0 -->
<!-- chercher l'image 100 (vidéo 720p) min=38618500.0 max=39051100.0 -->
<!-- chercher l'image 249 (vidéo 720p) min=82494800.0 max=85658400.0 -->
<!-- pas avant vers l'image 100 (vidéo 720p) min=39987500.0 max=40798700.0 -->
<!-- pas avant vers l'image 249 (vidéo 720p) min=83322300.0 max=87613900.0 -->
<!-- pas arrière vers l'image 99 (vidéo 720p) min=38964900.0 max=40650100.0 -->
<!-- pas arrière vers l'image 248 (vidéo 720p) min=82354100.0 max=86546700.0 -->

## Relevés

Une section par version. Les relevés de plus d'un mois sont élagués ; leurs
extrêmes survivent dans la table ci-dessus.

<!-- relevés -->

### 0.14.7 — 2026-10-07 — Release — charge 1.46 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.31 µs | 902 ns |
| composer une réplique de deux lignes | 185 ns | 32.4 ns |
| ouvrir une vidéo | 218 ms | 1.49 ms |
| chercher une position | 5.39 ms | 103 µs |
| rendre une image de 640×360 | 216 µs | 44.6 µs |
| rendre une image de 1280×720 | 724 µs | 88.3 µs |
| rendre une image de 1920×1080 | 1.41 ms | 187 µs |
| chercher l'image 0 (vidéo 720p) | 6.65 ms | 500 µs |
| chercher l'image 100 (vidéo 720p) | 38.6 ms | 1.82 ms |
| chercher l'image 249 (vidéo 720p) | 85.7 ms | 7.89 ms |
| pas avant vers l'image 100 (vidéo 720p) | 40 ms | 5.36 ms |
| pas avant vers l'image 249 (vidéo 720p) | 87.6 ms | 10.6 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 40.7 ms | 5.32 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 86.5 ms | 9.59 ms |
| construction du modèle sur 4000 sous-titres | 8.91 µs | 4.17 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.9 µs | 2.26 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.24 µs | 2.6 µs |
| réinitialisation du modèle après une ligne retirée | 8.27 µs | 2.91 µs |
| édition d'une cellule de texte | 454 ns | 199 ns |
| édition d'une cellule de position | 9.62 µs | 2.21 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 192 ms | 11.1 ms |
| versionString | 44.7 ns | 0.614 ns |
| parse | 36.6 ns | 2.61 ns |
| format | 42.9 ns | 6.71 ns |
| position vers image | 6.65 ns | 0.0731 ns |
| image vers position | 6.64 ns | 0.046 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0636 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 116 ms | 5.85 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 123 ms | 7.92 ms |
| erreurs courantes du français sur 4000 sous-titres | 164 ms | 10 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 183 ms | 11.1 ms |
| lecture de 4000 sous-titres | 2.05 ms | 96 µs |
| écriture de 4000 sous-titres | 555 µs | 42.3 µs |
| décalage de 4000 sous-titres | 7.56 µs | 3.2 µs |
| décalage puis annulation | 15.4 µs | 4.75 µs |
| transformation de 4000 sous-titres | 72.6 µs | 11.3 µs |
| conversion de fréquence sur 4000 sous-titres | 67.2 µs | 9.06 µs |
| alignement sur 4000 sous-titres | 127 µs | 12.5 µs |
| tri de 4000 sous-titres à l'envers | 321 µs | 32.5 µs |
| suppression d'un sous-titre sur deux | 199 µs | 35.3 µs |
| suppression puis annulation | 242 µs | 24.6 µs |
| insertion de 100 sous-titres vides au milieu | 50.9 µs | 7.35 µs |
| modification d'un texte, à travers une session | 174 ns | 24.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.32 ms | 143 µs |
| mise en italique de 4000 sous-titres | 947 µs | 130 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.65 ms | 1.04 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.44 ms | 545 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 168 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.24 ms | 59.6 µs |
| casse de titre sur 4000 sous-titres | 20.1 ms | 2.02 ms |
| collage de 4000 textes | 1 ms | 55.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 922 µs | 55.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 1.02 ms | 193 µs |
| déduction de fréquence sur 4000 sous-titres | 380 µs | 13.9 µs |
| walk of 200 files in a tree of depth 3 | 639 µs | 43.6 µs |
| shift of 200 files of 20 subtitles | 35.6 ms | 3.08 ms |
| shift of one file of 4000 subtitles | 3.45 ms | 263 µs |

### 0.14.5 — 2026-10-06 — Release — charge 1.49 — allure ×1.06

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.94 µs | 1.83 µs |
| composer une réplique de deux lignes | 180 ns | 3.42 ns |
| ouvrir une vidéo | 218 ms | 1.39 ms |
| chercher une position | 5.42 ms | 72.9 µs |
| rendre une image de 640×360 | 208 µs | 30.3 µs |
| rendre une image de 1280×720 | 738 µs | 131 µs |
| rendre une image de 1920×1080 | 1.5 ms | 339 µs |
| chercher l'image 0 (vidéo 720p) | 7.02 ms | 1.17 ms |
| chercher l'image 100 (vidéo 720p) | 39.1 ms | 2.79 ms |
| chercher l'image 249 (vidéo 720p) | 82.5 ms | 2.99 ms |
| pas avant vers l'image 100 (vidéo 720p) | 40.8 ms | 6.03 ms |
| pas avant vers l'image 249 (vidéo 720p) | 83.3 ms | 5.22 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 39 ms | 4.47 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 82.4 ms | 3.16 ms |
| construction du modèle sur 4000 sous-titres | 11.3 µs | 6.42 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.8 µs | 9.41 µs |
| rafraîchir après un décalage de 4000 sous-titres | 11 µs | 4.04 µs |
| réinitialisation du modèle après une ligne retirée | 11 µs | 3.88 µs |
| édition d'une cellule de texte | 412 ns | 145 ns |
| édition d'une cellule de position | 13.8 µs | 883 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 197 ms | 11.6 ms |
| versionString | 47.9 ns | 2.36 ns |
| parse | 38.7 ns | 0.889 ns |
| format | 38.5 ns | 13 ns |
| position vers image | 6.64 ns | 0.0523 ns |
| image vers position | 7.75 ns | 0.84 ns |
| mise à l'échelle par un rationnel exact | 7.91 ns | 0.388 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 124 ms | 5.21 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 126 ms | 2.77 ms |
| erreurs courantes du français sur 4000 sous-titres | 174 ms | 8.33 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 198 ms | 14 ms |
| lecture de 4000 sous-titres | 2.37 ms | 275 µs |
| écriture de 4000 sous-titres | 653 µs | 41.7 µs |
| décalage de 4000 sous-titres | 13.3 µs | 13.5 µs |
| décalage puis annulation | 20.6 µs | 10.2 µs |
| transformation de 4000 sous-titres | 90.8 µs | 14.5 µs |
| conversion de fréquence sur 4000 sous-titres | 84.5 µs | 6.04 µs |
| alignement sur 4000 sous-titres | 156 µs | 11.3 µs |
| tri de 4000 sous-titres à l'envers | 336 µs | 31 µs |
| suppression d'un sous-titre sur deux | 409 µs | 127 µs |
| suppression puis annulation | 1.24 ms | 862 µs |
| insertion de 100 sous-titres vides au milieu | 189 µs | 116 µs |
| modification d'un texte, à travers une session | 202 ns | 23.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.46 ms | 50.2 µs |
| mise en italique de 4000 sous-titres | 1.34 ms | 747 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 5.1 ms | 818 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 5.13 ms | 321 µs |
| recherche sans résultat sur 4000 sous-titres | 4.11 ms | 520 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.49 ms | 145 µs |
| casse de titre sur 4000 sous-titres | 22.1 ms | 1.36 ms |
| collage de 4000 textes | 1.1 ms | 134 µs |
| alignement d'une traduction de 4000 lignes, par position | 887 µs | 62.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 936 µs | 136 µs |
| déduction de fréquence sur 4000 sous-titres | 400 µs | 28.3 µs |
| walk of 200 files in a tree of depth 3 | 568 µs | 38.4 µs |
| shift of 200 files of 20 subtitles | 37.1 ms | 1.9 ms |
| shift of one file of 4000 subtitles | 3.7 ms | 614 µs |

### 0.14.3 — 2026-10-06 — Release — charge 1.48 — allure ×1.10

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.94 µs | 2.74 µs |
| composer une réplique de deux lignes | 168 ns | 2.88 ns |
| ouvrir une vidéo | 10.1 ms | 721 µs |
| chercher une position | 644 µs | 103 µs |
| construction du modèle sur 4000 sous-titres | 9.99 µs | 6.36 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.4 µs | 1.96 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.7 µs | 6.33 µs |
| réinitialisation du modèle après une ligne retirée | 9.78 µs | 4.8 µs |
| édition d'une cellule de texte | 437 ns | 162 ns |
| édition d'une cellule de position | 10.6 µs | 1.56 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 191 ms | 10.7 ms |
| versionString | 39.1 ns | 0.695 ns |
| parse | 40.7 ns | 1.93 ns |
| format | 33.7 ns | 2.23 ns |
| position vers image | 6.48 ns | 0.0716 ns |
| image vers position | 7.25 ns | 0.131 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0825 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 120 ms | 7.79 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 124 ms | 7.16 ms |
| erreurs courantes du français sur 4000 sous-titres | 167 ms | 8.52 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 187 ms | 9.93 ms |
| lecture de 4000 sous-titres | 2.24 ms | 149 µs |
| écriture de 4000 sous-titres | 652 µs | 65.7 µs |
| décalage de 4000 sous-titres | 10.2 µs | 3.56 µs |
| décalage puis annulation | 19.9 µs | 4.71 µs |
| transformation de 4000 sous-titres | 90.1 µs | 13.1 µs |
| conversion de fréquence sur 4000 sous-titres | 68.5 µs | 11.2 µs |
| alignement sur 4000 sous-titres | 153 µs | 21.1 µs |
| tri de 4000 sous-titres à l'envers | 310 µs | 47.6 µs |
| suppression d'un sous-titre sur deux | 680 µs | 764 µs |
| suppression puis annulation | 250 µs | 52 µs |
| insertion de 100 sous-titres vides au milieu | 59.9 µs | 7.53 µs |
| modification d'un texte, à travers une session | 176 ns | 18 ns |
| suppression des mentions sur 4000 sous-titres | 1.47 ms | 111 µs |
| mise en italique de 4000 sous-titres | 1 ms | 77.3 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.16 ms | 326 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.43 ms | 489 µs |
| recherche sans résultat sur 4000 sous-titres | 3.18 ms | 421 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.49 ms | 261 µs |
| casse de titre sur 4000 sous-titres | 20.4 ms | 1.83 ms |
| collage de 4000 textes | 1.18 ms | 112 µs |
| alignement d'une traduction de 4000 lignes, par position | 925 µs | 45.6 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 842 µs | 68.3 µs |
| déduction de fréquence sur 4000 sous-titres | 425 µs | 17.9 µs |
| walk of 200 files in a tree of depth 3 | 659 µs | 40.8 µs |
| shift of 200 files of 20 subtitles | 36.8 ms | 2.15 ms |
| shift of one file of 4000 subtitles | 3.27 ms | 213 µs |

### 0.13.33 — 2026-10-06 — Release — charge 1.48 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.93 µs | 317 ns |
| composer une réplique de deux lignes | 284 ns | 5.57 ns |
| ouvrir une vidéo | 8.82 ms | 190 µs |
| chercher une position | 587 µs | 183 µs |
| construction du modèle sur 4000 sous-titres | 8.36 µs | 1.83 µs |
| une fenêtre de 40 lignes, cinq colonnes | 19.5 µs | 6.89 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.64 µs | 2.89 µs |
| réinitialisation du modèle après une ligne retirée | 9.16 µs | 3.71 µs |
| édition d'une cellule de texte | 395 ns | 23.7 ns |
| édition d'une cellule de position | 10.6 µs | 2.13 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 184 ms | 4.66 ms |
| versionString | 44.6 ns | 0.597 ns |
| parse | 37.6 ns | 8.08 ns |
| format | 38.6 ns | 12.2 ns |
| position vers image | 6.32 ns | 0.0485 ns |
| image vers position | 6.47 ns | 0.069 ns |
| mise à l'échelle par un rationnel exact | 7.1 ns | 2.14 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 112 ms | 4.49 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 111 ms | 3.9 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 4.84 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 5.66 ms |
| lecture de 4000 sous-titres | 2.01 ms | 59.6 µs |
| écriture de 4000 sous-titres | 527 µs | 24.2 µs |
| décalage de 4000 sous-titres | 6.78 µs | 2.78 µs |
| décalage puis annulation | 13.5 µs | 4.03 µs |
| transformation de 4000 sous-titres | 73.3 µs | 7.75 µs |
| conversion de fréquence sur 4000 sous-titres | 73.6 µs | 16.3 µs |
| alignement sur 4000 sous-titres | 126 µs | 10.3 µs |
| tri de 4000 sous-titres à l'envers | 271 µs | 19.9 µs |
| suppression d'un sous-titre sur deux | 165 µs | 25 µs |
| suppression puis annulation | 245 µs | 30.4 µs |
| insertion de 100 sous-titres vides au milieu | 53.7 µs | 14.8 µs |
| modification d'un texte, à travers une session | 177 ns | 13.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 28.8 µs |
| mise en italique de 4000 sous-titres | 905 µs | 32.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4 ms | 235 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.04 ms | 113 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 116 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.21 ms | 58.8 µs |
| casse de titre sur 4000 sous-titres | 18.3 ms | 685 µs |
| collage de 4000 textes | 1.14 ms | 133 µs |
| alignement d'une traduction de 4000 lignes, par position | 803 µs | 47.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 828 µs | 66.1 µs |
| déduction de fréquence sur 4000 sous-titres | 371 µs | 24 µs |
| walk of 200 files in a tree of depth 3 | 574 µs | 42.6 µs |
| shift of 200 files of 20 subtitles | 33.2 ms | 1.84 ms |
| shift of one file of 4000 subtitles | 2.89 ms | 114 µs |

### 0.13.27 — 2026-10-05 — Release — charge 1.39 — allure ×1.16

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.72 µs | 5.33 µs |
| composer une réplique de deux lignes | 213 ns | 4.01 ns |
| ouvrir une vidéo | 9.26 ms | 501 µs |
| chercher une position | 574 µs | 83.3 µs |
| construction du modèle sur 4000 sous-titres | 10.6 µs | 3.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 962 ns |
| rafraîchir après un décalage de 4000 sous-titres | 9.89 µs | 3.56 µs |
| réinitialisation du modèle après une ligne retirée | 10.4 µs | 7.39 µs |
| édition d'une cellule de texte | 518 ns | 284 ns |
| édition d'une cellule de position | 11.4 µs | 798 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 209 ms | 2.83 ms |
| versionString | 51.7 ns | 0.625 ns |
| parse | 43.6 ns | 2.97 ns |
| format | 40.3 ns | 0.724 ns |
| position vers image | 7.81 ns | 0.108 ns |
| image vers position | 7.63 ns | 0.171 ns |
| mise à l'échelle par un rationnel exact | 8.22 ns | 1.28 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 128 ms | 1.44 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 133 ms | 1.83 ms |
| erreurs courantes du français sur 4000 sous-titres | 179 ms | 2.45 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 200 ms | 2.2 ms |
| lecture de 4000 sous-titres | 2.44 ms | 149 µs |
| écriture de 4000 sous-titres | 628 µs | 49.4 µs |
| décalage de 4000 sous-titres | 8.2 µs | 2.54 µs |
| décalage puis annulation | 15.9 µs | 5.06 µs |
| transformation de 4000 sous-titres | 87.3 µs | 16.8 µs |
| conversion de fréquence sur 4000 sous-titres | 81.2 µs | 14.1 µs |
| alignement sur 4000 sous-titres | 147 µs | 10.5 µs |
| tri de 4000 sous-titres à l'envers | 321 µs | 43.2 µs |
| suppression d'un sous-titre sur deux | 191 µs | 24.3 µs |
| suppression puis annulation | 294 µs | 43.5 µs |
| insertion de 100 sous-titres vides au milieu | 64.5 µs | 12.3 µs |
| modification d'un texte, à travers une session | 225 ns | 115 ns |
| suppression des mentions sur 4000 sous-titres | 1.43 ms | 182 µs |
| mise en italique de 4000 sous-titres | 1.07 ms | 132 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.7 ms | 243 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.82 ms | 165 µs |
| recherche sans résultat sur 4000 sous-titres | 3.12 ms | 198 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.45 ms | 92.3 µs |
| casse de titre sur 4000 sous-titres | 21.1 ms | 437 µs |
| collage de 4000 textes | 1.21 ms | 173 µs |
| alignement d'une traduction de 4000 lignes, par position | 991 µs | 64.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 928 µs | 71.9 µs |
| déduction de fréquence sur 4000 sous-titres | 428 µs | 26.9 µs |
| walk of 200 files in a tree of depth 3 | 675 µs | 98.3 µs |
| shift of 200 files of 20 subtitles | 37.9 ms | 359 µs |
| shift of one file of 4000 subtitles | 3.58 ms | 257 µs |

### 0.13.25 — 2026-10-05 — Release — charge 1.39 — allure ×0.98

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.69 µs | 246 ns |
| composer une réplique de deux lignes | 177 ns | 2.8 ns |
| ouvrir une vidéo | 8.98 ms | 340 µs |
| chercher une position | 592 µs | 189 µs |
| construction du modèle sur 4000 sous-titres | 8.23 µs | 437 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 535 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.18 µs | 144 ns |
| réinitialisation du modèle après une ligne retirée | 8.23 µs | 756 ns |
| édition d'une cellule de texte | 404 ns | 60.7 ns |
| édition d'une cellule de position | 9.23 µs | 535 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 183 ms | 5.31 ms |
| versionString | 47.3 ns | 0.71 ns |
| parse | 35.9 ns | 2.97 ns |
| format | 35.3 ns | 0.616 ns |
| position vers image | 6.48 ns | 0.0673 ns |
| image vers position | 6.32 ns | 0.0481 ns |
| mise à l'échelle par un rationnel exact | 6.73 ns | 0.128 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 3.23 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 112 ms | 1.64 ms |
| erreurs courantes du français sur 4000 sous-titres | 157 ms | 3.6 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 172 ms | 4.41 ms |
| lecture de 4000 sous-titres | 2.03 ms | 72.8 µs |
| écriture de 4000 sous-titres | 530 µs | 44 µs |
| décalage de 4000 sous-titres | 6.56 µs | 2.74 µs |
| décalage puis annulation | 12.7 µs | 2.87 µs |
| transformation de 4000 sous-titres | 71 µs | 5.6 µs |
| conversion de fréquence sur 4000 sous-titres | 68.3 µs | 11.9 µs |
| alignement sur 4000 sous-titres | 123 µs | 6.77 µs |
| tri de 4000 sous-titres à l'envers | 276 µs | 30.4 µs |
| suppression d'un sous-titre sur deux | 160 µs | 21 µs |
| suppression puis annulation | 244 µs | 37.2 µs |
| insertion de 100 sous-titres vides au milieu | 52.1 µs | 14.6 µs |
| modification d'un texte, à travers une session | 178 ns | 16.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.21 ms | 50.2 µs |
| mise en italique de 4000 sous-titres | 906 µs | 43.2 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.92 ms | 128 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.29 ms | 247 µs |
| recherche sans résultat sur 4000 sous-titres | 2.65 ms | 93 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 59.1 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 235 µs |
| collage de 4000 textes | 978 µs | 41.8 µs |
| alignement d'une traduction de 4000 lignes, par position | 806 µs | 43.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 796 µs | 50.8 µs |
| déduction de fréquence sur 4000 sous-titres | 440 µs | 10.6 µs |
| walk of 200 files in a tree of depth 3 | 550 µs | 12.2 µs |
| shift of 200 files of 20 subtitles | 32.3 ms | 1.8 ms |
| shift of one file of 4000 subtitles | 3.12 ms | 231 µs |

### 0.13.21 — 2026-10-05 — Release — charge 1.39 — allure ×1.02

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.36 µs | 2.1 µs |
| composer une réplique de deux lignes | 169 ns | 2.88 ns |
| ouvrir une vidéo | 9.21 ms | 400 µs |
| chercher une position | 589 µs | 187 µs |
| construction du modèle sur 4000 sous-titres | 9.89 µs | 4.36 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.8 µs | 3.22 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.37 µs | 2.31 µs |
| réinitialisation du modèle après une ligne retirée | 8.49 µs | 2.53 µs |
| édition d'une cellule de texte | 393 ns | 49.1 ns |
| édition d'une cellule de position | 9.2 µs | 651 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 189 ms | 10.2 ms |
| versionString | 53.2 ns | 0.851 ns |
| parse | 39.2 ns | 10.1 ns |
| format | 39.8 ns | 0.816 ns |
| position vers image | 8.21 ns | 0.788 ns |
| image vers position | 6.65 ns | 0.0761 ns |
| mise à l'échelle par un rationnel exact | 7.04 ns | 1.22 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 121 ms | 10.6 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 134 ms | 8.93 ms |
| erreurs courantes du français sur 4000 sous-titres | 169 ms | 10.5 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 180 ms | 9.61 ms |
| lecture de 4000 sous-titres | 2.02 ms | 170 µs |
| écriture de 4000 sous-titres | 521 µs | 33.8 µs |
| décalage de 4000 sous-titres | 7.77 µs | 4.21 µs |
| décalage puis annulation | 14.5 µs | 1.61 µs |
| transformation de 4000 sous-titres | 87.9 µs | 8.03 µs |
| conversion de fréquence sur 4000 sous-titres | 77.7 µs | 11.2 µs |
| alignement sur 4000 sous-titres | 127 µs | 18.6 µs |
| tri de 4000 sous-titres à l'envers | 275 µs | 27.9 µs |
| suppression d'un sous-titre sur deux | 184 µs | 16.6 µs |
| suppression puis annulation | 283 µs | 25.1 µs |
| insertion de 100 sous-titres vides au milieu | 53 µs | 16.9 µs |
| modification d'un texte, à travers une session | 180 ns | 21.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.36 ms | 103 µs |
| mise en italique de 4000 sous-titres | 987 µs | 95.9 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.47 ms | 175 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.47 ms | 277 µs |
| recherche sans résultat sur 4000 sous-titres | 2.72 ms | 197 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.35 ms | 189 µs |
| casse de titre sur 4000 sous-titres | 19.9 ms | 1.2 ms |
| collage de 4000 textes | 991 µs | 68.2 µs |
| alignement d'une traduction de 4000 lignes, par position | 850 µs | 70.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 896 µs | 43.8 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 15.1 µs |
| walk of 200 files in a tree of depth 3 | 567 µs | 41.7 µs |
| shift of 200 files of 20 subtitles | 36.3 ms | 1.89 ms |
| shift of one file of 4000 subtitles | 3.02 ms | 300 µs |

### 0.13.14 — 2026-10-02 — Release — charge 1.41 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.49 µs | 1.36 µs |
| composer une réplique de deux lignes | 167 ns | 3.36 ns |
| ouvrir une vidéo | 9.16 ms | 551 µs |
| chercher une position | 588 µs | 183 µs |
| construction du modèle sur 4000 sous-titres | 9.39 µs | 6.64 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.4 µs | 4.5 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.57 µs | 3.55 µs |
| réinitialisation du modèle après une ligne retirée | 8.73 µs | 3.93 µs |
| édition d'une cellule de texte | 434 ns | 177 ns |
| édition d'une cellule de position | 9.84 µs | 3.39 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 186 ms | 6.39 ms |
| versionString | 46.2 ns | 9.4 ns |
| parse | 42.4 ns | 20.4 ns |
| format | 35.3 ns | 0.485 ns |
| position vers image | 6.64 ns | 0.0738 ns |
| image vers position | 6.64 ns | 0.0527 ns |
| mise à l'échelle par un rationnel exact | 7.86 ns | 0.0699 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 115 ms | 4.95 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 3.61 ms |
| erreurs courantes du français sur 4000 sous-titres | 159 ms | 6.03 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 175 ms | 5.3 ms |
| lecture de 4000 sous-titres | 2.19 ms | 164 µs |
| écriture de 4000 sous-titres | 526 µs | 36 µs |
| décalage de 4000 sous-titres | 7.82 µs | 1.4 µs |
| décalage puis annulation | 13.7 µs | 2.67 µs |
| transformation de 4000 sous-titres | 71.2 µs | 12 µs |
| conversion de fréquence sur 4000 sous-titres | 71.6 µs | 8.09 µs |
| alignement sur 4000 sous-titres | 127 µs | 15.3 µs |
| tri de 4000 sous-titres à l'envers | 285 µs | 35.8 µs |
| suppression d'un sous-titre sur deux | 173 µs | 34.1 µs |
| suppression puis annulation | 246 µs | 33.1 µs |
| insertion de 100 sous-titres vides au milieu | 53 µs | 13.3 µs |
| modification d'un texte, à travers une session | 195 ns | 40.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.28 ms | 62.8 µs |
| mise en italique de 4000 sous-titres | 930 µs | 128 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.14 ms | 296 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.3 ms | 293 µs |
| recherche sans résultat sur 4000 sous-titres | 2.63 ms | 125 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 39.3 µs |
| casse de titre sur 4000 sous-titres | 18.8 ms | 856 µs |
| collage de 4000 textes | 991 µs | 57.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 875 µs | 71.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 815 µs | 59.7 µs |
| déduction de fréquence sur 4000 sous-titres | 374 µs | 25.9 µs |
| walk of 200 files in a tree of depth 3 | 571 µs | 35.9 µs |
| shift of 200 files of 20 subtitles | 34.6 ms | 1.57 ms |
| shift of one file of 4000 subtitles | 2.94 ms | 195 µs |

### 0.13.13 — 2026-10-02 — Release — charge 1.39 — allure ×0.92

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.3 µs | 1.73 µs |
| composer une réplique de deux lignes | 205 ns | 75.5 ns |
| ouvrir une vidéo | 9.24 ms | 461 µs |
| chercher une position | 588 µs | 190 µs |
| construction du modèle sur 4000 sous-titres | 7.32 µs | 2.7 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.5 µs | 6.39 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.32 µs | 2.75 µs |
| réinitialisation du modèle après une ligne retirée | 7.33 µs | 2.97 µs |
| édition d'une cellule de texte | 393 ns | 58.3 ns |
| édition d'une cellule de position | 8.21 µs | 930 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 179 ms | 5.69 ms |
| versionString | 44.4 ns | 0.599 ns |
| parse | 41.4 ns | 3.07 ns |
| format | 34.2 ns | 0.447 ns |
| position vers image | 6.32 ns | 0.0681 ns |
| image vers position | 6.47 ns | 0.073 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0507 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 113 ms | 4.29 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 116 ms | 4.24 ms |
| erreurs courantes du français sur 4000 sous-titres | 158 ms | 5.18 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 176 ms | 5.21 ms |
| lecture de 4000 sous-titres | 2.15 ms | 185 µs |
| écriture de 4000 sous-titres | 545 µs | 70.7 µs |
| décalage de 4000 sous-titres | 6.63 µs | 2.57 µs |
| décalage puis annulation | 12.7 µs | 2.95 µs |
| transformation de 4000 sous-titres | 67.5 µs | 5.73 µs |
| conversion de fréquence sur 4000 sous-titres | 66.5 µs | 7.2 µs |
| alignement sur 4000 sous-titres | 124 µs | 7.61 µs |
| tri de 4000 sous-titres à l'envers | 273 µs | 26.5 µs |
| suppression d'un sous-titre sur deux | 163 µs | 22.7 µs |
| suppression puis annulation | 243 µs | 30.1 µs |
| insertion de 100 sous-titres vides au milieu | 52.8 µs | 14.7 µs |
| modification d'un texte, à travers une session | 173 ns | 18.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.28 ms | 60.4 µs |
| mise en italique de 4000 sous-titres | 952 µs | 93.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.08 ms | 267 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.24 ms | 323 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 133 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 54.9 µs |
| casse de titre sur 4000 sous-titres | 18.6 ms | 938 µs |
| collage de 4000 textes | 1.09 ms | 125 µs |
| alignement d'une traduction de 4000 lignes, par position | 819 µs | 55.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 809 µs | 60.9 µs |
| déduction de fréquence sur 4000 sous-titres | 377 µs | 29.6 µs |
| walk of 200 files in a tree of depth 3 | 593 µs | 45.3 µs |
| shift of 200 files of 20 subtitles | 34.2 ms | 1.66 ms |
| shift of one file of 4000 subtitles | 3.24 ms | 387 µs |

### 0.13.10 — 2026-10-01 — Release — charge 1.46 — allure ×1.06

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.91 µs | 1.94 µs |
| composer une réplique de deux lignes | 166 ns | 3.61 ns |
| ouvrir une vidéo | 9.11 ms | 471 µs |
| chercher une position | 596 µs | 166 µs |
| construction du modèle sur 4000 sous-titres | 8.45 µs | 2.92 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.3 µs | 5.45 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.39 µs | 2.21 µs |
| réinitialisation du modèle après une ligne retirée | 8.39 µs | 2.46 µs |
| édition d'une cellule de texte | 461 ns | 57 ns |
| édition d'une cellule de position | 9.34 µs | 676 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 196 ms | 6.56 ms |
| versionString | 44.3 ns | 0.874 ns |
| parse | 41.7 ns | 3.28 ns |
| format | 41.2 ns | 1.28 ns |
| position vers image | 7.62 ns | 0.0821 ns |
| image vers position | 7.62 ns | 0.0795 ns |
| mise à l'échelle par un rationnel exact | 7.88 ns | 0.115 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 120 ms | 6.3 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 123 ms | 8.3 ms |
| erreurs courantes du français sur 4000 sous-titres | 167 ms | 7.1 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 194 ms | 16.3 ms |
| lecture de 4000 sous-titres | 2.52 ms | 188 µs |
| écriture de 4000 sous-titres | 630 µs | 44.8 µs |
| décalage de 4000 sous-titres | 9.61 µs | 3.89 µs |
| décalage puis annulation | 17.9 µs | 2.31 µs |
| transformation de 4000 sous-titres | 85.3 µs | 7.25 µs |
| conversion de fréquence sur 4000 sous-titres | 79.9 µs | 5.58 µs |
| alignement sur 4000 sous-titres | 152 µs | 8.42 µs |
| tri de 4000 sous-titres à l'envers | 339 µs | 52.4 µs |
| suppression d'un sous-titre sur deux | 528 µs | 185 µs |
| suppression puis annulation | 1.9 ms | 1.95 ms |
| insertion de 100 sous-titres vides au milieu | 420 µs | 307 µs |
| modification d'un texte, à travers une session | 581 ns | 142 ns |
| suppression des mentions sur 4000 sous-titres | 3.18 ms | 303 µs |
| mise en italique de 4000 sous-titres | 3.36 ms | 1.02 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 14 ms | 9.31 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 11.9 ms | 1.36 ms |
| recherche sans résultat sur 4000 sous-titres | 8.01 ms | 461 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 4.03 ms | 493 µs |
| casse de titre sur 4000 sous-titres | 55.6 ms | 4.42 ms |
| collage de 4000 textes | 1.12 ms | 70 µs |
| alignement d'une traduction de 4000 lignes, par position | 954 µs | 172 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 948 µs | 173 µs |
| déduction de fréquence sur 4000 sous-titres | 449 µs | 69.8 µs |
| walk of 200 files in a tree of depth 3 | 652 µs | 78.6 µs |
| shift of 200 files of 20 subtitles | 38.5 ms | 2.83 ms |
| shift of one file of 4000 subtitles | 3.6 ms | 333 µs |

### 0.13.0 — 2026-10-01 — Release — charge 0.85 — allure ×1.02

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 8.76 µs | 1.9 µs |
| composer une réplique de deux lignes | 162 ns | 3.92 ns |
| ouvrir une vidéo | 9.19 ms | 690 µs |
| chercher une position | 624 µs | 155 µs |
| construction du modèle sur 4000 sous-titres | 10.2 µs | 4.78 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.2 µs | 4.54 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.77 µs | 1.89 µs |
| réinitialisation du modèle après une ligne retirée | 9.15 µs | 7.72 µs |
| édition d'une cellule de texte | 416 ns | 139 ns |
| édition d'une cellule de position | 11.5 µs | 1.91 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 202 ms | 14.4 ms |
| versionString | 44.7 ns | 0.674 ns |
| parse | 36.9 ns | 3.21 ns |
| format | 37.5 ns | 0.4 ns |
| position vers image | 7.31 ns | 0.102 ns |
| image vers position | 7.86 ns | 2.14 ns |
| mise à l'échelle par un rationnel exact | 7.43 ns | 2.49 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 123 ms | 12.4 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 125 ms | 7.31 ms |
| erreurs courantes du français sur 4000 sous-titres | 169 ms | 11.7 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 188 ms | 15.8 ms |
| lecture de 4000 sous-titres | 2.38 ms | 223 µs |
| écriture de 4000 sous-titres | 590 µs | 65.6 µs |
| décalage de 4000 sous-titres | 7.47 µs | 2.86 µs |
| décalage puis annulation | 12.8 µs | 3.43 µs |
| transformation de 4000 sous-titres | 72.2 µs | 7.72 µs |
| conversion de fréquence sur 4000 sous-titres | 69.2 µs | 11.3 µs |
| alignement sur 4000 sous-titres | 126 µs | 13.9 µs |
| tri de 4000 sous-titres à l'envers | 310 µs | 23.5 µs |
| suppression d'un sous-titre sur deux | 172 µs | 41.8 µs |
| suppression puis annulation | 272 µs | 53 µs |
| insertion de 100 sous-titres vides au milieu | 52.4 µs | 15.7 µs |
| modification d'un texte, à travers une session | 175 ns | 16.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.39 ms | 72.6 µs |
| mise en italique de 4000 sous-titres | 1.03 ms | 90.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 5.21 ms | 992 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.71 ms | 509 µs |
| recherche sans résultat sur 4000 sous-titres | 3.03 ms | 318 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 56.1 µs |
| casse de titre sur 4000 sous-titres | 20.1 ms | 1.09 ms |
| collage de 4000 textes | 1.1 ms | 27.4 µs |
| alignement d'une traduction de 4000 lignes, par position | 811 µs | 44.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 802 µs | 50.1 µs |
| déduction de fréquence sur 4000 sous-titres | 429 µs | 21 µs |

### 0.12.23 — 2026-09-30 — Release — charge 1.46 — allure ×0.97

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.23 µs | 174 ns |
| composer une réplique de deux lignes | 162 ns | 4.02 ns |
| ouvrir une vidéo | 9.29 ms | 643 µs |
| chercher une position | 575 µs | 168 µs |
| construction du modèle sur 4000 sous-titres | 9.8 µs | 4.99 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.5 µs | 5.87 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.57 µs | 3.75 µs |
| réinitialisation du modèle après une ligne retirée | 9.54 µs | 1.48 µs |
| édition d'une cellule de texte | 426 ns | 166 ns |
| édition d'une cellule de position | 10.3 µs | 3.94 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 188 ms | 6.09 ms |
| versionString | 43.9 ns | 0.557 ns |
| parse | 36.7 ns | 2.58 ns |
| format | 38.1 ns | 0.751 ns |
| position vers image | 6.32 ns | 0.0477 ns |
| image vers position | 6.47 ns | 0.0669 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0752 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 114 ms | 3.35 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 117 ms | 4.03 ms |
| erreurs courantes du français sur 4000 sous-titres | 162 ms | 3.95 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 178 ms | 7.37 ms |
| lecture de 4000 sous-titres | 2.11 ms | 71.3 µs |
| écriture de 4000 sous-titres | 518 µs | 13.6 µs |
| décalage de 4000 sous-titres | 7.04 µs | 3.63 µs |
| décalage puis annulation | 13.6 µs | 3.81 µs |
| transformation de 4000 sous-titres | 73.7 µs | 5.69 µs |
| conversion de fréquence sur 4000 sous-titres | 77.3 µs | 11 µs |
| alignement sur 4000 sous-titres | 130 µs | 15.6 µs |
| tri de 4000 sous-titres à l'envers | 286 µs | 34.7 µs |
| suppression d'un sous-titre sur deux | 166 µs | 16.1 µs |
| suppression puis annulation | 249 µs | 25 µs |
| insertion de 100 sous-titres vides au milieu | 55.1 µs | 19.7 µs |
| modification d'un texte, à travers une session | 178 ns | 11.8 ns |
| suppression des mentions sur 4000 sous-titres | 1.24 ms | 44 µs |
| mise en italique de 4000 sous-titres | 945 µs | 44.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.08 ms | 80.7 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.26 ms | 91.9 µs |
| recherche sans résultat sur 4000 sous-titres | 2.9 ms | 407 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.28 ms | 44.2 µs |
| casse de titre sur 4000 sous-titres | 18.9 ms | 711 µs |
| collage de 4000 textes | 1.01 ms | 56.7 µs |
| alignement d'une traduction de 4000 lignes, par position | 836 µs | 36.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 820 µs | 31.2 µs |
| déduction de fréquence sur 4000 sous-titres | 373 µs | 21.9 µs |

### 0.12.19 — 2026-09-30 — Release — charge 1.44 — allure ×1.20

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.01 µs | 1.19 µs |
| composer une réplique de deux lignes | 203 ns | 4.38 ns |
| ouvrir une vidéo | 9.47 ms | 578 µs |
| chercher une position | 574 µs | 115 µs |
| construction du modèle sur 4000 sous-titres | 8.71 µs | 1.58 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.9 µs | 1.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.85 µs | 3.69 µs |
| réinitialisation du modèle après une ligne retirée | 8.74 µs | 2.3 µs |
| édition d'une cellule de texte | 496 ns | 49.9 ns |
| édition d'une cellule de position | 10.5 µs | 2.39 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 212 ms | 2.83 ms |
| versionString | 55 ns | 4.58 ns |
| parse | 43.8 ns | 2.76 ns |
| format | 44.3 ns | 0.98 ns |
| position vers image | 8.15 ns | 0.623 ns |
| image vers position | 8.03 ns | 0.221 ns |
| mise à l'échelle par un rationnel exact | 8.12 ns | 1.61 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 132 ms | 4.42 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 138 ms | 6.54 ms |
| erreurs courantes du français sur 4000 sous-titres | 187 ms | 4.35 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 207 ms | 6.74 ms |
| lecture de 4000 sous-titres | 2.47 ms | 243 µs |
| écriture de 4000 sous-titres | 708 µs | 241 µs |
| décalage de 4000 sous-titres | 9.47 µs | 2.34 µs |
| décalage puis annulation | 18.2 µs | 4.32 µs |
| transformation de 4000 sous-titres | 91 µs | 15.7 µs |
| conversion de fréquence sur 4000 sous-titres | 85.3 µs | 8.91 µs |
| alignement sur 4000 sous-titres | 159 µs | 23.8 µs |
| tri de 4000 sous-titres à l'envers | 402 µs | 116 µs |
| suppression d'un sous-titre sur deux | 201 µs | 24.3 µs |
| suppression puis annulation | 291 µs | 34.2 µs |
| insertion de 100 sous-titres vides au milieu | 68.8 µs | 19.1 µs |
| modification d'un texte, à travers une session | 332 ns | 185 ns |
| suppression des mentions sur 4000 sous-titres | 1.46 ms | 88 µs |
| mise en italique de 4000 sous-titres | 1.12 ms | 150 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.78 ms | 220 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 5.55 ms | 1.32 ms |
| recherche sans résultat sur 4000 sous-titres | 3.71 ms | 697 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.51 ms | 149 µs |
| casse de titre sur 4000 sous-titres | 21.8 ms | 539 µs |
| collage de 4000 textes | 1.29 ms | 201 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.04 ms | 137 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 995 µs | 72.1 µs |
| déduction de fréquence sur 4000 sous-titres | 451 µs | 32.1 µs |

### 0.12.12 — 2026-09-29 — Release — charge 1.39 — allure ×1.05

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.88 µs | 1.77 µs |
| composer une réplique de deux lignes | 179 ns | 2.61 ns |
| ouvrir une vidéo | 9.31 ms | 626 µs |
| chercher une position | 610 µs | 184 µs |
| construction du modèle sur 4000 sous-titres | 8.44 µs | 2.81 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.1 µs | 3.22 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.49 µs | 2.64 µs |
| réinitialisation du modèle après une ligne retirée | 7.27 µs | 2.57 µs |
| édition d'une cellule de texte | 463 ns | 99.3 ns |
| édition d'une cellule de position | 8.48 µs | 1.95 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 191 ms | 5.09 ms |
| versionString | 44.4 ns | 1.02 ns |
| parse | 37.6 ns | 2.63 ns |
| format | 40.1 ns | 15.6 ns |
| position vers image | 7.14 ns | 2.51 ns |
| image vers position | 6.83 ns | 1.11 ns |
| mise à l'échelle par un rationnel exact | 6.88 ns | 0.127 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 118 ms | 3.67 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 122 ms | 3.74 ms |
| erreurs courantes du français sur 4000 sous-titres | 166 ms | 4.49 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 184 ms | 4.73 ms |
| lecture de 4000 sous-titres | 2.24 ms | 174 µs |
| écriture de 4000 sous-titres | 589 µs | 24.8 µs |
| décalage de 4000 sous-titres | 6.59 µs | 3.1 µs |
| décalage puis annulation | 14.7 µs | 2.91 µs |
| transformation de 4000 sous-titres | 72.1 µs | 13 µs |
| conversion de fréquence sur 4000 sous-titres | 76.4 µs | 7.69 µs |
| alignement sur 4000 sous-titres | 136 µs | 25.6 µs |
| tri de 4000 sous-titres à l'envers | 302 µs | 23.6 µs |
| suppression d'un sous-titre sur deux | 189 µs | 24.8 µs |
| suppression puis annulation | 255 µs | 32.3 µs |
| insertion de 100 sous-titres vides au milieu | 56 µs | 22.1 µs |
| modification d'un texte, à travers une session | 179 ns | 24.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.37 ms | 80 µs |
| mise en italique de 4000 sous-titres | 988 µs | 71.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.23 ms | 206 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.42 ms | 235 µs |
| recherche sans résultat sur 4000 sous-titres | 3.01 ms | 173 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.35 ms | 84.2 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 862 µs |
| collage de 4000 textes | 1.04 ms | 125 µs |
| alignement d'une traduction de 4000 lignes, par position | 872 µs | 66.1 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 832 µs | 51.7 µs |
| déduction de fréquence sur 4000 sous-titres | 374 µs | 20.5 µs |

### 0.12.9 — 2026-09-28 — Release — charge 1.41 — allure ×1.01

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.28 µs | 674 ns |
| composer une réplique de deux lignes | 196 ns | 77.9 ns |
| ouvrir une vidéo | 8.96 ms | 183 µs |
| chercher une position | 592 µs | 171 µs |
| construction du modèle sur 4000 sous-titres | 8.18 µs | 614 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.1 µs | 258 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.16 µs | 649 ns |
| réinitialisation du modèle après une ligne retirée | 8.15 µs | 617 ns |
| édition d'une cellule de texte | 382 ns | 27.5 ns |
| édition d'une cellule de position | 9.58 µs | 2.51 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 177 ms | 2.98 ms |
| versionString | 39.3 ns | 0.479 ns |
| parse | 38.3 ns | 2.68 ns |
| format | 36 ns | 0.452 ns |
| position vers image | 6.65 ns | 0.0708 ns |
| image vers position | 6.64 ns | 0.0526 ns |
| mise à l'échelle par un rationnel exact | 6.87 ns | 0.0726 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 1.52 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 3.36 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 4.2 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 179 ms | 9.96 ms |
| lecture de 4000 sous-titres | 2.04 ms | 117 µs |
| écriture de 4000 sous-titres | 560 µs | 76.5 µs |
| décalage de 4000 sous-titres | 6.83 µs | 3.07 µs |
| décalage puis annulation | 12.8 µs | 3.61 µs |
| transformation de 4000 sous-titres | 78.7 µs | 14.1 µs |
| conversion de fréquence sur 4000 sous-titres | 76.1 µs | 11.4 µs |
| alignement sur 4000 sous-titres | 126 µs | 11.6 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 23.7 µs |
| suppression d'un sous-titre sur deux | 164 µs | 21.3 µs |
| suppression puis annulation | 247 µs | 33.2 µs |
| insertion de 100 sous-titres vides au milieu | 56.8 µs | 21.3 µs |
| modification d'un texte, à travers une session | 171 ns | 14.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.55 ms | 103 µs |
| mise en italique de 4000 sous-titres | 946 µs | 56.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.14 ms | 238 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.28 ms | 215 µs |
| recherche sans résultat sur 4000 sous-titres | 2.73 ms | 135 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.28 ms | 56.7 µs |
| casse de titre sur 4000 sous-titres | 18.5 ms | 763 µs |
| collage de 4000 textes | 1.01 ms | 60 µs |
| alignement d'une traduction de 4000 lignes, par position | 814 µs | 43.1 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 899 µs | 51.6 µs |
| déduction de fréquence sur 4000 sous-titres | 382 µs | 36.9 µs |

### 0.12.8 — 2026-09-27 — Release — charge 1.18 — allure ×0.98

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.59 µs | 1.4 µs |
| composer une réplique de deux lignes | 179 ns | 2.2 ns |
| ouvrir une vidéo | 8.95 ms | 177 µs |
| chercher une position | 563 µs | 146 µs |
| construction du modèle sur 4000 sous-titres | 10.9 µs | 2.24 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.1 µs | 266 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.5 µs | 1.73 µs |
| réinitialisation du modèle après une ligne retirée | 10.5 µs | 2.12 µs |
| édition d'une cellule de texte | 412 ns | 77.5 ns |
| édition d'une cellule de position | 11.7 µs | 1.31 µs |
| versionString | 41.2 ns | 8.61 ns |
| parse | 36.8 ns | 2.53 ns |
| format | 36.3 ns | 0.44 ns |
| position vers image | 6.66 ns | 0.195 ns |
| image vers position | 6.65 ns | 0.147 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0506 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 3.52 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 116 ms | 7.2 ms |
| erreurs courantes du français sur 4000 sous-titres | 161 ms | 9.4 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 176 ms | 11.5 ms |
| lecture de 4000 sous-titres | 2.04 ms | 55.3 µs |
| écriture de 4000 sous-titres | 525 µs | 26 µs |
| décalage de 4000 sous-titres | 7.74 µs | 3.1 µs |
| décalage puis annulation | 15 µs | 3.94 µs |
| transformation de 4000 sous-titres | 75.7 µs | 10.2 µs |
| conversion de fréquence sur 4000 sous-titres | 70.9 µs | 8.26 µs |
| alignement sur 4000 sous-titres | 130 µs | 15.9 µs |
| tri de 4000 sous-titres à l'envers | 268 µs | 30.8 µs |
| suppression d'un sous-titre sur deux | 160 µs | 24.6 µs |
| suppression puis annulation | 239 µs | 31.3 µs |
| insertion de 100 sous-titres vides au milieu | 55.5 µs | 20.6 µs |
| modification d'un texte, à travers une session | 181 ns | 45.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 47.9 µs |
| mise en italique de 4000 sous-titres | 885 µs | 33 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.9 ms | 81.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.09 ms | 89.5 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 86.6 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 67.9 µs |
| casse de titre sur 4000 sous-titres | 18.4 ms | 809 µs |
| collage de 4000 textes | 980 µs | 34.9 µs |
| alignement d'une traduction de 4000 lignes, par position | 828 µs | 39.3 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 806 µs | 34.3 µs |
| déduction de fréquence sur 4000 sous-titres | 369 µs | 11.1 µs |

### 0.12.7 — 2026-09-27 — Release — charge 1.08 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.08 µs | 214 ns |
| composer une réplique de deux lignes | 162 ns | 2.03 ns |
| ouvrir une vidéo | 9.01 ms | 351 µs |
| chercher une position | 563 µs | 180 µs |
| construction du modèle sur 4000 sous-titres | 7.13 µs | 494 ns |
| une fenêtre de 40 lignes, cinq colonnes | 16.8 µs | 357 ns |
| rafraîchir après un décalage de 4000 sous-titres | 7.39 µs | 3.24 µs |
| réinitialisation du modèle après une ligne retirée | 7.81 µs | 1.72 µs |
| édition d'une cellule de texte | 395 ns | 38.6 ns |
| édition d'une cellule de position | 8.09 µs | 318 ns |
| versionString | 39.4 ns | 0.58 ns |
| parse | 37 ns | 2.72 ns |
| format | 34 ns | 5.89 ns |
| position vers image | 6.64 ns | 0.0686 ns |
| image vers position | 6.64 ns | 0.0699 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0536 ns |
| erreurs courantes du français sur 4000 sous-titres | 153 ms | 6.72 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 8.26 ms |
| lecture de 4000 sous-titres | 2.24 ms | 157 µs |
| écriture de 4000 sous-titres | 503 µs | 16.8 µs |
| décalage de 4000 sous-titres | 6.69 µs | 3.01 µs |
| décalage puis annulation | 12.9 µs | 3.37 µs |
| transformation de 4000 sous-titres | 70 µs | 5.86 µs |
| conversion de fréquence sur 4000 sous-titres | 68.4 µs | 9.99 µs |
| alignement sur 4000 sous-titres | 140 µs | 4.23 µs |
| tri de 4000 sous-titres à l'envers | 298 µs | 21 µs |
| suppression d'un sous-titre sur deux | 183 µs | 19.9 µs |
| suppression puis annulation | 243 µs | 39.7 µs |
| insertion de 100 sous-titres vides au milieu | 59.1 µs | 12 µs |
| modification d'un texte, à travers une session | 172 ns | 6.41 ns |
| suppression des mentions sur 4000 sous-titres | 1.37 ms | 117 µs |
| mise en italique de 4000 sous-titres | 903 µs | 57.9 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.46 ms | 127 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 268 µs |
| recherche sans résultat sur 4000 sous-titres | 2.87 ms | 182 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.29 ms | 98.6 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 718 µs |
| collage de 4000 textes | 977 µs | 42.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 909 µs | 39.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 782 µs | 36.2 µs |
| déduction de fréquence sur 4000 sous-titres | 426 µs | 25.5 µs |

### 0.12.0 — 2026-09-26 — Release — charge 1.29 — allure ×0.99

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.94 µs | 2.91 µs |
| composer une réplique de deux lignes | 169 ns | 3.44 ns |
| ouvrir une vidéo | 9.24 ms | 665 µs |
| chercher une position | 563 µs | 143 µs |
| construction du modèle sur 4000 sous-titres | 7.29 µs | 2.53 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.3 µs | 7.31 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.48 µs | 2.29 µs |
| réinitialisation du modèle après une ligne retirée | 7.58 µs | 3.41 µs |
| édition d'une cellule de texte | 380 ns | 49.2 ns |
| édition d'une cellule de position | 8.27 µs | 1.37 µs |
| versionString | 37.9 ns | 7.85 ns |
| parse | 39.1 ns | 11.3 ns |
| format | 39.4 ns | 1.76 ns |
| position vers image | 7.81 ns | 2.51 ns |
| image vers position | 6.64 ns | 0.0412 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0783 ns |
| lecture de 4000 sous-titres | 2.35 ms | 296 µs |
| écriture de 4000 sous-titres | 521 µs | 43.1 µs |
| décalage de 4000 sous-titres | 7.72 µs | 3.14 µs |
| décalage puis annulation | 13.5 µs | 4.8 µs |
| transformation de 4000 sous-titres | 80.8 µs | 9.58 µs |
| conversion de fréquence sur 4000 sous-titres | 70.1 µs | 12.1 µs |
| alignement sur 4000 sous-titres | 137 µs | 23.6 µs |
| tri de 4000 sous-titres à l'envers | 299 µs | 19.3 µs |
| suppression d'un sous-titre sur deux | 171 µs | 28.7 µs |
| suppression puis annulation | 244 µs | 44.2 µs |
| insertion de 100 sous-titres vides au milieu | 50.5 µs | 8.83 µs |
| modification d'un texte, à travers une session | 299 ns | 986 ns |
| suppression des mentions sur 4000 sous-titres | 1.41 ms | 132 µs |
| mise en italique de 4000 sous-titres | 926 µs | 92.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.25 ms | 526 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.39 ms | 474 µs |
| recherche sans résultat sur 4000 sous-titres | 2.89 ms | 191 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.32 ms | 79.5 µs |
| casse de titre sur 4000 sous-titres | 19.2 ms | 1.04 ms |
| collage de 4000 textes | 1.06 ms | 71.3 µs |
| alignement d'une traduction de 4000 lignes, par position | 853 µs | 64.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 871 µs | 58.1 µs |
| déduction de fréquence sur 4000 sous-titres | 377 µs | 24 µs |

### 0.11.30 — 2026-09-25 — Release — charge 1.44 — allure ×1.02

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.4 µs | 1.41 µs |
| composer une réplique de deux lignes | 167 ns | 3.28 ns |
| ouvrir une vidéo | 9.33 ms | 714 µs |
| chercher une position | 575 µs | 136 µs |
| construction du modèle sur 4000 sous-titres | 8.49 µs | 2.59 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.4 µs | 3.39 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.33 µs | 1.11 µs |
| réinitialisation du modèle après une ligne retirée | 8.4 µs | 1.97 µs |
| édition d'une cellule de texte | 382 ns | 37 ns |
| édition d'une cellule de position | 10.7 µs | 415 ns |
| versionString | 43 ns | 9.85 ns |
| parse | 43.3 ns | 15.4 ns |
| format | 37.7 ns | 1.82 ns |
| position vers image | 6.64 ns | 0.0691 ns |
| image vers position | 6.64 ns | 0.073 ns |
| mise à l'échelle par un rationnel exact | 7.34 ns | 3.1 ns |
| lecture de 4000 sous-titres | 2.25 ms | 145 µs |
| écriture de 4000 sous-titres | 518 µs | 41.7 µs |
| décalage de 4000 sous-titres | 6.82 µs | 2.44 µs |
| décalage puis annulation | 13.5 µs | 3.81 µs |
| transformation de 4000 sous-titres | 71 µs | 4.36 µs |
| conversion de fréquence sur 4000 sous-titres | 68.9 µs | 8.02 µs |
| alignement sur 4000 sous-titres | 129 µs | 17.1 µs |
| tri de 4000 sous-titres à l'envers | 274 µs | 27.1 µs |
| suppression d'un sous-titre sur deux | 159 µs | 17 µs |
| suppression puis annulation | 253 µs | 63.1 µs |
| insertion de 100 sous-titres vides au milieu | 52.9 µs | 14.2 µs |
| modification d'un texte, à travers une session | 168 ns | 12.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.45 ms | 53.9 µs |
| mise en italique de 4000 sous-titres | 913 µs | 48.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.19 ms | 276 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.31 ms | 268 µs |
| recherche sans résultat sur 4000 sous-titres | 2.91 ms | 179 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.33 ms | 72.6 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 972 µs |
| collage de 4000 textes | 1.07 ms | 75.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 856 µs | 56.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 883 µs | 85.8 µs |
| déduction de fréquence sur 4000 sous-titres | 387 µs | 42.2 µs |

### 0.11.25 — 2026-09-25 — Release — charge 1.37 — allure ×1.19

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 14.1 µs | 2.34 µs |
| composer une réplique de deux lignes | 817 ns | 67.1 ns |
| ouvrir une vidéo | 32.7 ms | 25.8 ms |
| chercher une position | 617 µs | 112 µs |
| construction du modèle sur 4000 sous-titres | 13.2 µs | 1.44 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.8 µs | 1.19 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.2 µs | 2.92 µs |
| réinitialisation du modèle après une ligne retirée | 13.6 µs | 2.98 µs |
| édition d'une cellule de texte | 502 ns | 199 ns |
| édition d'une cellule de position | 11 µs | 727 ns |
| versionString | 50.6 ns | 1 ns |
| parse | 42.3 ns | 2.23 ns |
| format | 40.7 ns | 2.59 ns |
| position vers image | 7.85 ns | 1.35 ns |
| image vers position | 8.28 ns | 2.07 ns |
| mise à l'échelle par un rationnel exact | 8.3 ns | 0.221 ns |
| lecture de 4000 sous-titres | 2.64 ms | 200 µs |
| écriture de 4000 sous-titres | 949 µs | 412 µs |
| décalage de 4000 sous-titres | 8.78 µs | 3.21 µs |
| décalage puis annulation | 15.5 µs | 2.65 µs |
| transformation de 4000 sous-titres | 85.8 µs | 3.07 µs |
| conversion de fréquence sur 4000 sous-titres | 85.9 µs | 12.2 µs |
| alignement sur 4000 sous-titres | 153 µs | 11.8 µs |
| tri de 4000 sous-titres à l'envers | 329 µs | 46.1 µs |
| suppression d'un sous-titre sur deux | 183 µs | 14.7 µs |
| suppression puis annulation | 286 µs | 24.8 µs |
| insertion de 100 sous-titres vides au milieu | 70.8 µs | 21.1 µs |
| modification d'un texte, à travers une session | 219 ns | 64.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.57 ms | 53.8 µs |
| mise en italique de 4000 sous-titres | 1.09 ms | 99.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.69 ms | 235 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.82 ms | 238 µs |
| recherche sans résultat sur 4000 sous-titres | 3.16 ms | 193 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.43 ms | 76 µs |
| casse de titre sur 4000 sous-titres | 22 ms | 1.47 ms |
| collage de 4000 textes | 1.19 ms | 160 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.1 ms | 293 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 973 µs | 60 µs |
| déduction de fréquence sur 4000 sous-titres | 436 µs | 22 µs |

### 0.11.21 — 2026-09-24 — Release — charge 1.47 — allure ×0.96

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.01 µs | 1.24 µs |
| composer une réplique de deux lignes | 187 ns | 3.06 ns |
| ouvrir une vidéo | 8.94 ms | 164 µs |
| chercher une position | 557 µs | 124 µs |
| construction du modèle sur 4000 sous-titres | 7.25 µs | 1.7 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 711 ns |
| rafraîchir après un décalage de 4000 sous-titres | 7.19 µs | 1.15 µs |
| réinitialisation du modèle après une ligne retirée | 7.38 µs | 1.92 µs |
| édition d'une cellule de texte | 398 ns | 48.8 ns |
| édition d'une cellule de position | 8.16 µs | 737 ns |
| versionString | 42 ns | 0.435 ns |
| parse | 37.6 ns | 3.28 ns |
| format | 35.6 ns | 0.688 ns |
| position vers image | 6.47 ns | 0.0513 ns |
| image vers position | 6.32 ns | 0.0685 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0731 ns |
| lecture de 4000 sous-titres | 2.06 ms | 58.3 µs |
| écriture de 4000 sous-titres | 525 µs | 28.8 µs |
| décalage de 4000 sous-titres | 6.64 µs | 3.18 µs |
| décalage puis annulation | 14 µs | 9.38 µs |
| transformation de 4000 sous-titres | 70.9 µs | 5.75 µs |
| conversion de fréquence sur 4000 sous-titres | 69.3 µs | 20.4 µs |
| alignement sur 4000 sous-titres | 123 µs | 8.1 µs |
| tri de 4000 sous-titres à l'envers | 257 µs | 23.2 µs |
| suppression d'un sous-titre sur deux | 164 µs | 22.9 µs |
| suppression puis annulation | 241 µs | 35.9 µs |
| insertion de 100 sous-titres vides au milieu | 51.6 µs | 20.5 µs |
| modification d'un texte, à travers une session | 180 ns | 11.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 71.1 µs |
| mise en italique de 4000 sous-titres | 942 µs | 62.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.04 ms | 192 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.15 ms | 107 µs |
| recherche sans résultat sur 4000 sous-titres | 2.69 ms | 92.6 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 49.8 µs |
| casse de titre sur 4000 sous-titres | 18.2 ms | 684 µs |
| collage de 4000 textes | 1.01 ms | 32 µs |
| alignement d'une traduction de 4000 lignes, par position | 860 µs | 113 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 855 µs | 61.2 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 14.5 µs |

### 0.11.7 — 2026-09-21 — Release — charge 1.41 — allure ×0.98

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.17 µs | 1.85 µs |
| composer une réplique de deux lignes | 164 ns | 3.11 ns |
| ouvrir une vidéo | 9.11 ms | 591 µs |
| chercher une position | 569 µs | 158 µs |
| construction du modèle sur 4000 sous-titres | 7.38 µs | 2.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 838 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.38 µs | 2.33 µs |
| réinitialisation du modèle après une ligne retirée | 8.38 µs | 2.47 µs |
| édition d'une cellule de texte | 419 ns | 194 ns |
| édition d'une cellule de position | 9.33 µs | 667 ns |
| versionString | 41.3 ns | 12.5 ns |
| parse | 36.8 ns | 3.18 ns |
| format | 35.3 ns | 0.58 ns |
| position vers image | 6.48 ns | 0.0669 ns |
| image vers position | 6.32 ns | 0.0763 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0994 ns |
| lecture de 4000 sous-titres | 2.23 ms | 182 µs |
| écriture de 4000 sous-titres | 560 µs | 60 µs |
| décalage de 4000 sous-titres | 8.18 µs | 10.8 µs |
| décalage puis annulation | 14.9 µs | 3.4 µs |
| transformation de 4000 sous-titres | 72 µs | 9.68 µs |
| conversion de fréquence sur 4000 sous-titres | 66.2 µs | 8.29 µs |
| alignement sur 4000 sous-titres | 126 µs | 10.6 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 29.2 µs |
| suppression d'un sous-titre sur deux | 162 µs | 24.8 µs |
| suppression puis annulation | 235 µs | 32.9 µs |
| insertion de 100 sous-titres vides au milieu | 51.3 µs | 16.6 µs |
| modification d'un texte, à travers une session | 200 ns | 23.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.27 ms | 90.3 µs |
| mise en italique de 4000 sous-titres | 983 µs | 63.5 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4 ms | 260 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 288 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 165 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 72 µs |
| casse de titre sur 4000 sous-titres | 19.9 ms | 1.78 ms |
| collage de 4000 textes | 1.13 ms | 69.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 851 µs | 69.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 824 µs | 64.9 µs |
| déduction de fréquence sur 4000 sous-titres | 370 µs | 19 µs |

### 0.11.1 — 2026-09-19 — Release — charge 1.39 — allure ×1.11

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.83 µs | 993 ns |
| composer une réplique de deux lignes | 206 ns | 18.6 ns |
| ouvrir une vidéo | 9.31 ms | 642 µs |
| chercher une position | 584 µs | 182 µs |
| construction du modèle sur 4000 sous-titres | 9.69 µs | 2.15 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 460 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.38 µs | 2.2 µs |
| réinitialisation du modèle après une ligne retirée | 8.91 µs | 2.52 µs |
| édition d'une cellule de texte | 460 ns | 73.6 ns |
| édition d'une cellule de position | 9.51 µs | 1.27 µs |
| versionString | 42.3 ns | 0.852 ns |
| parse | 47.7 ns | 9.13 ns |
| format | 41.9 ns | 2.6 ns |
| position vers image | 7.62 ns | 0.0625 ns |
| image vers position | 8.06 ns | 0.438 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.0843 ns |
| lecture de 4000 sous-titres | 2.58 ms | 152 µs |
| écriture de 4000 sous-titres | 626 µs | 93.3 µs |
| décalage de 4000 sous-titres | 8.32 µs | 5.12 µs |
| décalage puis annulation | 14.9 µs | 3.26 µs |
| transformation de 4000 sous-titres | 85.5 µs | 8.04 µs |
| conversion de fréquence sur 4000 sous-titres | 84.6 µs | 18.3 µs |
| alignement sur 4000 sous-titres | 142 µs | 9.16 µs |
| tri de 4000 sous-titres à l'envers | 304 µs | 35 µs |
| suppression d'un sous-titre sur deux | 188 µs | 23.2 µs |
| suppression puis annulation | 287 µs | 73.1 µs |
| insertion de 100 sous-titres vides au milieu | 58.6 µs | 19.2 µs |
| modification d'un texte, à travers une session | 209 ns | 19.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.41 ms | 35 µs |
| mise en italique de 4000 sous-titres | 1.1 ms | 155 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.65 ms | 307 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.74 ms | 206 µs |
| recherche sans résultat sur 4000 sous-titres | 3.11 ms | 262 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.38 ms | 102 µs |
| casse de titre sur 4000 sous-titres | 21.2 ms | 1.87 ms |
| collage de 4000 textes | 1.26 ms | 189 µs |
| déduction de fréquence sur 4000 sous-titres | 447 µs | 14.4 µs |

### 0.10.18 — 2026-09-19 — Release — charge 0.85 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.74 µs | 1.28 µs |
| composer une réplique de deux lignes | 188 ns | 56 ns |
| ouvrir une vidéo | 9.65 ms | 1.41 ms |
| chercher une position | 568 µs | 173 µs |
| construction du modèle sur 4000 sous-titres | 8.8 µs | 2.69 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18 µs | 6.1 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.48 µs | 7.42 µs |
| réinitialisation du modèle après une ligne retirée | 8.38 µs | 1.42 µs |
| édition d'une cellule de texte | 404 ns | 120 ns |
| édition d'une cellule de position | 10.7 µs | 502 ns |
| versionString | 42 ns | 0.467 ns |
| parse | 37 ns | 4.98 ns |
| format | 37.1 ns | 5.35 ns |
| position vers image | 7.96 ns | 2.09 ns |
| image vers position | 6.64 ns | 0.0771 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.101 ns |
| lecture de 4000 sous-titres | 2.49 ms | 283 µs |
| écriture de 4000 sous-titres | 594 µs | 80.5 µs |
| décalage de 4000 sous-titres | 7.03 µs | 3.43 µs |
| décalage puis annulation | 13.8 µs | 4.13 µs |
| transformation de 4000 sous-titres | 77.3 µs | 10.8 µs |
| conversion de fréquence sur 4000 sous-titres | 70.6 µs | 17.3 µs |
| alignement sur 4000 sous-titres | 128 µs | 20.5 µs |
| tri de 4000 sous-titres à l'envers | 261 µs | 18.5 µs |
| suppression d'un sous-titre sur deux | 156 µs | 21.9 µs |
| suppression puis annulation | 236 µs | 33.8 µs |
| insertion de 100 sous-titres vides au milieu | 50.7 µs | 12.4 µs |
| modification d'un texte, à travers une session | 191 ns | 20.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.24 ms | 94.2 µs |
| mise en italique de 4000 sous-titres | 952 µs | 80.3 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.02 ms | 306 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 247 µs |
| recherche sans résultat sur 4000 sous-titres | 2.66 ms | 187 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 93 µs |
| casse de titre sur 4000 sous-titres | 19 ms | 1.52 ms |
| collage de 4000 textes | 1.04 ms | 79 µs |
| déduction de fréquence sur 4000 sous-titres | 427 µs | 17.6 µs |

### 0.10.17 — 2026-09-19 — Release — charge 0.91 — allure ×0.86

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.01 µs | 3.47 µs |
| composer une réplique de deux lignes | 176 ns | 2.87 ns |
| ouvrir une vidéo | 8.98 ms | 543 µs |
| chercher une position | 596 µs | 180 µs |
| construction du modèle sur 4000 sous-titres | 8.58 µs | 4.41 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15.6 µs | 2.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.52 µs | 3.96 µs |
| réinitialisation du modèle après une ligne retirée | 9.86 µs | 4.43 µs |
| édition d'une cellule de texte | 404 ns | 162 ns |
| édition d'une cellule de position | 9.54 µs | 1.87 µs |
| versionString | 41.8 ns | 0.484 ns |
| parse | 36.2 ns | 2.89 ns |
| format | 36.7 ns | 0.672 ns |
| position vers image | 6.75 ns | 0.252 ns |
| image vers position | 6.64 ns | 0.0761 ns |
| mise à l'échelle par un rationnel exact | 7.86 ns | 0.06 ns |
| lecture de 4000 sous-titres | 2.2 ms | 153 µs |
| écriture de 4000 sous-titres | 558 µs | 67.3 µs |
| décalage de 4000 sous-titres | 7.17 µs | 3.94 µs |
| décalage puis annulation | 13.3 µs | 2.72 µs |
| transformation de 4000 sous-titres | 78.7 µs | 7.82 µs |
| conversion de fréquence sur 4000 sous-titres | 68.3 µs | 10.2 µs |
| alignement sur 4000 sous-titres | 124 µs | 9.24 µs |
| tri de 4000 sous-titres à l'envers | 261 µs | 23.4 µs |
| suppression d'un sous-titre sur deux | 184 µs | 13.8 µs |
| suppression puis annulation | 232 µs | 26 µs |
| insertion de 100 sous-titres vides au milieu | 52.7 µs | 16.2 µs |
| modification d'un texte, à travers une session | 178 ns | 36.4 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 71.5 µs |
| mise en italique de 4000 sous-titres | 906 µs | 60.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.85 ms | 122 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.12 ms | 221 µs |
| recherche sans résultat sur 4000 sous-titres | 2.68 ms | 166 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.2 ms | 63.7 µs |
| casse de titre sur 4000 sous-titres | 17.9 ms | 585 µs |
| collage de 4000 textes | 964 µs | 32.9 µs |
| déduction de fréquence sur 4000 sous-titres | 367 µs | 9.61 µs |

### 0.10.15 — 2026-09-16 — Release — charge 0.75 — allure ×1.13

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.63 µs | 2.19 µs |
| composer une réplique de deux lignes | 194 ns | 3.52 ns |
| ouvrir une vidéo | 10.4 ms | 2.34 ms |
| chercher une position | 584 µs | 130 µs |
| construction du modèle sur 4000 sous-titres | 8.43 µs | 2.82 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.5 µs | 4.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.7 µs | 5.81 µs |
| réinitialisation du modèle après une ligne retirée | 10.3 µs | 8.17 µs |
| édition d'une cellule de texte | 477 ns | 92.7 ns |
| édition d'une cellule de position | 14.5 µs | 1.92 µs |
| versionString | 50.2 ns | 1.2 ns |
| parse | 52.6 ns | 24.8 ns |
| format | 44.5 ns | 7.24 ns |
| position vers image | 7.95 ns | 1.82 ns |
| image vers position | 7.42 ns | 0.142 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.1 ns |
| lecture de 4000 sous-titres | 2.6 ms | 153 µs |
| écriture de 4000 sous-titres | 627 µs | 85.3 µs |
| décalage de 4000 sous-titres | 9.65 µs | 5.76 µs |
| décalage puis annulation | 21.2 µs | 23.1 µs |
| transformation de 4000 sous-titres | 85.1 µs | 23 µs |
| conversion de fréquence sur 4000 sous-titres | 77.6 µs | 9.25 µs |
| alignement sur 4000 sous-titres | 150 µs | 20.8 µs |
| tri de 4000 sous-titres à l'envers | 320 µs | 21.1 µs |
| suppression d'un sous-titre sur deux | 196 µs | 37.4 µs |
| suppression puis annulation | 314 µs | 44.6 µs |
| insertion de 100 sous-titres vides au milieu | 71.6 µs | 32.4 µs |
| modification d'un texte, à travers une session | 236 ns | 65.2 ns |
| suppression des mentions sur 4000 sous-titres | 2.3 ms | 288 µs |
| mise en italique de 4000 sous-titres | 1.87 ms | 555 µs |
| déduction de fréquence sur 4000 sous-titres | 432 µs | 25 µs |

### 0.10.14 — 2026-09-15 — Release — charge 1.12 — allure ×1.05

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.19 µs | 2 µs |
| composer une réplique de deux lignes | 220 ns | 66.3 ns |
| ouvrir une vidéo | 11.8 ms | 1.73 ms |
| chercher une position | 596 µs | 153 µs |
| construction du modèle sur 4000 sous-titres | 10.9 µs | 11.7 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.7 µs | 878 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.29 µs | 1.33 µs |
| réinitialisation du modèle après une ligne retirée | 7.28 µs | 2.46 µs |
| édition d'une cellule de texte | 424 ns | 280 ns |
| édition d'une cellule de position | 9.45 µs | 1.28 µs |
| versionString | 43.6 ns | 12.3 ns |
| parse | 41.7 ns | 11.3 ns |
| format | 40 ns | 8.22 ns |
| position vers image | 8.02 ns | 0.118 ns |
| image vers position | 12.7 ns | 0.57 ns |
| mise à l'échelle par un rationnel exact | 8.3 ns | 0.232 ns |
| lecture de 4000 sous-titres | 2.62 ms | 227 µs |
| écriture de 4000 sous-titres | 573 µs | 25.3 µs |
| décalage de 4000 sous-titres | 8.13 µs | 3.15 µs |
| décalage puis annulation | 16.2 µs | 6.83 µs |
| transformation de 4000 sous-titres | 79.9 µs | 11.7 µs |
| conversion de fréquence sur 4000 sous-titres | 80.1 µs | 15.1 µs |
| alignement sur 4000 sous-titres | 151 µs | 19.6 µs |
| tri de 4000 sous-titres à l'envers | 304 µs | 22.6 µs |
| suppression d'un sous-titre sur deux | 157 µs | 23 µs |
| suppression puis annulation | 233 µs | 17.3 µs |
| insertion de 100 sous-titres vides au milieu | 54.4 µs | 15.6 µs |
| modification d'un texte, à travers une session | 182 ns | 28.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.02 ms | 34.7 µs |
| mise en italique de 4000 sous-titres | 1.24 ms | 68.7 µs |
| déduction de fréquence sur 4000 sous-titres | 371 µs | 13.5 µs |

### 0.10.10 — 2026-09-13 — Release — charge 1.48 — allure ×1.10

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.61 µs | 2.33 µs |
| composer une réplique de deux lignes | 224 ns | 172 ns |
| ouvrir une vidéo | 9.44 ms | 854 µs |
| chercher une position | 618 µs | 160 µs |
| construction du modèle sur 4000 sous-titres | 10.3 µs | 1.99 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.7 µs | 568 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.65 µs | 4.44 µs |
| réinitialisation du modèle après une ligne retirée | 9.86 µs | 3.44 µs |
| édition d'une cellule de texte | 512 ns | 195 ns |
| édition d'une cellule de position | 10.8 µs | 509 ns |
| versionString | 48.7 ns | 0.906 ns |
| parse | 45.9 ns | 20.2 ns |
| format | 44.1 ns | 4.62 ns |
| position vers image | 8.02 ns | 0.122 ns |
| image vers position | 7.81 ns | 0.822 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.0889 ns |
| lecture de 4000 sous-titres | 2.58 ms | 283 µs |
| écriture de 4000 sous-titres | 548 µs | 96.2 µs |
| décalage de 4000 sous-titres | 7.91 µs | 3.37 µs |
| décalage puis annulation | 15.3 µs | 3.93 µs |
| transformation de 4000 sous-titres | 83.5 µs | 8.28 µs |
| conversion de fréquence sur 4000 sous-titres | 77.1 µs | 6.64 µs |
| alignement sur 4000 sous-titres | 144 µs | 12.4 µs |
| tri de 4000 sous-titres à l'envers | 299 µs | 18.2 µs |
| suppression d'un sous-titre sur deux | 175 µs | 12.6 µs |
| suppression puis annulation | 271 µs | 23.7 µs |
| insertion de 100 sous-titres vides au milieu | 61 µs | 17.5 µs |
| modification d'un texte, à travers une session | 198 ns | 11.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.04 ms | 40.9 µs |
| mise en italique de 4000 sous-titres | 1.39 ms | 44.8 µs |
| déduction de fréquence sur 4000 sous-titres | 434 µs | 40.1 µs |

### 0.10.7 — 2026-09-12 — Release — charge 0.75 — allure ×0.83

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.52 µs | 2.26 µs |
| composer une réplique de deux lignes | 176 ns | 6.62 ns |
| ouvrir une vidéo | 8.99 ms | 230 µs |
| chercher une position | 557 µs | 146 µs |
| construction du modèle sur 4000 sous-titres | 7.33 µs | 2.35 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15.3 µs | 252 ns |
| rafraîchir après un décalage de 4000 sous-titres | 7.18 µs | 737 ns |
| réinitialisation du modèle après une ligne retirée | 7.38 µs | 1.7 µs |
| édition d'une cellule de texte | 420 ns | 60.4 ns |
| édition d'une cellule de position | 8.36 µs | 2.56 µs |
| versionString | 36.7 ns | 1.31 ns |
| parse | 36.6 ns | 3.77 ns |
| format | 36.9 ns | 0.69 ns |
| position vers image | 6.5 ns | 0.198 ns |
| image vers position | 6.33 ns | 0.138 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0768 ns |
| lecture de 4000 sous-titres | 2.11 ms | 93 µs |
| écriture de 4000 sous-titres | 522 µs | 19 µs |
| décalage de 4000 sous-titres | 6.61 µs | 2.89 µs |
| décalage puis annulation | 13 µs | 3.48 µs |
| transformation de 4000 sous-titres | 71.7 µs | 7.61 µs |
| conversion de fréquence sur 4000 sous-titres | 68.4 µs | 12.8 µs |
| alignement sur 4000 sous-titres | 123 µs | 5.54 µs |
| tri de 4000 sous-titres à l'envers | 264 µs | 26.3 µs |
| suppression d'un sous-titre sur deux | 153 µs | 17 µs |
| suppression puis annulation | 232 µs | 32.9 µs |
| insertion de 100 sous-titres vides au milieu | 53.1 µs | 20.2 µs |
| modification d'un texte, à travers une session | 171 ns | 10.4 ns |
| suppression des mentions sur 4000 sous-titres | 948 µs | 41.6 µs |
| mise en italique de 4000 sous-titres | 1.15 ms | 26.1 µs |
| déduction de fréquence sur 4000 sous-titres | 367 µs | 8.67 µs |

### 0.10.6 — 2026-09-12 — Release — charge 0.60 — allure ×0.93

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.24 µs | 1.65 µs |
| composer une réplique de deux lignes | 182 ns | 5.38 ns |
| ouvrir une vidéo | 9.39 ms | 784 µs |
| chercher une position | 587 µs | 192 µs |
| construction du modèle sur 4000 sous-titres | 8.77 µs | 5.77 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.1 µs | 545 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.52 µs | 3.53 µs |
| réinitialisation du modèle après une ligne retirée | 8.52 µs | 3.7 µs |
| édition d'une cellule de texte | 479 ns | 128 ns |
| édition d'une cellule de position | 11.1 µs | 2.53 µs |
| versionString | 41.8 ns | 0.92 ns |
| parse | 38.5 ns | 10.3 ns |
| format | 37 ns | 1.66 ns |
| position vers image | 7.62 ns | 0.113 ns |
| image vers position | 6.64 ns | 0.0593 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0756 ns |
| lecture de 4000 sous-titres | 2.41 ms | 331 µs |
| écriture de 4000 sous-titres | 622 µs | 132 µs |
| décalage de 4000 sous-titres | 7.07 µs | 3.57 µs |
| décalage puis annulation | 14.2 µs | 6.79 µs |
| transformation de 4000 sous-titres | 72.6 µs | 13.2 µs |
| conversion de fréquence sur 4000 sous-titres | 69.9 µs | 10.8 µs |
| alignement sur 4000 sous-titres | 143 µs | 13.7 µs |
| tri de 4000 sous-titres à l'envers | 312 µs | 33.4 µs |
| suppression d'un sous-titre sur deux | 178 µs | 15.6 µs |
| suppression puis annulation | 234 µs | 26 µs |
| insertion de 100 sous-titres vides au milieu | 52.1 µs | 20.2 µs |
| modification d'un texte, à travers une session | 191 ns | 128 ns |
| suppression des mentions sur 4000 sous-titres | 1.04 ms | 38.9 µs |
| mise en italique de 4000 sous-titres | 1.13 ms | 178 µs |
| déduction de fréquence sur 4000 sous-titres | 412 µs | 41.1 µs |

### 0.10.2 — 2026-09-12 — Release — charge 1.46 — allure ×1.05

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.7 µs | 1.13 µs |
| composer une réplique de deux lignes | 208 ns | 16.1 ns |
| ouvrir une vidéo | 10.4 ms | 1.57 ms |
| chercher une position | 589 µs | 139 µs |
| construction du modèle sur 4000 sous-titres | 8.44 µs | 1.9 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.4 µs | 1.45 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.94 µs | 3.8 µs |
| réinitialisation du modèle après une ligne retirée | 8.84 µs | 2.71 µs |
| édition d'une cellule de texte | 442 ns | 56.7 ns |
| édition d'une cellule de position | 9.81 µs | 2.53 µs |
| versionString | 43.6 ns | 0.829 ns |
| parse | 42.5 ns | 6.17 ns |
| format | 36.3 ns | 4.69 ns |
| position vers image | 7.63 ns | 0.198 ns |
| image vers position | 8.77 ns | 1.76 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.0968 ns |
| lecture de 4000 sous-titres | 3.32 ms | 1.14 ms |
| écriture de 4000 sous-titres | 714 µs | 194 µs |
| décalage de 4000 sous-titres | 7.69 µs | 3.48 µs |
| décalage puis annulation | 15.2 µs | 6.13 µs |
| transformation de 4000 sous-titres | 102 µs | 63.1 µs |
| conversion de fréquence sur 4000 sous-titres | 86.6 µs | 27.8 µs |
| alignement sur 4000 sous-titres | 149 µs | 10.2 µs |
| tri de 4000 sous-titres à l'envers | 334 µs | 67.8 µs |
| suppression d'un sous-titre sur deux | 206 µs | 39 µs |
| suppression puis annulation | 342 µs | 168 µs |
| insertion de 100 sous-titres vides au milieu | 83.5 µs | 39.4 µs |
| modification d'un texte, à travers une session | 223 ns | 28.4 ns |
| suppression des mentions sur 4000 sous-titres | 1.12 ms | 86.7 µs |
| mise en italique de 4000 sous-titres | 1.32 ms | 194 µs |
| déduction de fréquence sur 4000 sous-titres | 488 µs | 74.9 µs |

### 0.10.1 — 2026-09-11 — Release — charge 0.34 — allure ×1.10

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.26 µs | 1.94 µs |
| composer une réplique de deux lignes | 216 ns | 4.42 ns |
| ouvrir une vidéo | 9.76 ms | 846 µs |
| chercher une position | 566 µs | 172 µs |
| construction du modèle sur 4000 sous-titres | 9.01 µs | 3.34 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.7 µs | 2.55 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.84 µs | 2.49 µs |
| réinitialisation du modèle après une ligne retirée | 8.96 µs | 2.82 µs |
| édition d'une cellule de texte | 491 ns | 111 ns |
| édition d'une cellule de position | 10.3 µs | 1.38 µs |
| versionString | 44.4 ns | 3.03 ns |
| parse | 44.4 ns | 3.23 ns |
| format | 38.8 ns | 2 ns |
| position vers image | 8.03 ns | 0.237 ns |
| image vers position | 8.05 ns | 0.385 ns |
| mise à l'échelle par un rationnel exact | 8.34 ns | 0.449 ns |
| lecture de 4000 sous-titres | 2.65 ms | 168 µs |
| écriture de 4000 sous-titres | 616 µs | 55.3 µs |
| décalage de 4000 sous-titres | 9.29 µs | 1.65 µs |
| décalage puis annulation | 17.6 µs | 1.85 µs |
| transformation de 4000 sous-titres | 92.4 µs | 10.1 µs |
| conversion de fréquence sur 4000 sous-titres | 87 µs | 13.8 µs |
| alignement sur 4000 sous-titres | 156 µs | 20.7 µs |
| tri de 4000 sous-titres à l'envers | 367 µs | 124 µs |
| suppression d'un sous-titre sur deux | 196 µs | 33.2 µs |
| suppression puis annulation | 292 µs | 33.7 µs |
| insertion de 100 sous-titres vides au milieu | 66.3 µs | 11.5 µs |
| modification d'un texte, à travers une session | 217 ns | 23.4 ns |
| suppression des mentions sur 4000 sous-titres | 1.11 ms | 80.1 µs |
| mise en italique de 4000 sous-titres | 1.3 ms | 52.1 µs |
| déduction de fréquence sur 4000 sous-titres | 448 µs | 17.3 µs |

### 0.10.0 — 2026-09-11 — Release — charge 1.45 — allure ×1.01

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.69 µs | 3.49 µs |
| composer une réplique de deux lignes | 184 ns | 62.8 ns |
| ouvrir une vidéo | 11.3 ms | 2.05 ms |
| chercher une position | 575 µs | 162 µs |
| construction du modèle sur 4000 sous-titres | 8.92 µs | 2.86 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.7 µs | 516 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.46 µs | 3.27 µs |
| réinitialisation du modèle après une ligne retirée | 9.78 µs | 3.58 µs |
| édition d'une cellule de texte | 414 ns | 191 ns |
| édition d'une cellule de position | 10.7 µs | 713 ns |
| versionString | 43 ns | 0.552 ns |
| parse | 39 ns | 2.1 ns |
| format | 37.3 ns | 7.36 ns |
| position vers image | 7.62 ns | 0.108 ns |
| image vers position | 7.62 ns | 0.0773 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.087 ns |
| lecture de 4000 sous-titres | 2.39 ms | 99.5 µs |
| écriture de 4000 sous-titres | 593 µs | 43.8 µs |
| décalage de 4000 sous-titres | 6.95 µs | 2.86 µs |
| décalage puis annulation | 15.1 µs | 2.92 µs |
| transformation de 4000 sous-titres | 81.5 µs | 19.2 µs |
| conversion de fréquence sur 4000 sous-titres | 78.2 µs | 11.4 µs |
| alignement sur 4000 sous-titres | 143 µs | 9.97 µs |
| tri de 4000 sous-titres à l'envers | 298 µs | 14.6 µs |
| suppression d'un sous-titre sur deux | 155 µs | 25.7 µs |
| suppression puis annulation | 283 µs | 56.4 µs |
| insertion de 100 sous-titres vides au milieu | 52.6 µs | 20.3 µs |
| modification d'un texte, à travers une session | 192 ns | 110 ns |
| suppression des mentions sur 4000 sous-titres | 1.05 ms | 38.7 µs |
| déduction de fréquence sur 4000 sous-titres | 422 µs | 13.6 µs |

### 0.9.25 — 2026-09-11 — Release — charge 0.87 — allure ×0.97

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.31 µs | 902 ns |
| composer une réplique de deux lignes | 196 ns | 2.76 ns |
| ouvrir une vidéo | 9.5 ms | 846 µs |
| chercher une position | 579 µs | 164 µs |
| construction du modèle sur 4000 sous-titres | 8.67 µs | 4.68 µs |
| une fenêtre de 40 lignes, cinq colonnes | 16.5 µs | 4.27 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.66 µs | 3.31 µs |
| réinitialisation du modèle après une ligne retirée | 7.45 µs | 4.41 µs |
| édition d'une cellule de texte | 432 ns | 59.4 ns |
| édition d'une cellule de position | 9.36 µs | 693 ns |
| versionString | 42.6 ns | 0.465 ns |
| parse | 36.3 ns | 6.56 ns |
| format | 35.6 ns | 0.55 ns |
| position vers image | 8.31 ns | 2.71 ns |
| image vers position | 6.9 ns | 1.31 ns |
| mise à l'échelle par un rationnel exact | 6.87 ns | 0.08 ns |
| lecture de 4000 sous-titres | 2.38 ms | 173 µs |
| écriture de 4000 sous-titres | 614 µs | 40 µs |
| décalage de 4000 sous-titres | 8.45 µs | 8.26 µs |
| décalage puis annulation | 15.6 µs | 5.19 µs |
| transformation de 4000 sous-titres | 82.5 µs | 9.79 µs |
| conversion de fréquence sur 4000 sous-titres | 82.6 µs | 18.8 µs |
| alignement sur 4000 sous-titres | 129 µs | 23.5 µs |
| tri de 4000 sous-titres à l'envers | 274 µs | 54.9 µs |
| suppression d'un sous-titre sur deux | 161 µs | 17.7 µs |
| suppression puis annulation | 281 µs | 61.4 µs |
| insertion de 100 sous-titres vides au milieu | 61.8 µs | 21.9 µs |
| modification d'un texte, à travers une session | 177 ns | 39.6 ns |
| suppression des mentions sur 4000 sous-titres | 983 µs | 102 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 13.1 µs |

### 0.9.24 — 2026-09-10 — Release — charge 0.89 — allure ×0.97

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.74 µs | 1.98 µs |
| composer une réplique de deux lignes | 178 ns | 3.5 ns |
| ouvrir une vidéo | 9.95 ms | 1.81 ms |
| chercher une position | 581 µs | 125 µs |
| construction du modèle sur 4000 sous-titres | 8.61 µs | 4.94 µs |
| une fenêtre de 40 lignes, cinq colonnes | 16.2 µs | 2.75 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.1 µs | 2.28 µs |
| réinitialisation du modèle après une ligne retirée | 9.43 µs | 3.48 µs |
| édition d'une cellule de texte | 424 ns | 348 ns |
| édition d'une cellule de position | 10.7 µs | 2.45 µs |
| versionString | 41.8 ns | 0.684 ns |
| parse | 40.3 ns | 5.05 ns |
| format | 40.9 ns | 0.802 ns |
| position vers image | 7.25 ns | 0.0913 ns |
| image vers position | 7.43 ns | 0.215 ns |
| mise à l'échelle par un rationnel exact | 7.1 ns | 1.83 ns |
| lecture de 4000 sous-titres | 2.46 ms | 247 µs |
| écriture de 4000 sous-titres | 577 µs | 35 µs |
| décalage de 4000 sous-titres | 9.23 µs | 5.74 µs |
| décalage puis annulation | 15.5 µs | 3.01 µs |
| transformation de 4000 sous-titres | 77.7 µs | 16.6 µs |
| conversion de fréquence sur 4000 sous-titres | 76.7 µs | 10.2 µs |
| alignement sur 4000 sous-titres | 143 µs | 9.83 µs |
| tri de 4000 sous-titres à l'envers | 299 µs | 19.3 µs |
| suppression d'un sous-titre sur deux | 179 µs | 26.5 µs |
| suppression puis annulation | 276 µs | 35.9 µs |
| insertion de 100 sous-titres vides au milieu | 60.3 µs | 22.6 µs |
| modification d'un texte, à travers une session | 177 ns | 26.8 ns |
| suppression des mentions sur 4000 sous-titres | 982 µs | 26 µs |
| déduction de fréquence sur 4000 sous-titres | 416 µs | 23.4 µs |

### 0.9.23 — 2026-09-10 — Release — charge 0.71 — allure ×0.97

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.5 µs | 1.26 µs |
| composer une réplique de deux lignes | 177 ns | 3.37 ns |
| ouvrir une vidéo | 9.8 ms | 1.63 ms |
| chercher une position | 589 µs | 149 µs |
| construction du modèle sur 4000 sous-titres | 10.1 µs | 6.5 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 426 ns |
| rafraîchir après un décalage de 4000 sous-titres | 9.66 µs | 3.16 µs |
| réinitialisation du modèle après une ligne retirée | 9.57 µs | 2.24 µs |
| édition d'une cellule de texte | 444 ns | 57.9 ns |
| édition d'une cellule de position | 11.2 µs | 5.16 µs |
| versionString | 41.9 ns | 0.583 ns |
| parse | 37.7 ns | 13.1 ns |
| format | 39.2 ns | 1.97 ns |
| position vers image | 7.27 ns | 0.165 ns |
| image vers position | 7.42 ns | 0.0998 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0486 ns |
| lecture de 4000 sous-titres | 2.34 ms | 468 µs |
| écriture de 4000 sous-titres | 620 µs | 134 µs |
| décalage de 4000 sous-titres | 8.16 µs | 3.63 µs |
| décalage puis annulation | 15.3 µs | 4.08 µs |
| transformation de 4000 sous-titres | 72.1 µs | 4.35 µs |
| conversion de fréquence sur 4000 sous-titres | 76.9 µs | 4.9 µs |
| alignement sur 4000 sous-titres | 147 µs | 21.4 µs |
| tri de 4000 sous-titres à l'envers | 306 µs | 35.7 µs |
| suppression d'un sous-titre sur deux | 183 µs | 18.4 µs |
| suppression puis annulation | 297 µs | 150 µs |
| insertion de 100 sous-titres vides au milieu | 58.7 µs | 7.81 µs |
| modification d'un texte, à travers une session | 193 ns | 45.1 ns |
| suppression des mentions sur 4000 sous-titres | 988 µs | 107 µs |
| déduction de fréquence sur 4000 sous-titres | 373 µs | 21.2 µs |

### 0.9.22 — 2026-09-10 — Release — charge 0.92 — allure ×1.12

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 9.51 µs | 1.87 µs |
| composer une réplique de deux lignes | 226 ns | 72.4 ns |
| ouvrir une vidéo | 9.48 ms | 647 µs |
| chercher une position | 588 µs | 154 µs |
| construction du modèle sur 4000 sous-titres | 11.4 µs | 3.51 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.9 µs | 1.88 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.73 µs | 1.9 µs |
| réinitialisation du modèle après une ligne retirée | 10.5 µs | 3.44 µs |
| édition d'une cellule de texte | 496 ns | 116 ns |
| édition d'une cellule de position | 11.9 µs | 2.13 µs |
| versionString | 53.2 ns | 20 ns |
| parse | 42.4 ns | 5 ns |
| format | 39.4 ns | 2.01 ns |
| position vers image | 7.75 ns | 0.581 ns |
| image vers position | 8.12 ns | 3.01 ns |
| mise à l'échelle par un rationnel exact | 8.1 ns | 0.132 ns |
| lecture de 4000 sous-titres | 2.6 ms | 344 µs |
| écriture de 4000 sous-titres | 620 µs | 72.8 µs |
| décalage de 4000 sous-titres | 8.3 µs | 3.36 µs |
| décalage puis annulation | 16.8 µs | 5.13 µs |
| transformation de 4000 sous-titres | 87 µs | 17.4 µs |
| conversion de fréquence sur 4000 sous-titres | 81.5 µs | 14.5 µs |
| alignement sur 4000 sous-titres | 153 µs | 8.79 µs |
| tri de 4000 sous-titres à l'envers | 325 µs | 30.9 µs |
| suppression d'un sous-titre sur deux | 192 µs | 26.1 µs |
| suppression puis annulation | 291 µs | 34.3 µs |
| insertion de 100 sous-titres vides au milieu | 62.8 µs | 10.2 µs |
| modification d'un texte, à travers une session | 225 ns | 67.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.05 ms | 50.6 µs |
| déduction de fréquence sur 4000 sous-titres | 452 µs | 32.1 µs |

### 0.9.21 — 2026-09-09 — Release — charge 1.21 — allure ×1.02

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.69 µs | 1.6 µs |
| composer une réplique de deux lignes | 194 ns | 4.07 ns |
| ouvrir une vidéo | 9.42 ms | 963 µs |
| chercher une position | 548 µs | 145 µs |
| construction du modèle sur 4000 sous-titres | 13.4 µs | 3.46 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.9 µs | 5.9 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.59 µs | 2.45 µs |
| réinitialisation du modèle après une ligne retirée | 8.45 µs | 2.41 µs |
| édition d'une cellule de texte | 464 ns | 56.6 ns |
| édition d'une cellule de position | 11.1 µs | 13.4 µs |
| versionString | 41.9 ns | 0.541 ns |
| parse | 39.1 ns | 4.83 ns |
| format | 41.8 ns | 0.621 ns |
| position vers image | 7.62 ns | 0.0926 ns |
| image vers position | 8.01 ns | 0.089 ns |
| mise à l'échelle par un rationnel exact | 8.17 ns | 1.12 ns |
| lecture de 4000 sous-titres | 2.47 ms | 171 µs |
| écriture de 4000 sous-titres | 604 µs | 31.6 µs |
| décalage de 4000 sous-titres | 7.58 µs | 3.15 µs |
| décalage puis annulation | 15.1 µs | 3.97 µs |
| transformation de 4000 sous-titres | 78.6 µs | 8.02 µs |
| conversion de fréquence sur 4000 sous-titres | 80.3 µs | 14.1 µs |
| alignement sur 4000 sous-titres | 144 µs | 15.7 µs |
| tri de 4000 sous-titres à l'envers | 307 µs | 33.4 µs |
| suppression d'un sous-titre sur deux | 196 µs | 40.6 µs |
| suppression puis annulation | 273 µs | 25.1 µs |
| insertion de 100 sous-titres vides au milieu | 60.7 µs | 18.2 µs |
| modification d'un texte, à travers une session | 196 ns | 15.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.07 ms | 106 µs |
| déduction de fréquence sur 4000 sous-titres | 432 µs | 32.2 µs |

### 0.9.20 — 2026-09-08 — Release — charge 1.44 — allure ×0.80

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.75 µs | 1.68 µs |
| composer une réplique de deux lignes | 193 ns | 65.4 ns |
| ouvrir une vidéo | 9.19 ms | 552 µs |
| chercher une position | 587 µs | 190 µs |
| construction du modèle sur 4000 sous-titres | 7.66 µs | 2.34 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 2.69 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.41 µs | 3.29 µs |
| réinitialisation du modèle après une ligne retirée | 7.23 µs | 1.74 µs |
| édition d'une cellule de texte | 457 ns | 94.3 ns |
| édition d'une cellule de position | 8.14 µs | 1.39 µs |
| versionString | 36.7 ns | 2.8 ns |
| parse | 40.2 ns | 2.75 ns |
| format | 35.9 ns | 0.468 ns |
| position vers image | 6.32 ns | 0.0727 ns |
| image vers position | 8.56 ns | 3.16 ns |
| mise à l'échelle par un rationnel exact | 6.87 ns | 1.46 ns |
| lecture de 4000 sous-titres | 2.15 ms | 76 µs |
| écriture de 4000 sous-titres | 519 µs | 49.9 µs |
| décalage de 4000 sous-titres | 8.21 µs | 5.46 µs |
| décalage puis annulation | 14.7 µs | 3.1 µs |
| transformation de 4000 sous-titres | 71.2 µs | 9.17 µs |
| conversion de fréquence sur 4000 sous-titres | 72.8 µs | 17.4 µs |
| alignement sur 4000 sous-titres | 125 µs | 14.6 µs |
| tri de 4000 sous-titres à l'envers | 259 µs | 21.2 µs |
| suppression d'un sous-titre sur deux | 156 µs | 25 µs |
| suppression puis annulation | 236 µs | 32.8 µs |
| insertion de 100 sous-titres vides au milieu | 51.1 µs | 20 µs |
| modification d'un texte, à travers une session | 192 ns | 25.8 ns |
| suppression des mentions sur 4000 sous-titres | 934 µs | 83.8 µs |
| déduction de fréquence sur 4000 sous-titres | 371 µs | 22.9 µs |

### 0.9.19 — 2026-09-08 — Release — charge 1.20 — allure ×0.81

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.87 µs | 2.11 µs |
| composer une réplique de deux lignes | 165 ns | 2.65 ns |
| ouvrir une vidéo | 9.34 ms | 885 µs |
| chercher une position | 562 µs | 149 µs |
| construction du modèle sur 4000 sous-titres | 7.49 µs | 4.42 µs |
| une fenêtre de 40 lignes, cinq colonnes | 16.5 µs | 4.72 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.3 µs | 2.72 µs |
| réinitialisation du modèle après une ligne retirée | 8.5 µs | 4.18 µs |
| édition d'une cellule de texte | 432 ns | 251 ns |
| édition d'une cellule de position | 8.21 µs | 1.27 µs |
| versionString | 37.2 ns | 6.36 ns |
| parse | 40.5 ns | 2.72 ns |
| format | 35.9 ns | 0.438 ns |
| position vers image | 6.67 ns | 1.22 ns |
| image vers position | 6.4 ns | 0.311 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0481 ns |
| lecture de 4000 sous-titres | 2.2 ms | 166 µs |
| écriture de 4000 sous-titres | 589 µs | 27.5 µs |
| décalage de 4000 sous-titres | 7.39 µs | 7.4 µs |
| décalage puis annulation | 14.9 µs | 2.91 µs |
| transformation de 4000 sous-titres | 77.1 µs | 4.74 µs |
| conversion de fréquence sur 4000 sous-titres | 68.8 µs | 13 µs |
| alignement sur 4000 sous-titres | 125 µs | 11.7 µs |
| tri de 4000 sous-titres à l'envers | 311 µs | 128 µs |
| suppression d'un sous-titre sur deux | 192 µs | 41.7 µs |
| suppression puis annulation | 260 µs | 42.9 µs |
| insertion de 100 sous-titres vides au milieu | 61.5 µs | 27.3 µs |
| modification d'un texte, à travers une session | 208 ns | 27.4 ns |
| suppression des mentions sur 4000 sous-titres | 976 µs | 76.2 µs |
| déduction de fréquence sur 4000 sous-titres | 422 µs | 12.8 µs |

### 0.9.18 — 2026-09-08 — Release — charge 1.16 — allure ×1.10

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.09 µs | 3.59 µs |
| composer une réplique de deux lignes | 203 ns | 3.75 ns |
| ouvrir une vidéo | 9.85 ms | 880 µs |
| chercher une position | 647 µs | 131 µs |
| construction du modèle sur 4000 sous-titres | 14.1 µs | 6.69 µs |
| une fenêtre de 40 lignes, cinq colonnes | 19.1 µs | 1.59 µs |
| rafraîchir après un décalage de 4000 sous-titres | 15.5 µs | 22.6 µs |
| réinitialisation du modèle après une ligne retirée | 15.3 µs | 8 µs |
| édition d'une cellule de texte | 492 ns | 88 ns |
| édition d'une cellule de position | 14.5 µs | 1.94 µs |
| versionString | 87.8 ns | 145 ns |
| parse | 67.3 ns | 6.69 ns |
| format | 60.2 ns | 17.3 ns |
| position vers image | 10.2 ns | 1.84 ns |
| image vers position | 8.13 ns | 0.635 ns |
| mise à l'échelle par un rationnel exact | 8.34 ns | 0.433 ns |
| lecture de 4000 sous-titres | 4.75 ms | 1.61 ms |
| écriture de 4000 sous-titres | 1.21 ms | 603 µs |
| décalage de 4000 sous-titres | 11 µs | 5.35 µs |
| décalage puis annulation | 24.7 µs | 8.8 µs |
| transformation de 4000 sous-titres | 94.5 µs | 11.3 µs |
| conversion de fréquence sur 4000 sous-titres | 87.6 µs | 13.7 µs |
| alignement sur 4000 sous-titres | 156 µs | 8.17 µs |
| tri de 4000 sous-titres à l'envers | 352 µs | 49.6 µs |
| suppression d'un sous-titre sur deux | 200 µs | 27.5 µs |
| suppression puis annulation | 363 µs | 216 µs |
| insertion de 100 sous-titres vides au milieu | 83.7 µs | 67.5 µs |
| modification d'un texte, à travers une session | 216 ns | 14.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.1 ms | 134 µs |
| déduction de fréquence sur 4000 sous-titres | 455 µs | 22.8 µs |

### 0.9.17 — 2026-09-08 — Release — charge 0.96 — allure ×1.23

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.98 µs | 1.66 µs |
| composer une réplique de deux lignes | 200 ns | 53.5 ns |
| ouvrir une vidéo | 9.96 ms | 1.59 ms |
| chercher une position | 677 µs | 387 µs |
| construction du modèle sur 4000 sous-titres | 13.6 µs | 5.26 µs |
| une fenêtre de 40 lignes, cinq colonnes | 36.5 µs | 17.2 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.4 µs | 10.4 µs |
| réinitialisation du modèle après une ligne retirée | 13.1 µs | 15 µs |
| édition d'une cellule de texte | 1.46 µs | 403 ns |
| édition d'une cellule de position | 39.7 µs | 54.6 µs |
| versionString | 68.8 ns | 7.34 ns |
| parse | 74.9 ns | 21.4 ns |
| format | 68 ns | 7.86 ns |
| position vers image | 12 ns | 6.56 ns |
| image vers position | 12.3 ns | 1.23 ns |
| mise à l'échelle par un rationnel exact | 15.3 ns | 3.72 ns |
| lecture de 4000 sous-titres | 7.9 ms | 1.44 ms |
| écriture de 4000 sous-titres | 2.57 ms | 1.04 ms |
| décalage de 4000 sous-titres | 58.5 µs | 64.3 µs |
| décalage puis annulation | 60.3 µs | 87.9 µs |
| transformation de 4000 sous-titres | 413 µs | 592 µs |
| conversion de fréquence sur 4000 sous-titres | 78.2 µs | 8.87 µs |
| alignement sur 4000 sous-titres | 142 µs | 9.16 µs |
| tri de 4000 sous-titres à l'envers | 304 µs | 21.7 µs |
| suppression d'un sous-titre sur deux | 211 µs | 29.1 µs |
| suppression puis annulation | 284 µs | 25.2 µs |
| insertion de 100 sous-titres vides au milieu | 58.3 µs | 19.5 µs |
| modification d'un texte, à travers une session | 200 ns | 24.3 ns |
| suppression des mentions sur 4000 sous-titres | 1.09 ms | 114 µs |
| déduction de fréquence sur 4000 sous-titres | 425 µs | 21.6 µs |

### 0.9.16 — 2026-09-07 — Release — charge 1.44 — allure ×0.97

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.47 µs | 1.52 µs |
| composer une réplique de deux lignes | 197 ns | 16.1 ns |
| ouvrir une vidéo | 9.26 ms | 621 µs |
| chercher une position | 584 µs | 94.1 µs |
| construction du modèle sur 4000 sous-titres | 10.6 µs | 3.48 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.8 µs | 807 ns |
| rafraîchir après un décalage de 4000 sous-titres | 11.7 µs | 4.4 µs |
| réinitialisation du modèle après une ligne retirée | 11.2 µs | 4.05 µs |
| édition d'une cellule de texte | 413 ns | 144 ns |
| édition d'une cellule de position | 12.3 µs | 1.06 µs |
| versionString | 33 ns | 5.88 ns |
| parse | 35.2 ns | 3.73 ns |
| format | 36.9 ns | 1.45 ns |
| position vers image | 7.82 ns | 1.7 ns |
| image vers position | 7.75 ns | 1.87 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0571 ns |
| lecture de 4000 sous-titres | 2.15 ms | 124 µs |
| écriture de 4000 sous-titres | 603 µs | 29.9 µs |
| décalage de 4000 sous-titres | 8.7 µs | 2.45 µs |
| décalage puis annulation | 14.8 µs | 4.13 µs |
| transformation de 4000 sous-titres | 81.5 µs | 15.8 µs |
| conversion de fréquence sur 4000 sous-titres | 80.3 µs | 24.2 µs |
| alignement sur 4000 sous-titres | 135 µs | 21.7 µs |
| tri de 4000 sous-titres à l'envers | 318 µs | 44.8 µs |
| suppression d'un sous-titre sur deux | 158 µs | 22.1 µs |
| suppression puis annulation | 269 µs | 25.2 µs |
| insertion de 100 sous-titres vides au milieu | 54.6 µs | 18.9 µs |
| modification d'un texte, à travers une session | 202 ns | 29.1 ns |
| suppression des mentions sur 4000 sous-titres | 935 µs | 140 µs |
| déduction de fréquence sur 4000 sous-titres | 423 µs | 10.2 µs |

### 0.9.14 — 2026-09-07 — Release — charge 1.29 — allure ×1.02

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.07 µs | 3.43 µs |
| composer une réplique de deux lignes | 190 ns | 4.28 ns |
| ouvrir une vidéo | 9.36 ms | 459 µs |
| chercher une position | 592 µs | 150 µs |
| construction du modèle sur 4000 sous-titres | 8.62 µs | 2.08 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.2 µs | 562 ns |
| rafraîchir après un décalage de 4000 sous-titres | 9.74 µs | 2.45 µs |
| réinitialisation du modèle après une ligne retirée | 8.78 µs | 2.71 µs |
| édition d'une cellule de texte | 468 ns | 74.5 ns |
| édition d'une cellule de position | 9.83 µs | 949 ns |
| versionString | 39 ns | 5.49 ns |
| parse | 39.9 ns | 2.75 ns |
| format | 40.4 ns | 10 ns |
| position vers image | 8.01 ns | 0.0926 ns |
| image vers position | 8.03 ns | 0.181 ns |
| mise à l'échelle par un rationnel exact | 8.08 ns | 1.15 ns |
| lecture de 4000 sous-titres | 2.58 ms | 179 µs |
| écriture de 4000 sous-titres | 628 µs | 77.9 µs |
| décalage de 4000 sous-titres | 8.64 µs | 5.43 µs |
| décalage puis annulation | 16.1 µs | 8.15 µs |
| transformation de 4000 sous-titres | 85.9 µs | 13.7 µs |
| conversion de fréquence sur 4000 sous-titres | 80.4 µs | 8.73 µs |
| alignement sur 4000 sous-titres | 151 µs | 9.8 µs |
| tri de 4000 sous-titres à l'envers | 324 µs | 74.9 µs |
| suppression d'un sous-titre sur deux | 186 µs | 33.2 µs |
| suppression puis annulation | 277 µs | 38.3 µs |
| insertion de 100 sous-titres vides au milieu | 56.7 µs | 16.3 µs |
| modification d'un texte, à travers une session | 203 ns | 18 ns |
| suppression des mentions sur 4000 sous-titres | 1.02 ms | 57.1 µs |
| déduction de fréquence sur 4000 sous-titres | 433 µs | 26.3 µs |

### 0.9.13 — 2026-09-07 — Release — charge 1.35 — allure ×1.27

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.06 µs | 2.4 µs |
| composer une réplique de deux lignes | 191 ns | 3.41 ns |
| ouvrir une vidéo | 10.7 ms | 2.87 ms |
| chercher une position | 5.62 ms | 7.65 ms |
| construction du modèle sur 4000 sous-titres | 16.4 µs | 3.58 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.4 µs | 622 ns |
| rafraîchir après un décalage de 4000 sous-titres | 12.7 µs | 3.12 µs |
| réinitialisation du modèle après une ligne retirée | 13.1 µs | 5.94 µs |
| édition d'une cellule de texte | 786 ns | 354 ns |
| édition d'une cellule de position | 18.7 µs | 6.21 µs |
| versionString | 68.1 ns | 12.3 ns |
| parse | 66.2 ns | 17.3 ns |
| format | 75.2 ns | 11.8 ns |
| position vers image | 26.9 ns | 134 ns |
| image vers position | 21.6 ns | 57.2 ns |
| mise à l'échelle par un rationnel exact | 18.4 ns | 4.17 ns |
| lecture de 4000 sous-titres | 7.78 ms | 3.24 ms |
| écriture de 4000 sous-titres | 4.03 ms | 3.94 ms |
| décalage de 4000 sous-titres | 11.8 µs | 8.04 µs |
| décalage puis annulation | 15.4 µs | 4.93 µs |
| transformation de 4000 sous-titres | 85.1 µs | 16.4 µs |
| conversion de fréquence sur 4000 sous-titres | 85.6 µs | 22.2 µs |
| alignement sur 4000 sous-titres | 155 µs | 23.1 µs |
| tri de 4000 sous-titres à l'envers | 336 µs | 53 µs |
| suppression d'un sous-titre sur deux | 199 µs | 43.1 µs |
| suppression puis annulation | 283 µs | 30.5 µs |
| insertion de 100 sous-titres vides au milieu | 51.6 µs | 17.3 µs |
| modification d'un texte, à travers une session | 221 ns | 51.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.07 ms | 103 µs |
| déduction de fréquence sur 4000 sous-titres | 424 µs | 14.6 µs |

### 0.9.12 — 2026-09-06 — Release — charge 1.49 — allure ×0.93

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.83 µs | 1.82 µs |
| composer une réplique de deux lignes | 178 ns | 3.95 ns |
| ouvrir une vidéo | 9.4 ms | 862 µs |
| chercher une position | 568 µs | 156 µs |
| construction du modèle sur 4000 sous-titres | 8.98 µs | 7.52 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.3 µs | 449 ns |
| rafraîchir après un décalage de 4000 sous-titres | 9.69 µs | 2.78 µs |
| réinitialisation du modèle après une ligne retirée | 9.56 µs | 2.86 µs |
| édition d'une cellule de texte | 383 ns | 64.2 ns |
| édition d'une cellule de position | 9.27 µs | 1.72 µs |
| versionString | 33.3 ns | 4.21 ns |
| parse | 38.3 ns | 4.07 ns |
| format | 39 ns | 0.583 ns |
| position vers image | 7.67 ns | 0.548 ns |
| image vers position | 7.62 ns | 0.0937 ns |
| mise à l'échelle par un rationnel exact | 7.89 ns | 0.217 ns |
| lecture de 4000 sous-titres | 2.14 ms | 107 µs |
| écriture de 4000 sous-titres | 545 µs | 47.2 µs |
| décalage de 4000 sous-titres | 7.63 µs | 3.51 µs |
| décalage puis annulation | 13.1 µs | 3.13 µs |
| transformation de 4000 sous-titres | 73.9 µs | 12.2 µs |
| conversion de fréquence sur 4000 sous-titres | 84.4 µs | 50.3 µs |
| alignement sur 4000 sous-titres | 136 µs | 41.9 µs |
| tri de 4000 sous-titres à l'envers | 299 µs | 20 µs |
| suppression d'un sous-titre sur deux | 176 µs | 21.2 µs |
| suppression puis annulation | 242 µs | 35.7 µs |
| insertion de 100 sous-titres vides au milieu | 50.3 µs | 17.8 µs |
| modification d'un texte, à travers une session | 204 ns | 20.7 ns |
| suppression des mentions sur 4000 sous-titres | 942 µs | 45.3 µs |
| déduction de fréquence sur 4000 sous-titres | 400 µs | 26.3 µs |

### 0.9.10 — 2026-09-06 — Release — charge 1.19 — allure ×1.00

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.51 µs | 1.62 µs |
| composer une réplique de deux lignes | 212 ns | 66.2 ns |
| ouvrir une vidéo | 9.41 ms | 913 µs |
| chercher une position | 591 µs | 170 µs |
| construction du modèle sur 4000 sous-titres | 9.63 µs | 2.45 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.1 µs | 4.48 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.55 µs | 2.34 µs |
| réinitialisation du modèle après une ligne retirée | 9.5 µs | 1.52 µs |
| édition d'une cellule de texte | 431 ns | 51.5 ns |
| édition d'une cellule de position | 10.5 µs | 928 ns |
| versionString | 37.3 ns | 6.77 ns |
| parse | 39.1 ns | 10.1 ns |
| format | 42.9 ns | 16.9 ns |
| position vers image | 7.62 ns | 0.082 ns |
| image vers position | 7.63 ns | 0.22 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.0796 ns |
| lecture de 4000 sous-titres | 2.49 ms | 279 µs |
| écriture de 4000 sous-titres | 1.04 ms | 605 µs |
| décalage de 4000 sous-titres | 7.79 µs | 3.46 µs |
| décalage puis annulation | 15.4 µs | 3.25 µs |
| transformation de 4000 sous-titres | 84.7 µs | 11.1 µs |
| conversion de fréquence sur 4000 sous-titres | 79.5 µs | 15.2 µs |
| alignement sur 4000 sous-titres | 135 µs | 16.6 µs |
| tri de 4000 sous-titres à l'envers | 262 µs | 23 µs |
| suppression d'un sous-titre sur deux | 189 µs | 45.5 µs |
| suppression puis annulation | 264 µs | 22.4 µs |
| insertion de 100 sous-titres vides au milieu | 61.4 µs | 22.3 µs |
| modification d'un texte, à travers une session | 195 ns | 20.5 ns |
| suppression des mentions sur 4000 sous-titres | 882 µs | 81 µs |
| déduction de fréquence sur 4000 sous-titres | 426 µs | 15.6 µs |

### 0.9.8 — 2026-09-06 — Release — charge 1.44 — allure ×0.95

| Mesure | Moyenne | Écart-type |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.68 µs | 936 ns |
| composer une réplique de deux lignes | 233 ns | 97.9 ns |
| ouvrir une vidéo | 9.57 ms | 979 µs |
| chercher une position | 577 µs | 153 µs |
| construction du modèle sur 4000 sous-titres | 8.34 µs | 1.49 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17 µs | 979 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.38 µs | 2.38 µs |
| réinitialisation du modèle après une ligne retirée | 7.37 µs | 3.09 µs |
| édition d'une cellule de texte | 374 ns | 61.1 ns |
| édition d'une cellule de position | 8.21 µs | 1.39 µs |
| versionString | 31.1 ns | 5.47 ns |
| parse | 33.3 ns | 3.66 ns |
| format | 38 ns | 8.34 ns |
| position vers image | 6.64 ns | 0.072 ns |
| image vers position | 6.64 ns | 0.0687 ns |
| mise à l'échelle par un rationnel exact | 7.88 ns | 0.119 ns |
| lecture de 4000 sous-titres | 2.53 ms | 195 µs |
| écriture de 4000 sous-titres | 536 µs | 37.3 µs |
| décalage de 4000 sous-titres | 10.3 µs | 5.28 µs |
| décalage puis annulation | 17.4 µs | 11.4 µs |
| transformation de 4000 sous-titres | 90.7 µs | 16.6 µs |
| conversion de fréquence sur 4000 sous-titres | 70.6 µs | 11.2 µs |
| alignement sur 4000 sous-titres | 146 µs | 3.39 µs |
| tri de 4000 sous-titres à l'envers | 336 µs | 46.2 µs |
| suppression d'un sous-titre sur deux | 157 µs | 23.7 µs |
| suppression puis annulation | 239 µs | 36.5 µs |
| insertion de 100 sous-titres vides au milieu | 53.9 µs | 22.4 µs |
| modification d'un texte, à travers une session | 229 ns | 127 ns |
| suppression des mentions sur 4000 sous-titres | 1.06 ms | 182 µs |
| déduction de fréquence sur 4000 sous-titres | 370 µs | 12.5 µs |
