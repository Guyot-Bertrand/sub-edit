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
| `gaupol/` | les douze fichiers de motifs de Gaupol et leurs douze `.conf` d'activation | copiés, jamais retouchés |
| `entrees/` | les textes à éprouver, un fichier par type de motif | à la main |
| `attendus/` | ce que Gaupol fait de chaque texte, au format des `.cas` | `src/scripts/pattern-oracle.py` |

## `gaupol/` — les motifs, tels que Gaupol les livre

Copiés du dépôt de Gaupol à la révision `f433bae3d23ef3572411fa9adca3fbab5cb1b3f9`
(2026-07-28), depuis le répertoire `aeidon/data/patterns` de celui-ci. **Gaupol est
sous GPL-3, comme ce projet** : la copie est permise, et cette section en est
l'attribution — les motifs sont l'œuvre des auteurs de Gaupol, pas de ce dépôt.

**Ne jamais les retoucher à la main.** Ce sont eux que le moteur devra lire, et un
motif « réparé » ici cesserait d'être celui de Gaupol. Une mise à jour se fait en
recopiant l'ensemble depuis une révision plus récente de Gaupol, en changeant la
révision ci-dessus, puis en relançant l'oracle : le diff des attendus dit alors ce
que la mise à jour change.

Les `line-break` y sont aussi, bien que l'oracle ne les lise pas : le découpage de
lignes a ses propres attendus, issue
[#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495).

## `entrees/` — les textes à éprouver

Un fichier par type : `common-error`, `capitalization`, `hearing-impaired`. Une
ligne par cas :

```
cible | attente | libellé | "texte"
```

| Champ | Ce qu'il dit |
| :---- | :----------- |
| `cible` | un enregistrement seul — `Latn-en:3`, le troisième de `Latn-en.<type>`, appliqué qu'il soit actif ou non —, ou une cascade — `cascade Latn-en`, telle que Gaupol l'active par défaut, `cascade Latn-en tous`, tout coché |
| `attente` | `corrige` ou `intact` : ce que le cas **annonce** |
| `libellé` | ce que le cas montre, en français |
| `"texte"` | le texte, entre guillemets ; plusieurs textes séparés par `;` sont des sous-titres consécutifs |

Les échappements sont ceux des `.cas` : `\"`, `\\`, `\n`, `\t`.

**L'oracle refuse un cas qui ne fait pas ce qu'il annonce** — un `corrige` qui ne
change rien, un `intact` qui change —, **un enregistrement qui n'a pas un cas de
chaque sorte**, et une langue livrée sans sa cascade. C'est ce qui garde ces
entrées honnêtes : un cas `intact` se place **au bord** du motif, là où un moteur
qui traduirait mal un lookbehind corrigerait trop.

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
recherche et ses trois opérations, **ligne à ligne**. Chacune est nommée dans le
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

C'est **une observation, pas un contrôle** : elle demande Gaupol, et le dépôt ne le
contient pas. Ce qui se rejoue partout, c'est l'oracle sur les copies versionnées,
et c'est lui que la porte lance.
