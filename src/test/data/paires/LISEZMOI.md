# Une traduction, et ce qu'on en fait quand on l'ouvre

Huit cas, un même document principal — la scène de [`formats/`](../formats), sans
l'italique — et huit traductions de lui, écrites chacune pour **séparer les deux
façons dont Gaupol rattache un fichier de traduction** : par numéro, et par position.
À côté de chaque traduction, ce que chaque méthode doit en faire.

Le modèle porte les deux textes d'un sous-titre depuis la phase 1, et rien ne les
faisait entrer par un fichier. Ces cas sont ce que la phase 11 pourra confronter à
ce qu'elle écrit, et ils ont été écrits **avant** — pour la raison de
[`formats/`](../formats/LISEZMOI.md) : une fixture tirée de la compréhension qu'on a
d'un algorithme, relue par l'implémentation de ce même algorithme, ne prouve que leur
accord.

## La scène

Quatre sous-titres, dont deux sur deux lignes. Les positions sont celles de
`formats/scene.srt`.

| # | Début | Fin | Texte |
| -: | :--- | :--- | :---- |
| S1 | 00:00:01,000 | 00:00:03,000 | `The harbour wakes before` ⏎ `the town does.` |
| S2 | 00:00:04,000 | 00:00:06,000 | `Nothing moves on the water.` |
| S3 | 00:00:08,000 | 00:00:11,000 | `A gull turns once above the mast` ⏎ `and settles.` |
| S4 | 00:00:12,000 | 00:00:15,000 | `Then the light comes.` |

**Le principal est en anglais et les traductions en français**, pour qu'on ne prenne
jamais un texte pour l'autre en lisant un résultat. Chaque ligne de traduction est
nommée plus bas par le sous-titre qu'elle traduit : `T1` traduit S1, et ainsi de suite.

## Les cas

Huit répertoires, dont chacun tient **six fichiers, ni plus ni moins** :

| Fichier | Ce que c'est |
| :------ | :----------- |
| `principal.srt` | le document principal, le même dans les huit cas |
| `traduction.srt` | le fichier de traduction qu'on ouvre par-dessus |
| `attendu-numero.principal.srt` `attendu-numero.traduction.srt` | le résultat de l'alignement **par numéro**, écrit deux fois : les textes principaux, puis les traductions |
| `attendu-position.principal.srt` `attendu-position.traduction.srt` | le résultat de l'alignement **par position**, de même |

**Un résultat est une seule liste de sous-titres écrite deux fois**, une fois pour
chacun de ses textes, aux mêmes positions : c'est ce que l'écrivain sait déjà produire
(`WriteRequest::document`), et un attendu qu'un humain relit dans un diff plutôt
qu'une table à apprendre. Un sous-titre que personne n'a traduit s'écrit comme un
bloc sans texte dans le second fichier ; un sous-titre né d'une ligne de traduction,
comme un bloc sans texte dans le premier. **Un bloc sans texte se lit sans anomalie et
revient à l'octet** — c'est vérifié, voir plus bas.

| Cas | La traduction | Par numéro | Par position |
| :-- | :------------ | :--------- | :----------- |
| `temoin` | quatre lignes, aux positions du principal | chaque ligne sur son sous-titre | **la même chose** |
| `traduction-plus-courte` | trois lignes : celle de S4 manque | S1 à S3 traduits, S4 sans traduction | **la même chose** |
| `traduction-dans-le-desordre` | les quatre lignes, dans l'ordre 2, 1, 4, 3 du fichier | elle est triée d'abord : comme le témoin | **la même chose** |
| `une-ligne-de-moins-au-milieu` | trois lignes : celle de S2 manque | **glissement** : S2 reçoit `T3`, S3 reçoit `T4`, S4 reste sans traduction | S2 sans traduction, les trois autres à leur place |
| `une-ligne-de-plus-a-la-fin` | cinq lignes, la cinquième à 16–18 s | un sous-titre naît à **15–18 s** — trois secondes inventées, à la suite du dernier | un sous-titre naît à **16–18 s** — les positions de la traduction |
| `une-ligne-dans-un-intervalle-vide` | cinq lignes, une à 6,5–7,5 s, entre S2 et S3 | **glissement** : S3 reçoit cette ligne, S4 reçoit `T3`, et `T4` fait naître un sous-titre à 15–18 s | un sous-titre naît à 6,5–7,5 s, entre S2 et S3 ; les quatre autres à leur place |
| `deux-lignes-dans-un-meme-sous-titre` | S3 traduit en deux lignes, à 8–9,5 s et 9,5–11 s | la seconde glisse sur S4, et `T4` fait naître un sous-titre à 15–18 s | la seconde **fait naître** un sous-titre à 9,5–11 s ; S4 garde `T4` |
| `positions-decalees` | quatre lignes, toutes deux secondes plus tard | les positions sont ignorées : comme le témoin | `T1` tombe sur **S2**, les trois autres font naître un sous-titre chacune ; S1, S3 et S4 restent sans traduction — sept sous-titres |

**Trois cas s'accordent et cinq non, et les cinq sont ceux qui comptent** : un cas où
les deux méthodes rendent la même chose n'apprend rien sur laquelle choisir. Le témoin
est là pour montrer qu'elles peuvent s'accorder ; les deux autres, pour dire ce qui ne
sépare pas les méthodes — une traduction plus courte, une traduction désordonnée.

## Ce que chaque cas demande au cadrage

Ces cas posent leurs questions avant qu'on implémente, ce qui est leur raison d'être.
Elles sont versées à #426 (décision 4).

- **`une-ligne-de-moins-au-milieu`** — laquelle des deux méthodes est le défaut. Par
  numéro, une seule ligne manquante décale **toutes** les suivantes, sans que rien ne
  le dise ; par position, une seule reste sans traduction.
- **`une-ligne-de-plus-a-la-fin`, `une-ligne-dans-un-intervalle-vide`** — quelles
  positions reçoit un sous-titre né d'une ligne orpheline, et si on le dit. Les deux
  méthodes n'en donnent pas les mêmes.
- **`deux-lignes-dans-un-meme-sous-titre`** — si une traduction qui scinde un
  sous-titre en deux doit faire naître un sous-titre neuf.
- **`traduction-dans-le-desordre`** — si le nombre de lignes déplacées par le tri, que
  Gaupol rend (ici **2**), est dit à l'utilisateur.
- **`positions-decalees`** — Gaupol n'a aucune détection de décalage : une traduction
  calée deux secondes trop tard n'est pas reconnue pour ce qu'elle est. À nommer, pas à
  corriger.

## Elles s'écrivent à la main, et il n'y aura pas de fabrique

Pour la raison de `formats/` : ce qui vérifierait ces fichiers serait le code qui les
lit et les aligne, et un fichier engendré depuis notre compréhension de l'alignement ne
prouverait que leur accord.

**Ce qui tient lieu de tiers est une lecture de `aeidon/agents/open.py`**, de
`_align_translations_by_number` et `_align_translations_by_position`, et de
`_insert_blank_subtitles` — la fonction qui invente les trois secondes. Chaque attendu
a été calculé à la main d'après elle : le pointeur qui n'avance jamais, le milieu de
chaque ligne comparé aux bornes des sous-titres, la fenêtre de 3 s à la suite du dernier.

### Et ils ont été observés, une fois

Les seize résultats — huit cas, deux méthodes — ont ensuite été confrontés à ce que
Gaupol fait réellement : le paquet `aeidon` copié **hors** du clone, importé depuis la
copie, `Project.open_main` puis `open_translation(chemin, encodage, méthode)` sur chaque
cas. **Les seize concordent**, et le nombre de lignes déplacées du cas désordonné est
bien 2.

**Cette confrontation est faite à la main et ne se rejoue pas.** Aucun test ne lit le
clone de Gaupol : il est absent de la CI et d'une machine fraîche. Elle se refait en
un petit script — copier `aeidon/` ailleurs, l'importer, comparer — et elle n'est pas
dans le dépôt, parce qu'un script qui dépend du clone serait un test que personne d'autre
ne peut lancer.

### Ce que l'observation a corrigé

**Deux affirmations de l'issue qui a demandé ces fichiers se sont révélées à côté.**

- Elle écrivait de `positions-decalees` que *« chaque ligne fait naître un sous-titre »*.
  **Non** : la première ligne, décalée de deux secondes, a son milieu à 4 s, et 4 s
  tombe **dans** S2 — elle s'y colle. Trois lignes font naître un sous-titre, pas
  quatre, et S2 reçoit une traduction qui n'est pas la sienne sans que rien ne la
  distingue d'un rattachement juste.
- Elle marquait `deux-lignes-dans-un-meme-sous-titre` comme *« lu, pas observé »*. C'est
  maintenant observé, et c'est bien ce que la lecture de la boucle laissait attendre :
  la seconde ligne fait naître un sous-titre, le pointeur ayant déjà avancé.

## Ce que ces fixtures ne portent pas

- **Ouvrir une traduction sur un projet déjà traduit efface l'ancienne** — Gaupol vide
  d'abord toutes les traductions. Le cas avait été proposé et il est **écarté** : ce
  n'est pas une propriété d'une paire de fichiers mais de l'état d'un projet, qu'un
  fichier ne peut pas porter. Il s'éprouvera par un test sur un `Project` en mémoire,
  dans l'issue qui écrit l'ouverture.
- **Le nombre de lignes déplacées par le tri** est une valeur rendue, pas un fichier : il
  est écrit dans la table ci-dessus, et le test ne peut pas le lire ici.
- **Aucun de ces cas n'est prélevé.** Ils sont écrits, pas tirés d'un fichier réel.

## Ce que le harnais leur demande

`pairs_corpus_test.cpp`, sous `src/test/unit/core/format/`, ne rattache rien : il
éprouve **les données**, et ce sont les garde-fous que nul alignement ne peut casser.

- le répertoire et la table du test nomment les mêmes cas, **dans les deux sens** ;
- chaque cas tient exactement ses six fichiers ;
- chaque fichier est du SubRip qui se lit sans anomalie et **revient à l'octet** — un
  bloc sans texte compris ;
- les huit cas partent du même principal ;
- les deux fichiers d'un résultat ont les mêmes positions ;
- un sous-titre né d'une ligne de traduction n'a pas de texte principal, et les quatre
  textes de la scène survivent, dans leur ordre ;
- **chaque ligne de traduction atterrit exactement une fois**, quelle que soit la
  méthode : rien de perdu, rien d'inventé, rien de compté deux fois — ce qu'un attendu
  écrit à la main peut faire faux sans qu'un lecteur le voie ;
- les deux méthodes diffèrent **exactement** là où la table le dit ;
- seul le cas du désordre a une traduction dont les débuts ne sont pas triés.

La première issue d'implémentation ajoutera ce qui manque : ouvrir chaque traduction
sur son principal, et comparer le résultat à l'attendu octet pour octet.

## Pourquoi pas dans `valides/`

Depuis #289, `validFiles()` (`core/format/corpus_test.cpp`) retient **tout fichier
régulier** de `valides/` et exige qu'il s'ouvre : ce répertoire *est* la liste. Un
`LISEZMOI.md` y ferait échouer le test, et une traduction sans son principal y serait
lue comme un document à part entière. Les paires ont leur répertoire, et le leur.
