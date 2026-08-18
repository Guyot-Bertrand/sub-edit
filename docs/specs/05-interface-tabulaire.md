# Phase 5 — Interface : édition tabulaire

Cadrée le 2026-08-17, issue [#125](https://github.com/Guyot-Bertrand/sub-edit/issues/125).

## Objectif

Le cœur de l'usage : **ouvrir un fichier, l'éditer dans une table, l'enregistrer,
annuler.** C'est la première phase dont le produit est une fenêtre, et la
première où un utilisateur voit le projet autrement que par une sortie de
terminal.

Les quatre phases précédentes ont construit un noyau et une ligne de commande.
Celle-ci n'ajoute presque aucune capacité au noyau : elle en expose ce qui
existe. Trois commandes — `SetStart`, `SetEnd` et `Insert` — n'ont d'ailleurs
**aucune preuve de bout en bout** à ce jour, et deux autres — `SetText` et
`Remove` — ne sont atteintes qu'indirectement, par le retrait des mentions qui
les compose. Aucune sous-commande ne les expose. La fenêtre est la première
surface qui les appelle pour elles-mêmes, et elle n'en couvre que deux :
`SetStart` et `SetEnd`, `Insert` restant à la phase 7.

## Portée

**Ce que la phase construit :**

- une fenêtre, un projet à la fois, une table à cinq colonnes — numéro, début,
  fin, durée, texte ;
- l'édition en place du texte, du début et de la fin ;
- ouvrir, enregistrer, enregistrer sous ;
- annuler et rétablir, branchés sur l'historique du noyau ;
- quatre opérations par dialogue — décaler, transformer, convertir la fréquence
  d'image, retirer les mentions pour malentendants ;
- le marquage des sous-titres en désordre.

**Ce qui n'y est pas, et où ça va :**

| Écarté | Où | Pourquoi |
| :----- | :- | :------- |
| insertion et suppression de sous-titres depuis la fenêtre | phase 7 | l'édition des cellules est le cœur de l'usage ; ajouter une ligne ne l'est pas |
| configuration persistée, préférences | phase 7 | la feuille de route l'y place déjà, et rien de persistant n'est requis pour que la table fonctionne |
| multi-projets en onglets | phase 11 | hors contour du MVP |
| colonne de traduction | phase 11 | le modèle l'accueille, l'interface ne la construit pas |
| colonnes configurables, coloration des différences | non planifié | hors contour du MVP |
| mode d'édition en images | phase 9 | aucun format à images avant MicroDVD |
| édition de la durée | phase 10 | le noyau n'a pas de `SetDurationCommand`, et la phase 2 s'est arrêtée à « un texte, un début, une fin » |

**Trois travaux de noyau précèdent la fenêtre**, et ils sont dans la phase parce
qu'elle les déclenche : la réorganisation de `format/` et `model/`, la scission
des diagnostics, et la représentation compacte des sélections et des changements
([#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45)).

## Décisions applicables

| ADR | Ce qu'elle fixe |
| :-- | :-------------- |
| [0001](../adr/0001-cpp20-et-qt6.md) | Qt 6, et un cœur sans dépendance à l'interface |
| [0010](../adr/0010-annulation-par-commandes.md) | des commandes portant leur propre inverse |
| [0012](../adr/0012-ordre-des-sous-titres-par-composition.md) | le modèle ne trie pas de lui-même |
| [0014](../adr/0014-registre-d-exigences.md) | des exigences citées par un tag de test |
| [0018](../adr/0018-vocabulaire-des-formats-dans-le-modele.md) | le vocabulaire des formats au modèle, les opérations à `format/` |
| [0019](../adr/0019-table-en-adaptateur-mince.md) | une table qui lit à travers le modèle, un noyau qui rend ses changements |

## Ce que Gaupol fait, et où nous divergeons

L'analyse a porté sur `gaupol/view.py`, `page.py`, `application.py`,
`renderers/multiline.py`, `config.py` et les dialogues d'opération.

**Il duplique intégralement la table.** `page.py` recopie chaque sous-titre dans
un `Gtk.ListStore` et le tient à jour par huit gestionnaires de signaux. Le prix
est écrit dans son code : `reload_view_all()` reconstruit tout à l'ouverture,
`_on_project_subtitles_removed` débranche le modèle de la vue au-delà de
cinquante lignes retirées — « une grande série de mises à jour vives faites
directement à la vue est lente » —, et un `iterate_main()` traîne après chaque
rafraîchissement. Nous prenons l'adaptateur mince, et [0019](../adr/0019-table-en-adaptateur-mince.md)
dit pourquoi.

**Il ne stocke pas le numéro** : une fonction de cellule le calcule depuis
l'indice de ligne. Nous faisons pareil — le numéro est un rang, pas une donnée.

**Il n'a aucune migration de configuration, et c'est un dispositif.** Le fichier
est lu option par option ; une clé inconnue est acceptée puis effacée à
l'écriture si elle n'est plus dans les défauts ; une valeur illisible imprime sur
la sortie d'erreur et laisse le défaut ; toute option restée à sa valeur par
défaut est réécrite **commentée**, si bien qu'un changement de défaut prend effet
chez qui ne l'a jamais surchargé. `general.version` est écrit et n'est jamais
relu : une trace, pas un déclencheur. Le constat est consigné ici pour la
phase 7, qui en hérite.

**Sa fréquence d'entrée vient d'une préférence.**
`FramerateConvertDialog._init_values()` pose les deux listes sur
`page.project.framerate`, qui vient de `conf.editor.framerate` — 23,976 par
défaut. Aucune heuristique de nom de fichier, aucune lecture de vidéo, en vingt
ans. Voir « L'origine de la fréquence » plus bas.

**Son édition multiligne lui a coûté un widget.** `MultilineCellRenderer` fabrique
un `CellTextView`, une `Gtk.TextView` qui implémente `Gtk.CellEditable`, avec
`Entrée` et `Échap` gérés à la main et un contournement à la perte de focus
— « supprimer l'éditeur là envoie la gestion du focus de GTK dans une boucle
infinie ». Qt offre le même geste par un chemin documenté : un
`QStyledItemDelegate` qui rend un `QPlainTextEdit`.

## Le noyau avant la fenêtre

### 1. Le vocabulaire des formats rejoint le modèle

`SourceFile` retient tout du fichier d'origine sauf son format. Une fenêtre qui
ouvre puis enregistre en a besoin, et l'y ajouter créerait la première arête
`model/ → format/`.

[0018](../adr/0018-vocabulaire-des-formats-dans-le-modele.md) tranche : `format/`
ne garde que les opérations, le vocabulaire va au modèle, et ce qui ignore
jusqu'au mot « sous-titre » sort des deux — `io/` pour le disque, `text/` pour
les chaînes. `SourceFile` porte alors son format sans arête nouvelle.

Le déplacement est **mécanique et sans changement de comportement** : 38 fichiers
dont les inclusions changent. Il fait retomber `tidy-scope.sh` sur l'analyse
complète, donc une porte à plein tarif, payée une fois.

### 2. Les diagnostics se scindent

Trois catégories de `DiagnosticKind` ne décrivent pas ce qu'une lecture a
rencontré mais ce qu'un document est. Elles se séparent, et le repère dit
pourquoi : **une ligne n'existe qu'au moment de la lecture, un indice survit à
l'édition.**

| Reste dans `format/diagnostic.hpp`, repéré par une ligne | Passe dans `model/anomaly.hpp`, repéré par un `SubtitleIndex` |
| :--- | :--- |
| `IgnoredLine`, `MalformedTimestamp`, `MissingNumbering`, `InconsistentNumbering`, `TextBeforeAnyTimestamp`, `UnknownBlock`, `MixedNewlines` | `EndBeforeStart`, `OverlappingSubtitles`, `OutOfOrder` |

Le modèle expose `scanAnomalies(const Project&)`, que lisent **`inspect` et la
fenêtre**. `Project::outOfOrder()` perd son paramètre et devient la lecture
retenue, dont `scanAnomalies` se sert.

**Ce que ça change pour l'utilisateur :** `inspect` désigne un numéro de
sous-titre là où il donnait un numéro de ligne. Pour qui ouvre le fichier dans un
éditeur de texte, c'est un repère de moins bonne qualité ; c'est accepté parce
que l'indice est le seul qui survive à une édition, et qu'aucune anomalie ne
serait autrement calculable après une modification.

**La lecture du désordre est tranchée : `Breaks`.** Sur les départs
`0, 4000, 2000, 3000`, elle nomme `{2}` quand l'autre nomme `{2, 3}` — or la
ligne 3 est à sa place vis-à-vis de la 2 : la signaler désigne une ligne qu'il
n'y a rien à corriger. Une table qui surligne les fautives doit surligner celles
qu'on déplace.

**Le corpus n'a pas départagé, et il faut le dire.** La phase 3 avait exposé les
deux lectures sous `--order-report` pour « les comparer sur des fichiers réels ».
Les deux lectures ont été lancées sur les quinze fichiers de `src/data/` :
**aucun n'est en désordre**, elles s'accordent trivialement partout, et huit ne
sont pas de l'UTF-8 valide. La décision est donc prise par raisonnement, faute de
données, et non par mesure. `--order-report` disparaît.

### 3. Le coût d'une opération portant sur tout un fichier

[#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) passe **avant** la
table, et cet ordre change ce qu'elle doit produire.

Le modèle de table traduit un `Change` en `dataChanged(topLeft, bottomRight)` :
**Qt veut des plages, pas des indices.** Une table écrite avant #45 déballerait
quatre mille indices pour les recoller en intervalles ; écrite après, elle
consomme la forme compacte telle quelle. La représentation que #45 introduit doit
donc **exposer des intervalles utilisables directement**, et pas seulement
occuper moins de mémoire.

Deux notes sur la mesure et la portée :

- elle se prendra sur les benchmarks du noyau et non sur la fenêtre, qui n'existe
  pas encore ; l'empreinte de l'historique est déjà chiffrée — 32 Ko par
  décalage, 64 Ko par transformation — et c'est le point de comparaison ;
- son point 2, le retrait quadratique, garde sa raison d'être malgré l'absence
  de menu « supprimer » : `removeHearingImpaired` produit un `RemoveCommand` que
  l'annulation ré-insère.

## L'architecture de la fenêtre

### Où vit le code

`src/lib/subedit/gui/`, bibliothèque `subedit::gui`, sur le modèle de
`subedit::cli`. `src/exe/gui/main.cpp` reste du câblage.

Ce n'est pas un choix de style : `check-architecture.sh` refuse un `main` qui
définit une classe ou dépasse sa limite de lignes, et refuse toute inclusion de
Qt sous `core/`. La frontière est tenue par la porte.

### Le modèle de table

`SubtitleTableModel : QAbstractTableModel`, adaptateur mince sur `const Project&`.

| Colonne | Contenu | Éditable |
| :------ | :------ | :------- |
| numéro | `row + 1`, jamais stocké | non |
| début | `Timestamp::format()` | oui |
| fin | `Timestamp::format()` | oui |
| durée | `fin − début`, formatée | non |
| texte | le texte du document principal | oui |

`data()` va chercher `project.subtitleAt(index)` et formate à la volée. Aucun
sous-titre n'est recopié, et le modèle ne matérialise jamais ce qui n'est pas
visible.

La durée n'est pas éditable : le noyau n'a pas de commande pour ça, et en
inventer une ici serait du travail de noyau glissé dans une phase d'interface.

Le séparateur décimal affiché suit le format du projet — virgule pour SubRip,
point pour WebVTT — pour que ce qu'on lit à l'écran soit ce qui sera écrit.

### Ce que la fenêtre apprend d'un changement

`Session::apply`, `undo` et `redo` rendent `std::vector<Change>` au lieu de
`void`. L'annulation le rend inversé, par une fonction libre `invert(ChangeKind)`
— les indices sont les mêmes dans les deux sens.

Le modèle traduit :

| Ce que le `Change` dit | Ce que le modèle émet |
| :--------------------- | :-------------------- |
| `Positions` | `dataChanged` sur les colonnes début, fin, durée |
| `MainText` | `dataChanged` sur la colonne texte |
| `Reordering` | `dataChanged` sur toute la largeur des plages |
| `Insertion`, `Removal` | `beginResetModel` / `endResetModel` |

La réinitialisation est le prix de l'ordre des choses : Qt exige d'encadrer un
changement de structure **avant** qu'il ait lieu, `Session` ne le rapporte
qu'après, et aucune commande ne peut le prédire — `removeHearingImpaired` ne sait
quels sous-titres la règle vide qu'une fois la règle appliquée.
[0019](../adr/0019-table-en-adaptateur-mince.md) porte la décision et son
déclencheur de réexamen.

### L'annulation

Pas de `QUndoStack`. `canUndo()` et `canRedo()` pilotent deux `QAction` ; le
libellé « Annuler : … » vient de `CommandKind` traduit par une fonction libre de
`subedit::gui` — l'énumération existe pour ça, son commentaire le dit.

**La question de groupement laissée ouverte en phase 2 se referme sans
mécanisme.** Un `QStyledItemDelegate` valide une fois, à `Entrée` ou à la perte
du focus : une cellule éditée produit **une** commande. Il n'y a rien à grouper,
donc rien à construire.

### L'édition en place

Trois délégués :

- **texte** — `QPlainTextEdit`, multiligne, `Entrée` valide, `Maj+Entrée` insère
  un saut de ligne, `Échap` annule ;
- **début** et **fin** — champ de saisie contraint, lu par `Timestamp::parse`,
  qui est déjà permissif comme les fichiers réels l'exigent ; une saisie illisible
  laisse la cellule inchangée.

Une validation qui ne change rien ne produit aucune commande : l'historique n'a
pas à retenir une frappe suivie d'un `Entrée` sur un texte identique.

## La surface visible

### Ouvrir

`subedit-gui <fichier>` d'abord — la fenêtre est utile et testable dès qu'elle
prend un chemin en argument —, le dialogue d'ouverture ensuite.

La lecture passe par `readSubtitles`. Un `ReadError` donne un message et rien
d'ouvert. **Les diagnostics de lecture sont montrés** dans un panneau repliable :
c'est le premier endroit du projet où ils atteignent un utilisateur autrement que
par `-vvv`.

### Enregistrer, enregistrer sous

Enregistrer réécrit dans le format, les fins de ligne, le BOM et l'en-tête
d'origine — `SourceFile` les porte tous, et portera le format après la
réorganisation. L'écriture passe par l'écriture atomique existante, et
`Session::markSaved` remet le compteur de modifications à zéro.

« Enregistrer sous » ajoute le choix du chemin et du format. Un changement de
format laisse en place des `FormatExtras` de l'autre variante : **l'issue devra
regarder ce que les écrivains en font**, plutôt que le supposer.

Fermer ou ouvrir un autre fichier alors que `hasUnsavedChanges` est vrai demande
confirmation.

### Les opérations

Quatre dialogues, et une cible commune : **la sélection, ou tout le fichier**.
C'est ce que la phase 4 avait renvoyé ici — « phase 5, qui apporte une vraie
sélection ».

| Dialogue | Ce qu'il demande |
| :------- | :--------------- |
| décaler | une durée signée |
| transformer | deux repères, indice et position corrigée |
| convertir la fréquence | fréquence d'entrée, fréquence de sortie |
| retirer les mentions | rien qu'une confirmation, et un compte rendu |

Le retrait des mentions demande **une signature de plus au noyau** :
`removeHearingImpaired` parcourt aujourd'hui tout le projet en dur, et doit
accepter une `Selection`.

Une opération qui ne change rien — `removeHearingImpaired` rend `nullptr` dans ce
cas — n'entre pas dans l'historique et le dit.

### L'origine de la fréquence d'entrée

**Question close, et sans heuristique.** Le dialogue de conversion demande la
fréquence d'entrée, pré-remplie par celle du projet.

Le fichier ne la porte pas et ne peut pas la porter : SubRip n'a pas d'en-tête,
celui de WebVTT est du texte libre, et les deux formats du MVP sont temporels.
Une heuristique tirée du nom de fichier a été **écartée faute de données** — les
quinze fichiers de `src/data/` ont été renommés à la main, leurs noms sont le
produit d'un nettoyage et non une observation, et aucun fichier aux noms intacts
n'est disponible. Se tromper de fréquence décale tout le fichier sans rien
signaler : silencieux et global, le pire mode d'échec de cette opération.

Gaupol ne fait pas mieux et n'a jamais fait mieux ; l'iso-fonctionnalité est
atteinte.

**Deux pistes existent, et aucune n'est de cette phase.** Le conteneur vidéo
*déclare* sa fréquence, et la phase 6 associe déjà une vidéo au projet. Et les
positions elles-mêmes la *trahissent* : convertir une fréquence suppose qu'elles
étaient calées sur une grille d'images, et cette grille se mesure — huit des
quinze fichiers du corpus privé la donnent sans ambiguïté, contre un bruit de
fond de quelques pour cent. C'est l'objet de la **phase 16**, née de ce
cadrage.

**La phase 5 ne dépend d'aucune des deux.** Son dialogue demande la fréquence et
la pré-remplit par celle du projet ; le jour où une mesure existe, elle
remplacera ce pré-remplissage sans que le dialogue change de forme. Écrire
l'inverse — attendre la mesure pour construire le dialogue — ferait dépendre le
MVP d'une capacité qui n'existe nulle part ailleurs.

Aucune action dédiée ne déclare la fréquence : avec deux formats temporels, elle
ne sert qu'à la conversion, et c'est la liste d'entrée du dialogue qui la
déclare. Gaupol a un menu parce que Gaupol lit des formats à images ; nous n'en
avons aucun avant la phase 9.

**Le `ChangeKind` de la fréquence reste écarté.** La phase 2 avait posé la
condition — « si la fenêtre affiche la fréquence courante, elle aura besoin de
savoir qu'elle a changé ». Elle ne l'affiche pas. L'énumérateur n'aurait toujours
aucun lecteur, et la règle que `DiagnosticKind` énonce tient : un énumérateur que
personne ne lit est une promesse sans garant.

### Le désordre à l'écran

Les sous-titres que `scanAnomalies` signale sont marqués dans la table — fond
teinté sur les colonnes de position, et une infobulle qui nomme l'anomalie. Le
marquage se recalcule après chaque changement de positions.

### Ce que la fenêtre n'a pas

Nommé pour que personne ne le cherche : pas de barre de recherche, pas de
presse-papiers, pas d'aperçu vidéo, pas de préférences, pas d'onglets, pas de
colonne de traduction, pas de menu d'insertion.

## Tests

Le harnais est celui de [#119](https://github.com/Guyot-Bertrand/sub-edit/issues/119) :
**Catch2, et de Qt seulement ses fonctions de pilotage.** Le binaire fournit son
`main`, la `QApplication` vit sur sa pile, et `QT_QPA_PLATFORM=offscreen` est
posé par défaut.

| Niveau | Ce qui est éprouvé |
| :----- | :----------------- |
| unitaire, sans widget | la traduction d'un `Change` en signaux du modèle ; `invert` ; le libellé d'un `CommandKind` ; `scanAnomalies` |
| modèle de table | `rowCount`, `columnCount`, `data` sur chaque colonne, `setData` produisant une commande et une seule |
| fenêtre, piloté par `QTest` | ouvrir un fichier, éditer une cellule, annuler, rétablir, enregistrer, chaque dialogue d'opération |

**Ce qu'un test d'interface ne doit pas devenir :** une capture de pixels. On
assert sur ce que le modèle rend et sur ce que la session contient, jamais sur
un rendu.

Le corpus de fichiers de `src/test/data/` sert à l'ouverture ; **`src/data/` n'est
lu par aucun test**, la règle du projet ne changeant pas ici.

## Mesures

`make bench` tourne à chaque issue, comme partout. Deux mesures nouvelles
arrivent avec le code qui les rend possibles :

- **#45** — empreinte de l'historique et coût d'un retrait, avant et après, en
  `Release`. Point de comparaison : 270 o par édition de texte, 32 Ko par
  décalage, 64 Ko par transformation.
- **le modèle de table** — construire un modèle sur un projet de quatre mille
  sous-titres, et mesurer `data()` sur une fenêtre de lignes visibles. C'est la
  mesure qui dit si l'adaptateur mince tient sa promesse.

Il n'y a pas de benchmark d'interface au sens d'une fenêtre pilotée : #116 l'a
écarté, et sa raison tient toujours — la mesure vient avec le code qu'elle
mesure.

## Exigences

Inscrites à l'état `prévue`, comme la règle l'exige, avant le code.

| ID | Exigence |
| :- | :------- |
| `GUI-OPEN-01` | `subedit-gui <fichier>` ouvre le fichier et affiche ses sous-titres |
| `GUI-OPEN-02` | un fichier illisible donne un message et laisse la fenêtre vide |
| `GUI-OPEN-03` | les diagnostics de lecture sont montrés, avec leur ligne |
| `GUI-TABLE-01` | les cinq colonnes affichent numéro, début, fin, durée et texte |
| `GUI-TABLE-02` | les sous-titres en anomalie sont marqués et nommés |
| `GUI-EDIT-01` | éditer une cellule de texte modifie le sous-titre et rien d'autre |
| `GUI-EDIT-02` | éditer un début ou une fin lit un horodatage permissif |
| `GUI-EDIT-03` | une validation qui ne change rien n'entre pas dans l'historique |
| `GUI-UNDO-01` | annuler rétablit l'état précédent, l'action nomme l'opération |
| `GUI-UNDO-02` | rétablir refait ce qui vient d'être annulé |
| `GUI-SAVE-01` | enregistrer réécrit le fichier dans sa forme d'origine |
| `GUI-SAVE-02` | enregistrer sous choisit chemin et format |
| `GUI-SAVE-03` | fermer avec des modifications non enregistrées demande confirmation |
| `GUI-SHIFT-01` | le dialogue de décalage décale la cible |
| `GUI-TRANSFORM-01` | le dialogue de transformation corrige par deux repères |
| `GUI-FRAMERATE-01` | le dialogue de conversion re-cale la cible |
| `GUI-HEARING-01` | le retrait des mentions s'applique à la sélection ou au fichier |
| `GUI-HEARING-02` | un retrait qui ne change rien le dit et n'entre pas dans l'historique |

Deux lignes du registre bougent par ailleurs :

- `CLI-INSPECT-03` passe à `abandonnée` — `--order-report` disparaît ;
- une exigence nouvelle, `CLI-INSPECT-04`, dit que les anomalies d'un document
  sont rapportées par numéro de sous-titre.

## Manuel

`docs/manual/subedit-gui/`, **en prose**. #116 a écarté captures engendrées et
description engendrée de l'arbre de widgets, avec leurs raisons ; `make manual`
ne tiendra pas ce manuel, et sa justesse repose sur la relecture de fin de phase.
C'est écrit ici pour que personne ne croie le contraire.

Chaque issue met à jour la section qu'elle change, à l'étape 7 de l'ordre du
projet — après la porte, qui ne lit pas le manuel.

## Découpage en issues

| Ordre | Issue | Dépend de |
| :---: | :---- | :-------- |
| 1 | [#126](https://github.com/Guyot-Bertrand/sub-edit/issues/126) — ranger le vocabulaire des formats, les opérations, et le reste | — |
| 2 | [#127](https://github.com/Guyot-Bertrand/sub-edit/issues/127) — scinder les diagnostics de lecture et les anomalies d'un document | #126 |
| 3 | [#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) — coût d'une opération portant sur tout un fichier | #126 |
| 4 | [#128](https://github.com/Guyot-Bertrand/sub-edit/issues/128) — la fenêtre et son modèle de table | #127, #45 |
| 5 | [#129](https://github.com/Guyot-Bertrand/sub-edit/issues/129) — éditer une cellule : texte multiligne, début, fin | #128 |
| 6 | [#130](https://github.com/Guyot-Bertrand/sub-edit/issues/130) — annuler, rétablir, et le libellé de l'action | #129 |
| 7 | [#131](https://github.com/Guyot-Bertrand/sub-edit/issues/131) — ouvrir, enregistrer, enregistrer sous | #130 |
| 8 | [#132](https://github.com/Guyot-Bertrand/sub-edit/issues/132) — décaler, transformer, convertir la fréquence | #131 |
| 9 | [#133](https://github.com/Guyot-Bertrand/sub-edit/issues/133) — retirer les mentions, sur une sélection | #132 |
| 10 | [#134](https://github.com/Guyot-Bertrand/sub-edit/issues/134) — marquer le désordre, retirer `--order-report` | #128, #127 |
| 11 | [#135](https://github.com/Guyot-Bertrand/sub-edit/issues/135) — relecture de fin de phase 5 | tout |

L'ordre 5 → 6 → 7 mérite un mot : l'annulation vient **après** l'édition parce
qu'il faut quelque chose à annuler, et l'ouverture **après** l'annulation parce
qu'un chemin en argument suffit jusque-là. Chaque issue laisse un binaire qui
sert à quelque chose.

## Renvois

Tout « plus tard » de cette spec atterrit sur une phase nommée.

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| lire la fréquence d'image dans le conteneur vidéo | phase 6 |
| déduire la fréquence d'image des positions, et corriger | phase 16 |
| signaler les positions qui sortent de la grille d'images | phase 16, qui dit pourquoi pas ici |
| insertion et suppression depuis la fenêtre | phase 7 |
| configuration persistée, préférences, thème | phase 7 |
| édition de la durée | phase 10 |
| mode d'édition en images | phase 9 |
| multi-projets, colonne de traduction | phase 11 |
| un `Session` qui annonce un changement de structure avant de le faire | phase 7, si la mesure le demande |

## Critères de fin

- [ ] La fenêtre ouvre, édite, enregistre, annule et rétablit
- [ ] Les quatre opérations sont accessibles, sur sélection ou fichier entier
- [ ] Le noyau est réorganisé, les diagnostics scindés, #45 fait et mesuré
- [ ] `--order-report` a disparu, du binaire, du manuel et du registre
- [ ] Chaque exigence `GUI-…` est `implémentée` et citée par un test
- [ ] `docs/manual/subedit-gui/` décrit ce que la fenêtre fait, et rien de plus
- [ ] Les benchmarks sont rejoués et les mesures relevées
- [ ] La relecture de fin de phase a eu lieu
