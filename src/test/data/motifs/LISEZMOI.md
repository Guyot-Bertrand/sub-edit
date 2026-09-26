# Les motifs de correction de Gaupol, et ce qu'il en fait

Les motifs de correction de Gaupol, versionnés tels quels, des textes écrits à la
main pour les éprouver, et **ce que Gaupol en fait**, écrit par un oracle. C'est ce
que le moteur de correction de la phase 12 devra reproduire, quel que soit le
moteur d'expressions rationnelles retenu. Issue
[#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494), outil retenu par
l'initialisation de la phase ([#492](https://github.com/Guyot-Bertrand/sub-edit/issues/492)).

Ces attendus ont été écrits **avant** le moteur, pour la raison de
[`formats/`](../formats/LISEZMOI.md) : une fixture tirée de la compréhension qu'on
a d'un algorithme, relue par l'implémentation de ce même algorithme, ne prouve que
leur accord.

## Ce qu'il y a ici

| Répertoire | Ce que c'est | Écrit par |
| :--------- | :----------- | :-------- |
| [`packaging/patterns/`](../../../../packaging/patterns/) | les douze fichiers de motifs de Gaupol et leurs douze `.conf` d'activation | copiés, jamais retouchés |
| `entrees/` | les textes à éprouver, un fichier par type de motif | à la main |
| `attendus/` | ce que Gaupol fait de chaque texte, au format des `.cas`, et **la cascade de chaque code** (`cascades.txt`) | `src/scripts/pattern-oracle.py` |

## `packaging/patterns/` — les motifs, tels que Gaupol les livre

Ils ne vivent plus ici : le programme les installe et les lit, l'oracle et les
tests avec lui, et il n'y a donc qu'une copie. Leur provenance, leur attribution
et la règle de mise à jour sont dans
[`packaging/patterns/LISEZMOI.md`](../../../../packaging/patterns/LISEZMOI.md).
Issue [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498).

## `entrees/` — les textes à éprouver

Un fichier par type : `common-error`, `capitalization`, `hearing-impaired`,
`line-break`. Une ligne par cas :

```
cible | attente | libellé | "texte"
```

| Champ | Ce qu'il dit |
| :---- | :----------- |
| `cible` | un enregistrement seul — `Latn-en:3`, le troisième de `Latn-en.<type>`, appliqué qu'il soit actif ou non —, ou une cascade — `cascade Latn-en`, telle que Gaupol l'active par défaut, `cascade Latn-en tous`, tout coché |
| `attente` | `corrige` ou `intact` : ce que le cas **annonce** |
| `libellé` | ce que le cas montre, en français |
| `"texte"` | le texte, entre guillemets ; plusieurs textes séparés par `;` sont des sous-titres consécutifs |

Les échappements sont ceux des `.cas` : `\"`, `\\`, `\n`, `\t`. Les espaces
d'alignement d'une cible ne comptent pas.

**L'oracle refuse un cas qui ne fait pas ce qu'il annonce** — un `corrige` qui ne
change rien, un `intact` qui change —, **un enregistrement qui n'a pas un cas de
chaque sorte**, et une langue livrée sans sa cascade. C'est ce qui garde ces
entrées honnêtes : un cas `intact` se place **au bord** du motif, là où un moteur
qui traduirait mal un lookbehind corrigerait trop.

**Le découpage de lignes a sa propre cible et sa propre attente** — issue
[#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495) :

- la cible finit par les réglages, `<longueur>/<lignes>` — `Latn-en:2 16/2`,
  `cascade Latn-en 42/2` ; deux sélections de plus, `essai` pour les trois
  pénalités de `aeidon/test/test_liner.py` et `aucune` ;
- l'attente est `coupe` ou `intact` ;
- **chaque motif décide au moins un cas** : son découpage diffère de celui
  d'aucune pénalité aux mêmes réglages. L'oracle refuse un motif qui ne décide
  rien, sans quoi un moteur qui l'ignorerait passerait ;
- **en caractères seulement** : la mesure en *ems* dépend de la police, et c'est
  au cadrage d'en décider (#493).

**Aucun texte ne porte de balise.** Le parseur de Gaupol retire les balises et les
remet autour des remplacements ; c'est le travail du parseur de balises de la
phase 10, éprouvé ailleurs.

## `attendus/` — ce que Gaupol en fait

Un `.cas` par type, que `textCasesOf` sait lire : `libellé | "entrée" | "attendu"`,
`=` pour un texte que les motifs laissent, `supprimé` pour un sous-titre que le
retrait des mentions vide. Le libellé commence par la cible et l'attente ; un
sous-titre d'une suite porte son rang, `[2/3]`.

**Engendrés, jamais écrits à la main** :

```console
$ ./src/scripts/pattern-oracle.py --write   # après avoir changé une entrée
$ ./src/scripts/pattern-oracle.py --check   # ce que make fixtures lance
```

## Ce que l'oracle reproduit, et pourquoi pas `re.sub`

L'oracle reproduit le chargement de Gaupol (lignes de commentaire, `\0` retiré,
activation par les `.conf`), sa cascade des codes (`SkipIn`, `Policy=Replace`), sa
recherche, ses trois opérations sur le texte et son découpeur de lignes (`Liner`),
**ligne à ligne**. Chacune est nommée dans le
script à côté de ce qu'elle porte.

**Gaupol ne remplace pas avec `re.sub`.** Il cherche chaque correspondance dans le
texte **déjà modifié**, à partir de la fin du dernier remplacement, avec sa propre
règle pour les correspondances vides. La différence est étroite, et mesurée : des
dizaines de milliers de textes tirés au hasard n'en montrent aucune pour les
erreurs courantes et les majuscules, où `Repeat` rattrape tout. Elle apparaît dans
le nettoyage qui suit le retrait des mentions, et les deux cas qui la montrent sont
commentés dans `entrees/hearing-impaired.entrees`.

## Ce qui a été vérifié, et ce qui ne se rejoue pas

**L'oracle a été confronté au code de Gaupol lui-même** avant d'être versionné :
chaque cas, passé par les fonctions de Gaupol sur une copie de son paquet `aeidon`
faite hors du dépôt, une fois sans fichier et une fois dans un projet SubRip — pour
faire passer le parseur de balises. **Aucun désaccord.** La même confrontation,
refaite avec un oracle réduit à `re.sub`, trouve les deux cas de la section
précédente : elle sait voir une différence.

**Le découpeur a été confronté de même**, au `Liner` de Gaupol aux mêmes réglages
et aux mêmes pénalités : aucun désaccord, et les quatre découpages de
`test_liner.py` y sont exactement ceux que ce test affirme. Trois oracles
faussés — sans le terme de « pyramide », sans la variance des longueurs, sans la
règle qui attend `max_lines` lignes avant de couper — en trouvent chacun.

Deux comportements du découpeur valent d'être sus avant d'en écrire un :

- **il dépasse `max_lines` quand la longueur l'exige** — « It is 40 cm longer than
  mine » sort en trois lignes à 11 de large et deux lignes voulues, pour ne pas
  séparer « 40 » de « cm » ;
- **il ne coupe rien tant que le nombre de lignes essayé n'atteint pas
  `max_lines`** : deux mots trop longs pour une ligne restent sur une seule ligne
  quand on en permet trois, et se coupent quand on en permet deux.

C'est **une observation, pas un contrôle** : elle demande Gaupol, et le dépôt ne le
contient pas. Ce qui se rejoue partout, c'est l'oracle sur les copies versionnées,
et c'est lui que la porte lance.

## `cascades.txt` — les motifs que chaque code active

Une ligne par type et par code : `<type> <code> | <code>:<rang>± …`, les
enregistrements dans l'ordre où Gaupol les applique, `+` s'ils sont actifs par
défaut et `-` sinon. C'est ce que `get_patterns` rend, et **ce que le lecteur du
noyau rend à son tour** — le test `pattern_catalogue_test.cpp` les confronte, sans
lancer Python. Issue [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498).
