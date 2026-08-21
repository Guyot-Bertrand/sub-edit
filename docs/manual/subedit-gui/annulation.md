# Annuler et rétablir

Chaque opération entre dans un historique, et se défait. Une cellule éditée
compte pour **une** entrée : la frappe ne s'y inscrit pas touche par touche,
seule la validation compte.

| Action | Raccourci | Où |
| :----- | :-------- | :- |
| Annuler | `Ctrl+Z` | menu **Édition**, barre d'outils |
| Rétablir | `Ctrl+Maj+Z` | menu **Édition**, barre d'outils |

## L'action nomme ce qu'elle défera

Le menu ne dit pas « Annuler » mais **« Annuler : modification du texte »**, du
nom de l'opération qui sera défaite. Le bouton de la barre d'outils garde le mot
seul — sa largeur suivrait sinon la dernière opération, et il bougerait sous le
pointeur — mais son infobulle porte le libellé entier.

| Opération | Ce que l'action lit |
| :-------- | :------------------ |
| une cellule de texte éditée | modification du texte |
| un début, une fin | modification du début, modification de la fin |
| un décalage | décalage |
| une transformation | transformation |
| une conversion de fréquence | conversion de fréquence |
| un tri | tri |
| un retrait des mentions | retrait des mentions |
| une insertion, une suppression | insertion, suppression |

Quand il n'y a rien à défaire, l'action lit « Annuler » tout court **et elle est
inactive** : son raccourci ne fait rien.

## La marque de modification

Le titre de la fenêtre porte une marque tant que le fichier diffère de celui du
disque — une astérisque, ou ce que la plateforme utilise à sa place.

**Elle disparaît si on annule jusqu'au point de départ.** Ce n'est pas un
détail d'affichage : le nombre de modifications est compté, pas noté par un
oui-ou-non, donc revenir en arrière peut le ramener à zéro et la marque s'en va.

## Ce que l'historique ne fait pas encore

**Rien ne s'enregistre**, donc le point de référence est toujours l'ouverture du
fichier. « Enregistrer » viendra le déplacer.

Une opération refaite après une annulation **efface ce qu'il y avait à
rétablir** : rétablir rejouerait une commande dont l'état de départ n'existe
plus. C'est le comportement de tous les éditeurs, et il est délibéré.

L'historique garde mille entrées et oublie les plus anciennes au-delà.
