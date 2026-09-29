# Correct Texts…

`Tools ▸ Correct Texts…` ouvre un assistant à plusieurs pages, sur le modèle de
celui de Gaupol : une page de cible et de tâches, une page par tâche cochée,
une page de progression, et une page de confirmation qui propose les
changements sans jamais les imposer.

**Éteinte sur un document vide** : il n'y a rien à corriger.

## Tasks and Target — la première page

| Groupe | Choix | Par défaut |
| :----- | :---- | :--------- |
| `Target` | `Selection`, `Current Project`, `All Open Projects` | `Current Project` |
| `Document` | `Text`, `Translation` | `Text` |
| `Tasks` | `Mentions`, `Common Errors`, `Capitalization`, `Line Break` — à cocher, un ou plusieurs | `Common Errors` et `Capitalization` cochées, les deux autres non |

`Selection` est éteinte sans ligne choisie dans l'onglet montré ; `Translation`
est éteinte tant qu'aucun projet ouvert ne porte de traduction.

**Ce que chaque cible désigne** :

| Cible | Porte sur |
| :---- | :-------- |
| `Selection` | les lignes choisies dans l'onglet montré |
| `Current Project` | tout le fichier de l'onglet montré |
| `All Open Projects` | tout le fichier de chaque onglet ouvert |

**`All Open Projects` sur `Translation` saute les projets sans traduction** :
un projet qui n'en porte pas n'a rien à proposer dans une colonne vide, et
l'assistant ne le compte pas.

**Les pages de tâche suivent, dans l'ordre de Gaupol — `Mentions`, `Common
Errors`, `Capitalization`, `Line Break` — jamais celui dans lequel les cases
ont été cochées.** Une tâche non cochée n'a pas de page ; ses réglages
retenus ne changent pas pour autant. **Aucune page de tâche ne porte sa propre
case « active »** : c'est celle de cette première page, et elle seule, qui
décide si la tâche s'exécute.

## Les pages de tâche

Chacune des quatre pages porte la même base : trois menus déroulants en
cascade — écriture, langue, pays — pour choisir le code des motifs à
appliquer, puis la liste des motifs de ce code, un à cocher par nom. **Les
trois menus ne proposent que ce que le catalogue de motifs porte
effectivement** — ceux livrés avec le programme et ceux qu'un utilisateur y a
déposés (voir [Les motifs déposés](#les-motifs-déposés)) — jamais une liste
de langues ou de pays lue ailleurs. `Zyyy` (« toute écriture ») est le code de
départ, ce qui donne les motifs valables partout.

**Un motif décoché reste décoché tant qu'on ne le recoche pas**, y compris en
changeant de code puis en y revenant, et ce choix est retenu d'une ouverture
de l'assistant à l'autre.

| Page | Ce qu'elle ajoute à la base |
| :--- | :-------------------------- |
| `Mentions` | deux cases, `Sound in brackets` et `Sound in parentheses`, non cochées par défaut |
| `Common Errors` | deux cases, `Human` et `OCR`, cochées par défaut |
| `Capitalization` | rien de plus |
| `Line Break` | une longueur maximale (`24`, par défaut), un nombre de lignes maximal (`3`), une unité — `Characters` ou `Ems`, `Characters` par défaut |

**Les deux cases de `Mentions` ne nomment aucun motif compilé** : elles
commandent un balayage du texte au noyau, celui que
[`Remove Hearing-Impaired Mentions…`](operations.md#remove-hearing-impaired-mentions)
fait déjà d'un geste direct. L'une **ou** l'autre cochée suffit à le lancer.

**`Common Errors` filtre par classe** : un motif qui porte les deux classes
s'applique dès que l'une des deux cases est cochée ; les trois autres pages ne
connaissent pas cette distinction.

**L'unité de `Line Break` ne se retient pas** d'une session à l'autre — seules
les deux valeurs numériques le sont. Choisie en `Ems`, la longueur est mesurée
avec la police de la fenêtre plutôt qu'en nombre de caractères.

### Les motifs déposés

Un motif qu'un utilisateur a déposé dans son propre répertoire de motifs
s'ajoute à ceux livrés avec le programme, dans le même catalogue et sous les
mêmes menus — rien ne les distingue une fois chargés.

## Progress

Une fois `Next` pressé depuis la dernière page de tâche cochée (ou depuis la
première page si aucune tâche ne l'est), l'assistant calcule les corrections
proposées sur un fil d'arrière-plan, une barre de progression indéterminée à
l'écran. Le calcul fini, la page de confirmation s'ouvre d'elle-même.

**`Cancel` à cette étape est le seul geste d'abandon** : rien n'interrompt le
calcul en cours, il n'est simplement jamais lu. Un calcul qui se termine juste
après une annulation peut donc, dans une fenêtre de temps étroite, lire un
projet que l'annulation vient de fermer — un risque accepté, jamais rencontré
autrement qu'en théorie.

## Confirmation — la dernière page

![La page de confirmation : trois lignes, une acceptée, une supprimée, une
refusée.](captures/correction.png)

![La même page sous la palette sombre.](captures/correction-sombre.png)

Une ligne par texte que l'assistant changerait — jamais une ligne pour un
texte qu'une tâche a laissé tel quel.

| Colonne | Ce qu'elle montre |
| :------ | :----------------- |
| `Accept` | une case, cochée par défaut : la correction sera appliquée ou non |
| `Original` | le texte tel qu'il est, sa partie qui changerait **en gras** |
| `Proposed` | le texte proposé, sa partie changée en gras, **éditable** — le retoucher change ce qui sera écrit, sans changer la case `Accept` |

**Le changement est marqué en gras, jamais en couleur** : la même diff se lit
dans les deux palettes sans dépendre d'aucune teinte.

**Une ligne dont la proposition est une suppression** — le sous-titre n'aurait
plus de texte — montre la colonne `Proposed` vide, et son `Original` entier en
gras.

| Bouton | Ce qu'il fait |
| :----- | :------------- |
| `Mark All` | coche `Accept` sur toutes les lignes |
| `Unmark All` | décoche `Accept` sur toutes les lignes |
| `Preview` | place le lecteur intégré sur le sous-titre de la ligne choisie, si le projet a un film — rien s'il n'en a pas |
| `Finish` | applique les lignes cochées et ferme l'assistant |

`Remove all blank subtitles`, cochée par défaut, retire les sous-titres
qu'une correction laisserait sans aucun texte plutôt que de les garder vides.

**Un motif que le calcul n'a pas pu appliquer est nommé sous la table**, avec
la raison :

```text
Not applied: some pattern (will not compile)
```

Les autres motifs, ceux qui se sont bien appliqués, ne sont pas concernés.

## Ce qu'annuler dit, et ce qu'`Cancel` ne fait jamais

**`Finish` fait une entrée d'historique par projet touché**, jamais une par
correction : `Undo` défait d'un coup toutes les corrections acceptées sur ce
projet, chacune avec le texte qu'elle avait. Le menu la nomme
`Undo: correcting texts`.

La barre d'état dit le compte, les textes changés d'abord, les sous-titres
retirés ensuite :

```text
Edited 3 and removed 1 subtitles
```

**`Cancel`, à n'importe quelle page, n'applique rien et ne retient rien** — ni
les réglages choisis pendant cette ouverture, ni la moindre correction. La
prochaine ouverture de l'assistant repart des réglages d'avant.

## Un exemple

Un fichier ouvert porte deux espaces entre deux mots d'une réplique et une
mention entre crochets sans autre texte. `Tools ▸ Correct Texts…`, `Common
Errors` déjà coché, `Next` jusqu'à la confirmation : la ligne des deux espaces
montre `Bonjour  Marie` à gauche, `Bonjour Marie` à droite, la correction en
gras et déjà acceptée. `Finish` l'applique, et la barre d'état dit :

```text
Edited 1 and removed 0 subtitles
```
