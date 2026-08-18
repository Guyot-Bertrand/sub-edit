# 0019 — Lire à travers le modèle du noyau, qui rend ses changements

**Date :** 2026-08-17
**Statut :** acceptée

## Contexte

La phase 5 pose un `QAbstractTableModel` au-dessus de `Project`. Deux formes
s'offrent : un **adaptateur mince**, dont `data()` va chercher la donnée dans le
projet à chaque appel, ou un **modèle propre** qui détient sa copie et se
resynchronise sur des signaux.

Gaupol a pris la seconde, et son code en montre le prix. `gaupol/page.py` recopie
chaque sous-titre dans un `Gtk.ListStore` et le tient à jour par huit
gestionnaires de signaux. On y lit :

- `reload_view_all()`, qui vide et repeuple la totalité à chaque ouverture ;
- dans `_on_project_subtitles_removed`, `if len(rows) > 50:` **débranche le
  modèle de la vue**, avec ce commentaire : « une grande série de mises à jour
  vives faites directement à la vue est lente » ;
- un `gaupol.util.iterate_main()` après chaque rafraîchissement, pour rendre la
  main à la boucle d'événements.

Ce ne sont pas des maladresses : c'est ce que coûte la duplication, payé par la
référence, écrit dans son code.

Le noyau, lui, **ne connaît aucun mécanisme de signal**, et c'est délibéré —
`Change` le dit dans son propre commentaire : « le cœur rend l'information et
l'appelant en fait ce qu'il veut ». Mais `Session::apply`, `undo` et `redo`
rendent `void`. Une fenêtre branchée dessus n'a aucun moyen de savoir quelles
lignes rafraîchir, et n'aurait que le choix de tout redessiner.

Enfin, Qt propose `QUndoStack`. Le noyau a la sienne, et c'est elle qui fait
autorité : la ligne de commande de la phase 3 en dépend aussi.

## Décision

**La table est un adaptateur mince.** `data()` lit `project.subtitleAt(index)`
et formate à la volée ; aucun sous-titre n'est recopié. La colonne du numéro
n'est pas une donnée mais `row + 1`.

**`Session::apply`, `undo` et `redo` rendent ce qu'ils ont changé** — un
`std::vector<Change>`, celui que `describe()` produit déjà. L'annulation le rend
**inversé** : défaire une insertion est une suppression, et une fonction libre
`invert(ChangeKind)` suffit, les indices étant les mêmes dans les deux sens.
Aucun observateur, aucun signal n'entre dans le noyau.

**L'historique du noyau est le seul.** `canUndo()` et `canRedo()` pilotent les
deux actions de la fenêtre ; il n'y a pas de `QUndoStack`.

**Un changement de structure passe par une réinitialisation du modèle.** Qt
exige d'encadrer une insertion ou une suppression de lignes *avant* qu'elle ait
lieu — `beginRemoveRows` — alors que `Session` ne rend son compte rendu
qu'après. Donc :

| Ce qui change | Ce que le modèle émet |
| :------------ | :-------------------- |
| texte, positions, réordonnancement | `dataChanged` sur les plages concernées |
| lignes ajoutées ou retirées | `beginResetModel` / `endResetModel` |

## Alternatives écartées

- **Un modèle propre synchronisé,** comme Gaupol. Il duplique chaque sous-titre
  et fait de la resynchronisation le lieu où vivent les défauts. Écarté sur
  pièces, et non par goût : les trois cicatrices citées plus haut sont dans
  `page.py`.

- **`QUndoStack`.** Une seconde pile d'annulation, qu'il faudrait tenir en
  accord avec la première. Deux sources de vérité pour la même question, dont
  une seule est visible depuis la ligne de commande.

- **Un observateur dans le noyau** — `Session::setObserver`. Il résoudrait
  l'encadrement des changements de structure en prévenant avant et après. Il
  ajoute au cœur un mécanisme dont la CLI n'a aucun usage, et fait entrer
  l'ordre des notifications dans son contrat. À reconsidérer si l'encadrement
  devient réellement gênant, pas avant.

- **Décrire la commande avant de l'appliquer,** pour connaître d'avance la
  structure touchée. Impossible en général : `removeHearingImpaired` ne sait
  quels sous-titres la règle vide qu'une fois la règle appliquée. Une solution
  qui marche pour certaines commandes et pas pour d'autres n'en est pas une.

## Conséquences

**Rendu facile.** Rafraîchir ce qui bouge et rien d'autre, sur plusieurs
milliers de lignes. Une seule autorité pour l'annulation, partagée avec la ligne
de commande. Un cœur qui reste libre de Qt, ce que `check-architecture.sh`
vérifie déjà.

**Rendu difficile.** Un changement de structure coûte une reconstruction de la
vue et perd la sélection. C'est tenable tant que son seul producteur est le
retrait des mentions pour malentendants — global, rare, et suivi d'un
rafraîchissement de toute façon.

**Ce que ça coûte de défaire.** L'adaptateur mince, presque rien : le modèle est
mince par définition. Le compte rendu rendu par `Session`, davantage — trois
signatures publiques que la CLI compile aussi.

**Déclencheur de réexamen.** La phase 7 apporte l'insertion et la suppression de
sous-titres depuis la fenêtre. Une réinitialisation complète à chaque ligne
ajoutée serait alors ridicule, et c'est là que `Session` devra annoncer un
changement de structure avant de le faire. Le mesurer avant de le construire.
