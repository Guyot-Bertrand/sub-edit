# 0009 — Stocker le texte comme chaîne brute portant les balises du format

**Date :** 2026-08-05
**Statut :** acceptée

## Contexte

Le texte d'un sous-titre porte des balises de mise en forme, propres à chaque
format : `<i>`, `<b>`, `<u>`, `<font color="#RRGGBB">` pour SubRip ; les mêmes
sans `<font>` pour WebVTT, plus `<c.classe>`, `<v Locuteur>`, `<ruby>` et des
horodatages internes ; des surcharges arbitraires `{\an8}`, `{\pos(x,y)}`,
`{\t(...)}` pour ASS en phase 9.

Deux représentations étaient envisageables : conserver la chaîne telle quelle,
ou décoder vers un modèle structuré — texte plus intervalles de style.

L'analyse de Gaupol a tranché la question de fait. Son format pivot interne
existe **uniquement le temps d'une conversion entre formats** ; rien n'est
stocké sous cette forme. Et ce décodage est destructeur : `markups/webvtt.py`
annonce que les balises comme `<c>` « ne peuvent pas être supportées », et son
`_post_decode` supprime **toute balise non reconnue** :

```python
text = self._substitute(text, r"</?[^biu]>", "")
text = self._substitute(text, r"</?[^/>]{2,}>", "")
```

Gaupol ne perd donc `<v Marie>` ou `<c.loud>` que lorsqu'il convertit. À
l'ouverture, l'édition et l'enregistrement d'un `.vtt`, rien n'est décodé et
rien n'est perdu. **Sa fidélité vient précisément du stockage en chaîne brute.**

## Décision

Le texte est stocké **tel qu'il apparaît dans le fichier**, balises comprises.
Aucun décodage n'a lieu à l'ouverture ni à l'enregistrement.

Les opérations qui transforment le texte passent par un **parseur conscient des
balises** : il retire les balises, mémorise leurs positions, applique la
transformation au texte nu, puis les réinsère.

Un modèle structuré reste utilisé, comme chez Gaupol, **uniquement comme pivot
lors d'une conversion entre formats**, où la perte est inévitable et assumée.

## Alternatives écartées

- **Modèle structuré permanent — texte et intervalles de style.** Sémantiquement
  bien plus propre, et les opérations de texte y deviennent triviales. Écarté
  parce qu'il impose le décodage destructeur **en permanence** : ouvrir un `.vtt`
  contenant `<v Marie>` et l'enregistrer perdrait ces balises. Sur l'un des deux
  formats prioritaires, dès le MVP. La perte ne serait pas différée à la phase 9,
  elle commencerait tout de suite.
- **Modèle structuré avec fragments opaques** — les balises non comprises
  seraient conservées verbatim dans des fragments traités comme atomiques. Cette
  variante préserve la fidélité et garde une sémantique propre. Écartée pour un
  motif de simplicité assumé : elle demande de concevoir dès la phase 1 un modèle
  de fragments dont seule la phase 9 éprouvera la solidité, alors que la chaîne
  brute est fidèle par construction et sans conception préalable. Reste la
  meilleure porte de sortie si le parseur conscient des balises se révélait
  intenable.

## Conséquences

**La fidélité de l'aller-retour est acquise par construction**, pour tous les
formats, y compris pour des balises que nous ne connaissons pas.

En contrepartie, **aucune opération de texte n'est triviale**. Chacune doit
passer par le parseur, sous peine de casser les balises : appliquer `\[.*?\]`
directement au texte pour supprimer une mention pour malentendants effacerait
une balise ouvrante située à l'intérieur des crochets, laissant sa fermante
orpheline. C'est un coût réel et récurrent, payé à chaque opération.

Le parseur est donc une pièce d'infrastructure de premier plan. Il est écrit une
fois, en phase 4 où il devient nécessaire, et réutilisé par les phases 10 et 12.
Sa conception mérite le même soin que le modèle de données, et ses cas limites —
balises chevauchant une correspondance, balises encadrant toutes les lignes,
insertion à la frontière d'une balise — appellent des tests dédiés.

Le modèle ne peut pas garantir que les balises sont bien formées : un `<i>` sans
fermeture reste tel quel. C'est cohérent avec
[0008](0008-lecture-au-mieux-avec-diagnostics.md) — signaler par un diagnostic,
ne pas réparer d'autorité.
