# La table

La fenêtre montre les sous-titres du fichier ouvert, un par ligne, dans cinq
colonnes.

**La fenêtre est en anglais**, comme la ligne de commande. La traduction est
une phase à elle seule ; ce manuel cite donc les intitulés tels qu'ils
s'affichent.

| Colonne | Ce qu'elle montre |
| :------ | :---------------- |
| `#` | le rang du sous-titre, à partir de 1 |
| `Start` | la position d'apparition, `HH:MM:SS,mmm` |
| `End` | la position de disparition |
| `Duration` | `End − Start` |
| `Text` | le texte du sous-titre |

**Le numéro n'est pas une donnée du fichier** mais le rang de la ligne. Une
insertion ou une suppression renumérote donc tout ce qui suit, sans que rien ne
soit réécrit.

**Le séparateur décimal suit le format du fichier** : une virgule pour SubRip,
un point pour WebVTT. Ce qui s'affiche est ce qui sera écrit.

**La durée (`Duration`) est calculée**, jamais saisie. Un sous-titre dont la fin précède le
début affiche une durée négative plutôt que zéro : c'est une anomalie du
fichier, et la masquer la rendrait introuvable.

**Le début, la fin et le texte s'éditent en place** ; le numéro et la durée non.
Voir [Éditer une cellule](edition.md).

**La sélection désigne ce sur quoi une opération porte** — voir
[Les opérations](operations.md).

**Un texte de plusieurs lignes n'en montre qu'une** : la hauteur des lignes de
la table est fixe, et le reste est coupé à l'affichage. Rien n'est perdu — le
texte entier réapparaît dès qu'on ouvre la cellule, et c'est lui qui sera
écrit.

## Ce que la table ne fait pas encore

Ajouter ou supprimer une ligne n'est pas possible. Le tri par colonne n'existe
pas — l'ordre affiché est celui du fichier, et c'est le seul qui ait un sens
pour des sous-titres.
