# 0035 — Les opérations de `Tools` sortent de la fenêtre

Statut : acceptée — 2026-09-25
Issue [#486](https://github.com/Guyot-Bertrand/sub-edit/issues/486), sortie de l'analyse de `MainWindow`
demandée avant la clôture de la phase 11. **Revient sur une alternative que l'[ADR
0034](0034-trois-collaborateurs-de-la-fenetre.md) avait écartée.**

## Contexte

L'ADR 0034 a laissé les opérations de `Tools` dans la fenêtre, avec cette raison : « elles sont déjà
minces — un dialogue, une fonction du noyau, `applyOperation` —, et n'ont pas d'état : les déplacer ne
rendrait rien de plus testable ». Deux choses ont changé depuis.

- **#461 a rendu la page explicite.** `applyOperation` reçoit la page visée et ne lit plus `m_page` ; la
  cible d'une opération se lit dans la sélection **de la page** (`ProjectPage::tableSelection`), qui est
  celle de la table quand la page est affichée. Une opération peut donc s'exécuter sur une page construite
  seule, ce qui était faux quand 0034 a été écrite.
- **Le poids.** Après #483, #484 et #485, les treize opérations de `Tools` et la route vers l'historique
  pesaient encore **environ 350 lignes** d'un `main_window.cpp` de 1 851 — la plus grosse famille restante.
  L'utilisateur a rouvert la question en retenant la piste à l'analyse.

La raison de 0034 était juste sur un point : ces opérations ne tiennent presque pas d'état. Elle ne l'était
plus sur l'autre : sans fenêtre, elles ne se testaient pas du tout.

## Décision

**Un collaborateur `ProjectOperations`**, sur le modèle de 0034, porte :

- les treize opérations de `Tools` — décaler, transformer, convertir, ajuster les durées, ajouter un fichier,
  scinder le projet, retirer les mentions, italique, casse, tirets, aligner, ramener sur la grille,
  analyser la grille ;
- **la route unique vers l'historique**, `apply` et `applyQuietly`, avec ce qu'une opération laisse passé la
  fin du film. Les gestes d'`Edit` et la recherche l'empruntent aussi : elle n'a qu'un chemin, et ce chemin
  est ici ;
- le seul état de la famille : la forme du dernier ajustement des durées, que les réglages lisent et
  écrivent.

**Chaque opération reçoit la page** et lit sa cible dans la sélection de cette page. **Ce qui appartient à
l'écran passe par `ProjectOperations::View`** : le parent des boîtes, le texte visé, la durée du film de la
page, le message passager de la barre d'état, le fichier à ajouter et sa lecture — qui restent à
`ProjectFiles` —, le panneau des diagnostics, la sélection de lignes, et l'ouverture d'un projet à côté
pour la queue d'une scission.

**La fenêtre garde les `connect`**, et ce qui précède un geste sans dialogue : `commitCellEditor`, qui
valide la cellule ouverte avant que l'italique, la casse ou les tirets ne lisent leur cible (#397).

## Alternatives écartées

- **Laisser `applyOperation` dans la fenêtre** et n'en sortir que les opérations. Elles l'auraient
  demandé par la `View`, qui aurait alors porté le cœur de la famille ; et la route qu'elles empruntent
  toutes n'aurait pas été testée avec elles.
- **Une classe par opération.** Treize types pour treize gestes qui partagent une route, une cible et un
  compte rendu : la frontière serait devenue le nombre de fichiers.
- **Passer la cible en argument** plutôt que la lire dans la page. C'était possible, et plus pur ; mais
  `Split Project…` a besoin de la sélection elle-même — il la montre pendant le choix et la rend si l'on
  annule (#462) —, et deux façons de trouver la cible dans une même famille en font une de trop.

## Conséquences

- **Les opérations ont leurs tests, sans fenêtre** (`project_operations_test.cpp`) ; ceux de la fenêtre ne
  changent pas.
- `main_window.cpp` passe de 1 851 à 1 506 lignes, `main_window.hpp` de 794 à 692 ; l'analyse visait
  environ 1 450.
- Une interface de plus, et la plus large des quatre — neuf gestes. Elle se lit comme la liste de ce qu'une
  opération ne peut pas faire seule, et c'est ce qui justifie chacun.
- **Ce qui reste à la fenêtre** : les onglets et la liste des pages, les gestes d'`Edit` qui manipulent la
  table (insérer, retirer, fusionner, scinder une ligne, le presse-papiers), l'ouverture des fichiers et des
  traductions, les réglages. L'ADR 0034 disait que les onglets en sont le cœur ; c'est toujours vrai.
