# La table

La fenêtre montre les sous-titres du fichier ouvert, un par ligne, dans cinq
colonnes.

| Colonne | Ce qu'elle montre |
| :------ | :---------------- |
| `N°` | le rang du sous-titre, à partir de 1 |
| `Début` | la position d'apparition, `HH:MM:SS,mmm` |
| `Fin` | la position de disparition |
| `Durée` | `Fin − Début` |
| `Texte` | le texte du sous-titre, sauts de ligne compris |

**Le numéro n'est pas une donnée du fichier** mais le rang de la ligne. Une
insertion ou une suppression renumérote donc tout ce qui suit, sans que rien ne
soit réécrit.

**Le séparateur décimal suit le format du fichier** : une virgule pour SubRip,
un point pour WebVTT. Ce qui s'affiche est ce qui sera écrit.

**La durée est calculée**, jamais saisie. Un sous-titre dont la fin précède le
début affiche une durée négative plutôt que zéro : c'est une anomalie du
fichier, et la masquer la rendrait introuvable.

## Ce que la table ne fait pas encore

Rien n'est modifiable : les cellules s'affichent, ne s'éditent pas. La sélection
d'une ligne fonctionne, mais aucune opération ne s'y applique. Le tri par
colonne n'existe pas — l'ordre affiché est celui du fichier, et c'est le seul
qui ait un sens pour des sous-titres.
