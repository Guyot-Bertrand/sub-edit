# Annuler et rétablir

Chaque opération entre dans un historique, et se défait. Une cellule éditée
compte pour **une** entrée : la frappe ne s'y inscrit pas touche par touche,
seule la validation compte.

| Action | Raccourci | Où  |
| :----- | :-------- | :-- |
| `Undo` | `Ctrl+Z` | menu **Edit**, barre d'outils |
| `Redo` | `Ctrl+Maj+Z` | menu **Edit**, barre d'outils |

## L'action nomme ce qu'elle défera

Le menu ne dit pas « Undo » mais **« Undo: editing a text »**, du nom de
l'opération qui sera défaite. Le bouton de la barre d'outils garde le mot
seul — sa largeur suivrait sinon la dernière opération, et il bougerait sous le
pointeur — mais son infobulle porte le libellé entier.

| Opération | Ce que l'action lit |
| :-------- | :------------------ |
| une cellule de texte éditée | `editing a text` |
| un début, une fin | `editing a start`, `editing an end` |
| une insertion de lignes | `inserting` |
| une suppression de lignes | `removing` |
| un décalage, `Shift onto Grid` compris | `shifting` |
| une transformation | `transforming` |
| une conversion de fréquence | `converting the frame rate` |
| un alignement sur une cadence | `aligning on the frame rate` |
| un retrait des mentions | `removing hearing-impaired mentions` |

**Dix libellés, et c'est tout ce que la fenêtre sait produire.** Le noyau en
nomme un onzième — `sorting` — qu'aucune action de la fenêtre n'atteint : le tri
n'a lieu que sous une politique d'ordre stricte, alors que la fenêtre ouvre ses
documents sous la politique souple, qui signale le désordre au lieu de le
réparer.

**`Shift onto Grid` ne se distingue pas d'un décalage**, et c'est exact : elle
en est un, dont le montant a été mesuré plutôt que saisi.

Ce sont les mêmes mots que ceux de la ligne de commande : ils sont écrits une
fois, dans le noyau.

Quand il n'y a rien à défaire, l'action lit « Undo » tout court **et elle est
inactive** : son raccourci ne fait rien.

## La marque de modification

Le titre de la fenêtre porte une marque tant que le fichier diffère de celui du
disque — une astérisque, ou ce que la plateforme utilise à sa place.

**Elle disparaît si on annule jusqu'au point de départ.** Ce n'est pas un
détail d'affichage : le nombre de modifications est compté, pas noté par un
oui-ou-non, donc revenir en arrière peut le ramener à zéro et la marque s'en va.

## Deux choses à savoir

**Le point de référence est le dernier enregistrement**, ou l'ouverture du
fichier s'il n'y en a pas eu. Enregistrer le déplace : la marque disparaît, et
annuler ensuite la fait réapparaître.

Une opération refaite après une annulation **efface ce qu'il y avait à
rétablir** : rétablir rejouerait une commande dont l'état de départ n'existe
plus. C'est le comportement de tous les éditeurs, et il est délibéré.

L'historique garde mille entrées et oublie les plus anciennes au-delà.
