# 0034 — Trois collaborateurs de la fenêtre, qui reçoivent la page qu'ils visent

Statut : acceptée — 2026-09-24
Issue [#460](https://github.com/Guyot-Bertrand/sub-edit/issues/460), ouverte par la relecture de fin de
phase 11 ([#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441)).

## Contexte

`MainWindow` pesait **1 951 lignes** à l'ouverture de la phase 11, et **2 968** à sa relecture. L'[ADR
0033](0033-un-projet-est-une-page.md) a sorti de la fenêtre ce qu'un projet **retient** — `ProjectPage` —,
pas ce qu'un projet **subit** : tout ce que la troisième tranche a appris à faire s'est ajouté à la
fenêtre, et chacune de ces méthodes lit `m_page`, la page de l'onglet courant.

Deux conséquences, et la seconde est la plus coûteuse :

- une seule classe de 94 méthodes, où ce qui ouvre un fichier voisine avec ce qui déplace une colonne ;
- **rien de tout cela ne se teste sans construire la fenêtre entière** — menus, lecteur, barre d'état —,
  parce que rien ne peut viser un projet autrement qu'en en faisant l'onglet courant.

## Décision

**Trois familles sortent de la fenêtre, chacune dans un type à elle.** Le critère de la coupe : une famille
tient un état propre, et ne parle au reste de la fenêtre que par un petit nombre de gestes nommés.

| Type | Ce qu'il porte | Ce qu'il retient |
| :--- | :------------- | :--------------- |
| `ProjectFiles` | ouvrir, enregistrer, enregistrer sous, la question des modifications non enregistrées sur un ou plusieurs projets, `Save All`, lire une traduction, trier un dépôt | le répertoire du dernier fichier ouvert ou enregistré |
| `ProjectSearch` | le dialogue de recherche, la cible, chercher et remplacer dans un projet ou dans tous | les deux options, et le dialogue une fois ouvert |
| `TableColumns` | les entrées de `View ▸ Columns`, la colonne de traduction, masquer, l'ordre, les largeurs, le texte que vise la cellule courante | les largeurs des colonnes masquées |

**Chacun reçoit la page qu'il vise** — un `ProjectPage&`, ou la liste des pages —, et ne lit jamais la
page courante. C'est ce qui les rend testables sans fenêtre : une page se construit seule
(`ProjectPage::make`), et un test en passe une.

**Ce qu'ils demandent à la fenêtre passe par une interface par collaborateur**, et seulement ce qu'ils
ne peuvent pas faire eux-mêmes parce que cela appartient à l'écran :

- `ProjectFiles::View` — amener l'onglet d'une page au premier plan (un dialogue doit s'ouvrir sur le
  projet qu'il concerne), dire qu'un document vient d'être écrit (titre, film, actions), et le widget
  sur lequel poser les boîtes ;
- `ProjectSearch::View` — les pages dans l'ordre des onglets et celle qu'on montre, en amener une au
  premier plan, le texte visé et la cible que dit la sélection, se rendre à une correspondance,
  appliquer une commande à une page ;
- `TableColumns` n'en a pas : il ne connaît que la table.

La fenêtre les implémente ; un test en passe un double.

**Ce qui reste à la fenêtre** : les onglets et la liste des pages, les menus et leurs actions, le
lecteur, la barre d'état, le titre, et les opérations de `Tools` et d'`Edit`, qui passent toutes par
`applyOperation` et ne tiennent pas d'état à elles.

> **Complété le 2026-09-25, [#483](https://github.com/Guyot-Bertrand/sub-edit/issues/483) : les actions
> sortent à leur tour**, dans `WindowActions`. Il construit les trente-neuf actions, leurs raccourcis et
> leur état de départ, et les pose dans les menus et la barre d'outils ; **il ne connecte rien** — ce
> que fait chaque geste reste un slot de la fenêtre, qui le branche — et ne décide pas quand une action
> s'éteint, ce que `refreshActions` recalcule après chaque opération. La frontière n'est donc pas celle
> des trois collaborateurs ci-dessus : pas d'état propre, pas de page visée, pas d'interface `View`. Ce
> qu'il porte est ce qui ne change plus une fois construit, et c'est ce qui se teste sans fenêtre — sur
> une `QMainWindow` nue. Un agrégat de pointeurs lus par leur nom (`m_actions->undo`) plutôt qu'une
> classe à accesseurs : les accesseurs de test de la fenêtre existent déjà et renvoient ces pointeurs,
> aucun test de la fenêtre n'a été réécrit. Le constructeur de la fenêtre passe de 548 à 269 lignes,
> `main_window.cpp` de 2 531 à 2 216, `main_window.hpp` de 908 à 872.

## Alternatives écartées

- **Une interface unique pour les trois**, une « fenêtre vue d'un collaborateur ». Plus courte à écrire,
  elle aurait donné à chacun tous les gestes des deux autres : la recherche aurait pu amener un
  document au premier plan pour l'enregistrer. La frontière se lit dans ce qu'une interface ne permet
  pas.
- **Des signaux Qt plutôt que des interfaces.** Ils disent ce qui s'est passé, pas ce qu'il faut qu'il se
  passe : `mayDiscard` doit savoir qu'un onglet est au premier plan **avant** d'ouvrir `Save As…`, et un
  signal ne répond pas.
- **Découper par menu** — `File`, `Edit`, `Tools`. Les menus rangent pour l'utilisateur, pas pour le code :
  `Find and Replace…` est sous `Edit` et n'a rien de commun avec `Cut`, `Columns` est sous `View` et tient
  la même table que `targetDocument`.
- **Sortir aussi les opérations de `Tools`.** Elles sont déjà minces — un dialogue, une fonction du noyau,
  `applyOperation` —, et n'ont pas d'état : les déplacer ne rendrait rien de plus testable.

## Conséquences

- **Les collaborateurs ont leurs tests, sans fenêtre** ; les tests de la fenêtre ne changent pas, et c'est
  ce qui prouve que la coupe n'a rien déplacé du comportement.
- **Viser une page sans en faire l'onglet courant devient possible.** Cette ADR ne le fait pas : pour ne rien
  changer, les gestes sur plusieurs projets demandent encore à la fenêtre d'amener chaque page au premier
  plan. [#461](https://github.com/Guyot-Bertrand/sub-edit/issues/461) retirera ces passages, sauf devant
  un dialogue qui doit montrer son projet.

  > **Fait en #461.** `View::apply` reçoit la page, `View::show` ne sert plus qu'avant un `Save As…`, la
  > question d'un document unique et l'affichage d'une correspondance trouvée dans un autre projet.
- Une interface de plus par collaborateur, à tenir à jour ; c'est le prix d'une frontière qu'on peut
  lire.
