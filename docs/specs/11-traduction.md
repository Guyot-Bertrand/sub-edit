# Phase 11 — Traduction et multi-projets

Cadrage de l'issue [#426](https://github.com/Guyot-Bertrand/sub-edit/issues/426),
après l'initialisation [#425](https://github.com/Guyot-Bertrand/sub-edit/issues/425).

## Ce que la feuille de route promet, et ce qui est déjà là

Second document en regard du principal, alignement du fichier de traduction par
numéro ou par position, onglets, sauvegarde et fermeture groupées, scission d'un
projet, ajout d'un fichier à la suite d'un autre — plus deux renvois : le lecteur
pour un document de traduction, venu de la phase 6, et la recherche au-delà du
document courant, venue de la phase 10.

**Et cette réserve, que la feuille de route écrit de cette phase et d'aucune
autre : « le besoin n'est pas confirmé ».** Un cadrage ne peut pas le confirmer à
la place de qui l'a écrit ; il peut construire la phase pour que la question se
pose au bon moment. C'est la décision D1.

**Le modèle a été écrit pour cette phase-ci, et le noyau l'accueille déjà.** Ce
qui manque est ce qui *entre* — un second fichier — et ce que la fenêtre en fait.

| Ce qui est déjà là | Où |
| :----------------- | :- |
| un sous-titre porte **les deux textes et une seule paire de positions** — « a translation has no timing of its own » | `Subtitle`, phase 1 |
| `Document` nomme les deux, et **les commandes le prennent** : le texte, la fusion et la scission, la dégradation à la conversion | phases 2, 4, 9, 10 |
| **l'historique tient ses modifications par document** : un changement de positions salit les deux, un changement de texte le sien | `affects(ChangeKind, Document)` |
| `Session::hasUnsavedChanges(Document)` et `markSaved(Document)` | `Session` |
| **l'écrivain écrit l'un ou l'autre texte** | `WriteRequest::document` |
| une lecture qui rend les diagnostics, dont `OutOfOrder`, et un noyau qui ne trie jamais de lui-même | ADR 0008, 0012 |
| huit paires principal/traduction, **avec l'alignement attendu de chaque méthode**, observé sur Gaupol lui-même | [#427](https://github.com/Guyot-Bertrand/sub-edit/issues/427), `src/test/data/paires/` |

**Ce qui n'y est pas.** `Project` ne retient qu'**un** `SourceFile` — un chemin, un
format, un encodage —, et `sourceFile()` est lu à vingt-neuf endroits, dont douze dans
des opérations du noyau qui y cherchent le dialecte de balises. La fenêtre écrit
`Document::Main` en dur à quatorze endroits. Et `MainWindow` retient, dans un seul
objet de 1 951 lignes, ce qui est à la fenêtre et ce qui est à un projet.

## Analyse préalable — ce que Gaupol fait

Lue dans `aeidon/agents/open.py`, `gaupol/agents/open.py` et `close.py`,
`gaupol/dialogs/split.py` et `multi_close.py`, `gaupol/actions/`, et **exécutée**
là où la lecture ne suffisait pas : les huit cas de #427 ont été confrontés au
paquet `aeidon`, copié hors du clone. Cinq constats gouvernent ce qui suit.

**La traduction est un second fichier posé sur la même liste de sous-titres.**
Elle n'a pas de temps à elle : ses lignes sont *rattachées* aux sous-titres du
principal, par numéro ou par position, et celles qui ne trouvent pas leur place
en font naître de nouveaux. `open_translation` **efface d'abord toutes les
traductions existantes**, puis aligne.

**Une opération de texte vise le document de la colonne qui a le focus.**
`text_column_to_document(col)` : la casse, les tirets, l'italique s'appliquent à
la colonne de texte courante, et l'action est grisée si le focus n'est pas dans
une colonne de texte. L'italique lit même les balises **du format de ce
document-là**.

**L'ouverture n'annonce rien, sauf le tri.** L'alignement ne dit pas combien de
lignes ont trouvé leur place ni combien ont fait naître un sous-titre ; en
revanche, si le fichier a dû être trié, une boîte demande s'il faut l'ouvrir —
c'est le comportement que l'[ADR 0012](../adr/0012-ordre-des-sous-titres-par-composition.md)
a déjà lu et refusé de copier.

**Fermer plusieurs projets pose une seule question.** Un dialogue liste les
documents modifiés — le principal et la traduction d'un même projet sont deux
lignes —, une case chacun, et répond enregistrer, fermer sans enregistrer ou
annuler ; annuler ne ferme rien. Avec un seul document à confirmer, c'est un
dialogue simple. **Un fichier disparu du disque compte comme modifié** : c'est la
seule copie de ce que le projet contient.

**L'ajout et la scission sont deux décalages inverses.** Ajouter décale de la fin
du dernier sous-titre ; scinder décale le projet neuf de *moins* la fin du dernier
sous-titre resté à l'origine. Les deux sont une seule entrée d'historique.

> **Une phrase du cadrage était fausse, et la lecture l'a corrigée.** #426
> écrivait que Gaupol trie la traduction « sans que la fenêtre le dise autrement
> que par un nombre rendu ». Il demande confirmation — c'est ce que l'ADR 0012
> disait déjà, et que le cadrage n'a pas relu avant d'écrire.

## D1 — La phase se livre en trois tranches, et une porte précède la dernière

La feuille de route écrit que le besoin n'est pas confirmé. Les six pièces — la
traduction, les onglets, l'enregistrement groupé, la fermeture groupée, la
scission, l'ajout — **ne portent pas le même besoin**, et les livrer ensemble parce
que Gaupol les livre ensemble est un choix qu'on n'a pas fait.

| Tranche | Ce qu'elle livre | Ce qu'elle touche |
| :------ | :--------------- | :---------------- |
| **1 — la traduction** | un second document en regard du principal, aligné, saisi, enregistré ; la question de fermeture à réponse riche | le modèle, la table, les opérations de texte |
| **2 — ajouter un fichier** | l'ajout à la suite d'un autre | un menu, une commande |
| **3 — le multi-projets** | des onglets, l'enregistrement et la fermeture groupés, la scission, la recherche partout | **toute la fenêtre** |

**Les deux premières valent sans la troisième**, et ne dépendent pas d'elle. La
troisième fait de `MainWindow` une page parmi d'autres : c'est le seul morceau de la
phase dont on ne revient pas facilement, et c'est celui dont la feuille de route
doute. **Une porte, [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435),
précède donc la dernière tranche** : ses cinq issues sont `blocked` jusqu'à ce que
la décision soit prise, et écrite, quand la fenêtre sait ce qu'une traduction et un
ajout lui apportent.

**Aucune des deux réponses n'est un échec.** Si le multi-projets est livré, l'[ADR
0033](../adr/0033-un-projet-est-une-page.md) passe de « proposée » à « acceptée ».
S'il ne l'est pas, ses cinq issues sont fermées avec leur raison, l'ADR est
abandonnée, et la phase se clôt sur sa relecture avec la traduction.

**#435 a tranché pour le multi-projets, le 2026-09-22.** L'ADR 0033 est acceptée, et
les cinq issues de la troisième tranche quittent `blocked` — #436, l'extraction de
`ProjectPage`, en premier.

## D2 — Un projet est une page, et la page est un onglet

**À décider par [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435), et
décrit ici pour qu'on décide en le sachant.** [ADR 0033](../adr/0033-un-projet-est-une-page.md),
proposée.

> **Corrigé en relecture de fin de phase.** L'ADR 0033 est **acceptée** depuis #435, le 2026-09-22 —
> D1 le dit, cette ligne ne le disait pas. Le paragraphe ci-dessous est resté au conditionnel du cadrage ;
> ce que la séparation est devenue est écrit plus bas, par #436 et #437.

`MainWindow` retient, dans un même objet, **deux sortes d'état** :

| À la fenêtre | À un projet |
| :----------- | :---------- |
| les menus et leurs actions, la barre d'état, le titre | la session et son historique (`m_session`) |
| **le lecteur**, la vue vidéo, la minuterie, **la table (`m_table`) — #436** | le modèle (`m_model`) |
| le thème, les répertoires, les réglages de l'ajustement | la ligne où la lecture a été placée (`m_placedAt`) |
| **le presse-papiers**, la boîte de recherche | la vidéo associée (`m_associated`, `m_watching`) |
| | la réplique que l'image porte (`m_shown`) |
| | la correspondance de la recherche (`m_match`, `m_searchTarget`) |

**Des onglets, c'est cette séparation faite, et la fenêtre qui en possède
plusieurs.** Un objet, `ProjectPage`, porte la colonne de droite ; la fenêtre en
garde une ou plusieurs, et **recalcule ses menus, sa barre d'état, son titre et son
film quand la page courante change**. Le lecteur n'est pas dupliqué : un seul, dont
le film se change — `syncVideo` sait déjà le faire quand l'association change — et
dont **la position de lecture ne survit pas au changement d'onglet**. Le manuel le
dit ; c'est le prix d'un seul processus mpv.

> **Revu par [#471](https://github.com/Guyot-Bertrand/sub-edit/issues/471), à la demande de l'utilisateur.**
> Le prix s'est révélé trop lourd à l'usage : chaque retour sur un onglet faisait repartir son film au début.
> Le lecteur reste unique, mais **chaque page retient la position de son film** au départ de son onglet
> (`ProjectPage::resumeAt`) et le lecteur y retourne au retour, en pause. Un autre film choisi entre-temps
> repart du début.

**Le presse-papiers reste à la fenêtre** : copier dans un projet et coller dans
l'autre est ce pour quoi on en ouvre deux.

**L'extraction se fait d'abord, seule** ([#436](https://github.com/Guyot-Bertrand/sub-edit/issues/436)) :
un refactoring qui n'a qu'une page, et dont les tests de la fenêtre passent sans
qu'on en réécrive un. C'est ce qui prouve la séparation avant qu'un second projet
ne l'éprouve.

**Une correction que #436 a dû faire : `m_table` reste à la fenêtre, pas à la page.**
La table du tableau ci-dessus le disait avec `m_model`, comme une seule chose ;
`ProjectPage` ne porte que le second. La vue Qt elle-même — la construction du
widget, son parent, sa place dans `QSplitter` — est un partage d'onglets à poser
(une pile de tables, ou une table qui change de modèle), et le poser maintenant
aurait été trancher une question de #437 dans une issue dont le seul critère est
de ne rien changer. `MainWindow::table()` rend le même widget qu'avant : aucun test
n'a eu à le savoir.

**Ce que la décision écarte** est dans l'ADR : une fenêtre par projet, moins chère
aujourd'hui, mais qui fait de « fermer tout » une affaire d'application et du
lecteur un objet par fenêtre ; et un `QTabWidget` dont on échangerait l'état à
chaque changement d'onglet, où vivent les bugs de l'état qui fuit d'un projet à
l'autre.

**#437 a tranché la question que #436 avait laissée ouverte : une seule table,
dont le modèle change.** Pas une pile de tables par page — l'option que #436
nommait sans choisir. `QTabBar` seule, pas `QTabWidget` : rien à empiler, une
étiquette par page et un signal quand on en choisit une autre.

**Deux choses que Qt fait de lui-même, et qu'il a fallu reprendre.**
`QAbstractItemView::setModel` jette le modèle de sélection qu'il avait et
en fabrique un neuf à chaque appel — y compris pour revenir à un modèle déjà
montré. Sans y prendre garde, chaque page aurait donc oublié sa sélection au
premier changement d'onglet. `ProjectPage` porte désormais la sienne
(`tableSelection`), faite une fois, remise à la table à chaque page — ADR 0033
gagne cette ligne, que le tableau ci-dessus ne comptait pas.

**Le lecteur partagé a le même piège, une ligne plus haut dans le code.**
`watchAssociatedVideo` ne fait rien quand l'association d'une page n'a pas
changé depuis la dernière fois qu'elle a été synchronisée — la bonne règle pour
un seul projet, fausse pour plusieurs : revenir à une page dont l'association
n'a pas bougé laisse le lecteur sur le film de la page qu'on vient de quitter.
Une deuxième chose à comparer, `m_playingPage`, qui dit quelle page pilote le
lecteur en ce moment — pas seulement laquelle la fenêtre montre.

**Deux entrées que l'issue promettait, et que cette PR ne livre pas** : glisser
un fichier sur la fenêtre, et un menu `Projects` énumérant les onglets. Ni
l'une ni l'autre n'est dans les critères de fin de #437 — seule la prose de
Gaupol les nommait.

**Le glisser-déposer a eu sa propre issue**, [#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453),
et ses cinq questions ont été tranchées avant d'écrire. **Toute la fenêtre** accepte le dépôt. **Un
sous-titre suit la route d'`Open…`** — un onglet chacun, un fichier déjà ouvert rend le focus à son onglet
sans être relu : `openFile` est désormais ce que les deux gestes partagent. **Une vidéo va à l'onglet
courant**, comme `Select Video…`, une fois les sous-titres du même dépôt ouverts. Plusieurs fichiers : le tri
de Gaupol (`isVideoFile`, la liste fermée d'extensions), avec deux écarts — **deux vidéos ou plus sont
refusées avec un message** plutôt qu'en silence, et **un fichier illisible n'arrête pas les autres** ; ce
qui n'a pu se faire est dit dans une seule boîte, une ligne chacun.
**Le menu `Projects` n'en a pas** : il double ce que la barre d'onglets fait
déjà, et `Ctrl+PageUp`/`Ctrl+PageDown` couvrent le clavier — à reprendre si
l'usage en montre le besoin, plutôt qu'un renvoi pour un geste redondant.

> **Précisé en relecture de fin de phase.** « Le menu `Projects` » désigne ici **la liste des onglets**
> que Gaupol y range, avec `Previous` et `Next`. Un menu `Projects` existe depuis #438, pour les deux
> gestes qui portent sur tous les projets à la fois — `Save All` et `Close All` — et **sans** cette liste :
> c'est elle, et elle seule, qui n'a pas de renvoi.

## D3 — Un document, un fichier

[ADR 0032](../adr/0032-un-document-un-fichier.md), acceptée.

**`Project` retient un second `SourceFile`, celui de la traduction, optionnel.**
Chemin, format, fins de ligne, encodage, en-tête, ce que le fichier déclare de lui-même
(ADR 0030) : tout ce qu'un document retient de son fichier pour se réécrire à
l'identique, et la traduction n'est pas moins un fichier que le principal. Une
traduction peut être en SSA quand le principal est en SubRip, et en Windows-1252 quand
il est en UTF-8.

**Sans fichier de traduction, elle suit le format du principal** : une traduction
tapée dans une colonne vide s'écrira comme lui, jusqu'à ce qu'on lui en choisisse un.

**Onze des douze sites du noyau qui lisent `sourceFile()` pour choisir un dialecte
lisent celui du document visé** : `italics_command` (2), `dialogue_dashes_command`
(2), `clipboard` (2), `search` (3), `letter_case_command` (1) et
`hearing_impaired_removal` (1). Aujourd'hui ils lisent tous le principal — les deux
derniers **reçoivent déjà un `Document`** et lisent pourtant le format du principal —,
ce qui n'est faux que le jour où une traduction existe. **Le douzième,
`duration_adjustment`, reste au principal, à dessein** : la vitesse de lecture se calcule
sur `mainText`, et l'ajustement est une opération de positions.

**Ils ne changent pas tous en même temps.** #429 en change huit — tous ceux qui reçoivent déjà
un `Document`. Les trois de `search` **attendaient** : la recherche ne prenait pas de `Document`, elle
lisait `mainText`, et c'est #433 qui le lui a donné. Ils nommaient le principal (`Document::Main`), et
un test le gardait ; ce test dit désormais les deux sens, une recherche par document.

**Ce que la décision écarte** est dans l'ADR : un fichier de traduction que le projet
ignorerait — la fenêtre le tiendrait à côté de sa `Session`, ce que l'ADR 0018 a déjà
refusé ; une traduction lue puis oubliée comme source, qu'on ne saurait plus enregistrer
à son chemin ni dans son format ; et un second `Project` lié au premier, qui dupliquerait
des positions qu'une traduction n'a pas.

**Ce que la décision coûte, et l'ADR le dit** : les réglages par sous-titre d'une
traduction — un style ASS, la position d'un sous-titre WebVTT — **ne sont pas conservés**.
Un sous-titre n'en porte qu'un jeu, celui du principal.

Première issue d'implémentation : [#429](https://github.com/Guyot-Bertrand/sub-edit/issues/429).

## D4 — Ouvrir une traduction

Deux méthodes, par position et par numéro, comme Gaupol ; **la position est le
défaut**, et les huit cas de `src/test/data/paires/` disent pourquoi : sur
`une-ligne-de-moins-au-milieu`, une seule ligne manquante fait glisser **toutes** les
suivantes par numéro, alors que par position une seule reste sans traduction.

**Six règles, dont quatre s'écartent de Gaupol, et chacun de ces écarts a sa raison.**

- **Les deux méthodes parcourent la traduction dans l'ordre du temps, et n'en trient
  aucun document.** Gaupol trie, compte, et demande confirmation ; l'[ADR
  0012](../adr/0012-ordre-des-sous-titres-par-composition.md) a refusé cette voie pour
  le principal, et le noyau ne trie jamais de lui-même. **Le compte des lignes qui n'étaient
  pas en ordre est dit**, dans le compte rendu de l'ouverture — la lecture rend déjà
  `OutOfOrder`. Le cas `traduction-dans-le-desordre` donne, pour les deux méthodes,
  ce que donne le témoin.
- **Un sous-titre né d'une ligne de traduction garde les positions de cette ligne,
  quelle que soit la méthode.** Par numéro, Gaupol invente trois secondes à la suite du
  dernier sous-titre — `_insert_blank_subtitles` — quand le fichier de traduction
  **énonce** les siennes. Inventer ce qui est écrit est une perte, et le cas
  `une-ligne-de-plus-a-la-fin` la montre : 15–18 s inventées d'un côté, 16–18 s dites de
  l'autre.
- **Ouvrir efface les traductions existantes**, comme Gaupol, **et entre dans l'historique
  en une seule entrée** — ce que Gaupol ne fait pas (`register=None`). L'annulation rend
  tout : les textes, les sous-titres nés retirés, le fichier de la traduction détaché.
  Gaupol laisse les sous-titres nés d'un premier alignement quand on en ouvre un second ;
  ici, on annule le premier avant d'ouvrir par l'autre méthode.
- **L'ouverture dit ce qui s'est passé.** ADR 0008 : dire plutôt que taire. Le compte est
  celui de `TranslationOutcome` — lignes rattachées à leur sous-titre, lignes qui ont fait
  naître un sous-titre, sous-titres restés sans traduction, lignes hors d'ordre — et la
  phrase qui le dit vit dans `core/wording.hpp`, l'endroit des mots que la ligne de commande
  reprendra. **Dans la barre d'état quand tout s'est rattaché et que rien n'est resté seul,
  dans une boîte à fermer sinon** : la règle de #398, pour un geste qui a quelque chose à
  dire.
- **Une traduction modifiée et non enregistrée est proposée à l'enregistrement** avant
  d'être remplacée : la question que Gaupol pose (`_show_translation_warning_dialog`).
- **Le fichier déjà ouvert comme principal est refusé**, et la fenêtre le dit.

**Ce que la lecture doit à l'existant** : une traduction en images (MicroDVD) se lit à la
fréquence du projet, une traduction sans fin (LRC, TMPlayer) avec les fins que la lecture
invente — ADR 0029 —, et **c'est sur elles que se calcule le milieu d'une ligne**.

**`positions-decalees` est le cas que la spec ne corrige pas, et le nomme.** Gaupol n'a pas
de détection de décalage : une traduction calée deux secondes trop tard n'est pas reconnue
pour ce qu'elle est, et sa première ligne se colle **sur le mauvais sous-titre** sans que
rien la distingue d'un rattachement juste. Le compte de l'ouverture est ce qui laisse
l'utilisateur le voir : trois lignes sur quatre y font naître un sous-titre.

**Ce que la fenêtre en a fait** (#432) : deux questions l'une après l'autre plutôt qu'un dialogue
unique — le sélecteur de `Open…`, puis un petit dialogue à nous pour la méthode, qui passe par
`Prompts::run` et se teste sans boucle modale. **L'encodage n'est pas demandé**, comme il ne l'est pas
à l'ouverture du principal : il est reconnu, et la fenêtre n'a jamais offert de le choisir à la
lecture. La lecture est faite **avant** la question de la méthode : un fichier qui ne s'ouvre pas, ou
qui est le principal, ne vaut pas qu'on demande comment l'aligner. Ce que la lecture a rencontré va au
panneau des diagnostics, celui de la dernière lecture. Une traduction qu'on vient d'ouvrir **n'est pas
modifiée** — c'est le principal qui l'est quand des sous-titres sont nés.

Issue : [#430](https://github.com/Guyot-Bertrand/sub-edit/issues/430) pour le noyau,
[#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) pour la fenêtre.

## D5 — La colonne de traduction, et le document que les opérations visent

**Une sixième colonne, `Translation`, à droite de `Text`.** Absente d'un projet sans
traduction — c'est ce que fait Gaupol, qui la retire à l'ouverture d'un principal et à la
création d'un projet —, elle apparaît quand le projet en reçoit une, et une entrée la bascule. **Cette entrée est celle d'un
menu `View` que la fenêtre n'a pas** — elle a `File`, `Edit`, `Video`, `Tools` et `Help` — **et qui naît
avec elle**, comme chez Gaupol ; c'est aussi là que se rangerait, un jour, le reste des colonnes
([#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)). Ses cellules se saisissent comme celles
du texte.

> **Écrit en relecture de fin de phase.** Ce « un jour » est venu dans la phase même : #442 a été traitée
> à la suite de la troisième tranche, et l'entrée de la traduction est passée dans un sous-menu,
> `View ▸ Columns`, avec celles des quatre autres colonnes qui se masquent — voir la fin de cette section.

**Le document visé est celui de la colonne courante** : la traduction si la cellule courante
est dans sa colonne, le principal ailleurs. C'est la règle de Gaupol, à une différence près :
**rien n'est grisé.** Gaupol désactive les opérations de texte hors d'une colonne de texte ;
ici, sans colonne de traduction, le document visé est toujours le principal, et la fenêtre se
comporte comme avant. Un grisé de plus ferait de la phase une régression pour qui n'ouvre
jamais de traduction.

| Ce que fait l'opération | Ce qu'elle vise |
| :---------------------- | :-------------- |
| **positions** — décaler, transformer, convertir la fréquence, aligner, ajuster les durées | les deux : les positions sont communes, et les commandes déplacent déjà les deux |
| **structure** — insérer, supprimer, fusionner, scinder | les deux : un sous-titre est un tout, et fusion et scission recollent déjà les deux textes |
| **texte** — retirer les mentions, italique, casse, tirets, copier, couper, coller, saisir une cellule | le document visé |
| **recherche** | le document visé — D8 |

**La barre d'état dit le document visé quand il y en a deux**, et se tait sinon.

**Retirer les mentions, visé sur la traduction, ne retire jamais de sous-titre.** Sur le
principal, un texte que la règle vide emporte son sous-titre : un sous-titre sans texte n'en est
pas un. Sur la traduction, le même geste aurait détruit le texte principal, qu'on n'avait pas
visé — c'est la question que #429 a fait remonter, et **Gaupol ne distingue pas les deux** : son
`remove_hearing_impaired` retire tout sous-titre dont le texte visé se vide, quel que soit le
document. Ici la traduction est **vidée** et le sous-titre reste ; le compte rendu la compte
comme nettoyée (`1 subtitle cleaned, 0 removed`), et annuler rend son texte. Refuser le geste
n'aurait rien protégé de plus, et laissait à l'utilisateur une traduction pleine de mentions
sans recours. C'est l'écart n° 8.

**Le presse-papiers ne porte pas de document** : il est une liste de textes. On peut donc copier
une cellule du principal et la coller dans la colonne de traduction, ce que Gaupol permet.

**Ce que #442 a ajouté : le reste des colonnes.** Le renvoi ci-dessus avait un destinataire sans
phase ; il a été traité à la suite de la tranche 3, sur trois réponses données avant d'écrire :

- **Toutes se masquent sauf `Text`**, sous `View ▸ Columns` — un sous-menu, comme Gaupol ; l'entrée de
  la traduction y est passée. Sans le texte principal il n'y a plus rien à éditer.
- **Toutes se déplacent**, en glissant l'en-tête, et **l'ordre comme les colonnes retirées sont
  retenus** dans les préférences (`table.order`, `table.hidden`, ADR 0022), avec les largeurs. La
  traduction n'y est pas : sa visibilité garde la règle ci-dessus.
- **Une cellule courante dans une colonne qu'on retire passe dans `Text`**, même ligne, sélection
  gardée — la règle que `targetDocument` appliquait déjà à une traduction masquée, étendue à la cellule.

Deux détails que l'implémentation a dû régler. Une colonne masquée mesure zéro, et le lecteur des
préférences refuse une largeur nulle — toute la ligne aurait été perdue : la fenêtre garde la largeur
qu'elle avait en partant. Et l'ordre survit au changement d'onglet sans rien faire : `setModel` garde
les sections quand leur nombre ne change pas.

Issues : [#431](https://github.com/Guyot-Bertrand/sub-edit/issues/431),
[#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442).

## D6 — Ajouter un fichier, et scinder un projet

**L'ajout est indépendant de tout le reste**, et c'est sa tranche. `Tools ▸ Append File…`, dans le
menu où Gaupol le range.

- Les positions du fichier ajouté sont **décalées de la fin du dernier sous-titre du projet**,
  comme Gaupol. Rien n'est inséré entre les deux. **L'ajout est grisé sur un projet sans sous-titre**,
  comme chez Gaupol (`len(subtitles) > 0`) : il n'y a pas de fin dont décaler.
- **Le fichier ajouté passe par la conversion.** Ses balises sont traduites dans le dialecte du
  projet et la perte est dite — la règle et les mots du collage d'un autre format, phase 10 —,
  et c'est `convertProjectFor` qui fait le travail. Les sous-titres ajoutés n'ont pas de
  traduction.
- Une seule entrée d'historique, « Appending file » ; la sélection est celle des sous-titres
  ajoutés.
- **Un fichier déjà ouvert est permis** : Gaupol le permet, et ajouter un fichier à lui-même est
  légitime.

**Une chose que ce cadrage promettait et que #434 a retirée : le sélecteur n'offre pas
l'encodage.** La même correction qu'à #432 s'applique ici : la fenêtre n'en offre nulle part à la
lecture, `Open…` compris, et en ajouter un pour ce seul sélecteur aurait été une incohérence de plus
plutôt qu'une richesse.

**Ce que #434 a dû trancher, et que ce cadrage ne disait pas.** L'ajout devient un `CommandKind` de
plus, et deux bascules exhaustives existantes en dépendent : l'ordre (`mayBreakOrder`) le classe comme
l'insertion et le collage — ce qu'il pose peut être hors ordre —, et le dépassement du film
(`movesPositions`) le classe **avec le décalage et non avec l'insertion** : ce qu'il ajoute est décalé
de tout ce que le projet porte déjà, et c'est exactement ce que l'avertissement de dépassement existe
pour repérer.

**La scission est l'inverse exact de l'ajout**, `Tools ▸ Split Project…`, grisée sous deux
sous-titres comme chez Gaupol. La suite est copiée dans un projet neuf, ouvert dans
un nouvel onglet, et retirée de l'origine ; le projet neuf est **décalé de moins la fin du dernier
sous-titre resté**, si bien que scinder puis ajouter rend le projet de départ. Ses deux textes
suivent leurs sous-titres, et il hérite du format et de l'encodage, sans chemin.

**Ce que la lecture de Gaupol ne disait pas, et que le cadrage tranche.** `Calculator.add` ne borne
pas à zéro, et `seconds_to_time` écrit même un signe `-` : si les deux moitiés d'un projet se
chevauchent à la coupure, Gaupol produit une position négative. `subedit` la sait représenter et aucun
fichier ne sait l'écrire — c'est `firstBeforeOrigin`. **La scission est donc refusée, elle nomme le
sous-titre qui tomberait avant l'origine, et propose de couper ailleurs** : la règle du décalage, pas
une règle de plus.

**Ce que #439 a livré.** `splitProject` rend un `std::expected<SplitProject, SplitRefusal>` : le
projet neuf et une commande — un `RemoveCommand` sous un nouveau `CommandKind::SplitProject`, que l'ordre
et le dépassement du film classent avec la suppression, rien n'y bouge à l'origine. La coupure est
bornée du deuxième sous-titre au dernier (`std::out_of_range` sinon : la boîte ne l'offre pas).

- **Le projet neuf hérite de la traduction quand l'origine en a une** — fichier de traduction sans chemin,
  même format, même encodage —, sans quoi ses textes de traduction n'auraient ni colonne ni état modifié.
- **Il naît modifié.** Aucun fichier ne le tient : le fermer sans demander perdrait les sous-titres.
  `Session::markUnsaved` est le pendant de `markSaved`, et n'est appelé que là.
- **Le refus nomme le sous-titre**, par `firstBeforeOrigin`, en numérotant comme la table.
- **La boîte s'ouvre sur la ligne courante.** Gaupol ne le disait pas ; c'est « à partir d'ici ».

Issues : [#434](https://github.com/Guyot-Bertrand/sub-edit/issues/434) pour l'ajout,
[#439](https://github.com/Guyot-Bertrand/sub-edit/issues/439) pour la scission.

## D7 — Enregistrer, et fermer plusieurs documents

**Chaque document s'enregistre à part.** `Save Translation` et `Save Translation As…` écrivent la
traduction dans son format et son encodage, avec le dialogue et l'avertissement de perte de
`Save As…` ; l'état modifié est par document, comme l'historique l'a déjà, et le titre de la fenêtre
l'est si l'un des deux l'est.

**Fermer, ou quitter, avec plusieurs documents modifiés pose une seule question.** `Prompts` reçoit la
liste des documents modifiés, une case chacun, et répond enregistrer ceux qui sont cochés, fermer sans
enregistrer, ou annuler ; **avec un seul document modifié, la question reste celle d'aujourd'hui.**
C'est le dialogue de Gaupol, et non une question par projet, plus simple à écrire et plus pénible à
répondre : trois projets modifiés feraient trois boîtes qu'on ne peut plus annuler d'un coup.

**Cette réponse plus riche est ce que la première tranche doit livrer**, avant les onglets : dès
qu'un projet a une traduction, il a deux documents à fermer. C'est pourquoi elle est dans
[#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) et non dans la fermeture groupée.
`FakePrompts` la prendra à cette occasion : son `nextUnsavedChoice`, qu'un seul fichier de tests
utilise, ne suffit plus.

**Un fichier disparu du disque compte comme modifié**, et la liste le dit. `FileSystem::exists`
sait déjà répondre.

**`Save All`, `Close All` et la sortie de la fenêtre sont de la troisième tranche** : ce sont les
mêmes questions posées sur N onglets. `Save All` écrit chaque document modifié qui a un fichier ;
**un document sans fichier ouvre `Save As…`, un à la fois**, et un abandon arrête la suite en disant
ce qui a été écrit. **`Save All As…` n'est pas livré** : c'est une suite de dialogues que `Save All`
fait déjà, un à la fois, pour ceux qui n'ont pas de nom.

**Ce que #432 a livré** : la réponse plus riche passe par `Prompts::run`, comme tout dialogue à nous —
un `UnsavedDocumentsDialog` dont on lit `choice()` et `toSave()` —, si bien que `Prompts` n'a pas gagné
de méthode ; `aboutUnsavedChanges` prend en revanche le document, pour dire de laquelle des deux il
s'agit. **Deux documents modifiés ouvrent la liste, un seul pose la question de toujours.** Écrire la
traduction lit **le fichier de la traduction** — chemin, format, encodage, fins de ligne, en-tête —, et
la perte annoncée est celle **du document écrit** : `convertFor` ne parcourt plus les deux textes avec le
format du principal. Ouvrir un autre fichier remplace la traduction, donc pose la même question.

**Ce que #438 a livré.** Un menu `Projects`, **avec ces deux entrées seulement** : `Save All` et
`Close All`. Celui de Gaupol énumère aussi les onglets et porte `Save All As…` ; le premier double la
barre d'onglets (D2), le second est écarté ci-dessus.

- **`Close All` est la fermeture de la fenêtre.** Elle n'a pas de méthode à elle : l'action est
  branchée sur `close()`, et `closeEvent` pose la question. La fenêtre garde toujours un projet
  — fermer tous les projets et fermer la fenêtre sont un seul acte, et il n'y a pas d'état intermédiaire
  à écrire. Le critère « quitter passe par la même question » est vrai par construction.
- **La liste est celle de #432, sur tous les onglets.** `mayDiscardAllChanges` rassemble les documents
  modifiés de chaque page, un vecteur d'index de page en parallèle. **`Save` lit les cases**, pas
  `toSave()` : celui-ci nomme des documents, et deux projets ont chacun un principal. Les noms ne
  sont pas qualifiés par projet — la liste de Gaupol non plus. Un seul document modifié, quel que soit
  l'onglet, pose la question simple, l'onglet amené au premier plan.
- **`Save All` réutilise `saveDocument`**, qui ouvre déjà `Save As…` pour un document sans fichier. Un
  abandon ou un échec arrête la boucle ; la boîte dit combien ont été écrits sur combien.
- **L'étiquette d'un onglet porte une étoile** tant que l'un de ses documents est modifié, mise à jour
  au même endroit que l'étoile de la fenêtre (`refreshActions`).

Issues : [#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432),
[#438](https://github.com/Guyot-Bertrand/sub-edit/issues/438).

## D8 — Le lecteur et la recherche

**Les deux renvois tombent ici, et ils prennent la règle de D5** plutôt que d'en inventer une
chacun.

- **La réplique dessinée sur l'image est celle du document visé**, ou du principal quand il n'y a
  pas de traduction. **Pas de réglage** : une seule règle pour dire quel texte, sur toute la fenêtre.
  C'est le renvoi de la phase 6 — « il faudra dire lequel des deux s'affiche, et si le choix est un
  réglage ou suit l'onglet actif » : il suit la colonne active, et ce n'est pas un réglage.
- **La recherche porte sur le document visé.** **Le `Document` est un paramètre** de `findNext`,
  `findPrevious`, `replaceMatch` et `replaceAll`, à côté de la cible, **et non un champ de
  `SearchOptions`** — la première rédaction de cette décision disait l'inverse, et #433 l'a corrigée :
  `SearchOptions` est ce que le fichier de préférences retient d'une session à l'autre (`search.regex`,
  `search.ignore-case`), et le texte visé n'est pas une préférence, il suit la colonne courante. La
  boîte dit dans
  quel champ elle cherche (`Searching in: Translation`, absente quand il n'y a qu'un texte) ; **remplacer écrit dans le texte source de ce document**, les balises de son
  format respectées. C'est la moitié du renvoi de la phase 10. Gaupol cherche dans le principal, la
  traduction ou les deux ; **« les deux » n'est pas livré**, et l'écrire est le motif : chercher dans
  deux textes à la fois ferait de chaque correspondance un couple (sous-titre, document), et la table
  qui se déplace toute seule y perdrait son sens.
- **La portée « tous les projets ouverts »** est de la troisième tranche : elle n'a de sens qu'avec
  plusieurs projets. `Replace All` y est **une entrée d'historique par projet touché**, si bien
  qu'annuler dans un onglet ne défait que ce que cet onglet a reçu.

**Ce que #440 a livré.** Une case `All open projects` dans la boîte de recherche — pas un sélecteur à
trois états : les deux portées d'avant, la sélection ou le document, ne se choisissent pas, elles se lisent
sur la table, et la nouvelle est la seule qui demande un geste. **Éteinte avec un projet, décochée d'office
quand il n'en reste qu'un** ; cochée, elle ignore la sélection et prend chaque projet en entier.

- **L'algorithme est celui de la fenêtre, pas du noyau** : le noyau cherche dans un projet et boucle sur
  lui-même, la fenêtre sait ce qu'est un onglet. Le projet montré d'abord ; si sa correspondance
  suivante est en fait le tour du même projet, les autres onglets sont visités dans l'ordre (à l'envers en
  remontant), chacun depuis son début ; le tour est dit `Search wrapped around` quand il repasse le bout.
- **Une correspondance qu'un onglet garde de sa dernière visite ne sert à rien** : les autres projets sont
  cherchés depuis leur début, jamais depuis leur ancienne correspondance, et un changement de motif ou
  d'option les oublie toutes.
- **`Replace All` passe par chaque page** : `switchToPage` puis `applyOperation`, comme `Save All`, ce qui
  fait une entrée d'historique par projet touché sans rien inventer.

  > **Corrigé par #461.** `applyOperation` reçoit désormais la page qu'il vise : `Replace All` donne à
  > chaque page sa commande derrière son onglet, sans `switchToPage` — une entrée d'historique par projet
  > touché, toujours, mais ni clignotement ni film rechargé. Le compte, `replaced N matches in M
  projects`, est `noticeOfReplaceAll(count, projects)`.
- **Sur le document visé de chaque projet** : la colonne est celle de la table, une seule pour toute la fenêtre.

**Deux choses que la décision ne disait pas, et que #433 a dû trancher.**

- **La table reste dans la colonne où elle était.** `selectRows` posait la cellule courante en colonne 0
  — donc dans le principal — chaque fois qu'elle sélectionnait des lignes : une recherche commencée dans
  la traduction continuait dans le principal dès la première correspondance. La colonne courante est
  conservée, et c'est ce qui rend vraie la règle « le texte visé est celui de la colonne courante » pour
  une opération qui déplace la sélection.
- **Une correspondance est une place dans un texte.** Quand la colonne courante passe d'un texte à
  l'autre, la correspondance retenue est oubliée : la traduction d'un sous-titre peut lire exactement ce
  que dit le principal, et un `Replace` la réécrirait sans que l'utilisateur l'ait vue dans ce texte.

**La réplique dessine le texte tel que le modèle le porte, balises comprises** — ADR 0009, que l'issue
n'a pas modifiée. Retirer les balises « selon le format du document » n'est pas livré : ce serait changer
ce que voit quelqu'un qui n'ouvre jamais de traduction, et le critère de l'issue est que rien ne change
sans traduction.

Issues : [#433](https://github.com/Guyot-Bertrand/sub-edit/issues/433),
[#440](https://github.com/Guyot-Bertrand/sub-edit/issues/440).

## D9 — La ligne de commande attend la phase 13

`-t/--translation-file` existe chez Gaupol, et la spec de la phase 4 a renvoyé ici « nettoyer le
document de traduction depuis la ligne de commande ». **Ni l'un ni l'autre n'entre dans cette phase**,
et le renvoi change de destinataire : la phase 13, celle de la ligne de commande complète.

Ce qui rend le renvoi honnête : **rien de ce que cette phase écrit n'est propre à la fenêtre.**
L'ouverture d'une traduction, son alignement et le compte qu'elle rend vivent au noyau ; la phrase qui le
dit vit dans `core/wording.hpp`, l'endroit des mots partagés. La phase 13 n'aura qu'une grammaire à
écrire. Et `Document::Translation` n'apparaît toujours nulle part dans `src/lib/subedit/cli` — une option
`--document` serait de la grammaire morte, jusqu'à ce qu'il y ait quelque chose à viser.

## Ce que la phase ne livre pas

Chacun avec une phase ou une issue, parce qu'un renvoi sans destinataire finit par désigner une phase
déjà passée.

- **La ligne de commande de la traduction** — phase 13, D9.
- ~~**Masquer et réordonner les colonnes de la table.**~~ **Livré malgré tout, par
  [#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)**, laissée sans milestone à dessein et
  traitée à la suite de la troisième tranche, une fois ses trois questions tranchées — voir D5.
- **`Save All As…`** — écarté, et pas renvoyé : D7.
- **Chercher dans le principal et la traduction à la fois** — écarté, et pas renvoyé : D8.
- **Les réglages par sous-titre d'une traduction** — un style ASS, une position WebVTT — **écartés** :
  un sous-titre n'en porte qu'un jeu, celui du principal. [ADR 0032](../adr/0032-un-document-un-fichier.md).
- **Détecter qu'une traduction est décalée.** Gaupol ne le fait pas, et le cas `positions-decalees`
  montre ce que cela coûte ; le compte de l'ouverture le rend visible, pas corrigeable. Ce serait de la
  correction de synchronisation : phase 14, le calage fin.

## Écarts avec Gaupol

**Huit, chacun avec sa raison**, et la relecture de fin de phase vérifie que le réalisé les tient et
qu'aucun autre ne s'est glissé.

> **Complété en relecture de fin de phase.** Les huit sont tenus. **Neuf autres s'étaient glissés**,
> tous dans la troisième tranche et dans les deux issues qui l'ont suivie, chacun dit dans sa section mais
> aucun dans ce tableau ; ils sont ajoutés en dessous du trait. Un seul n'était voulu par personne —
> la scission qui ne dit rien — et il devient une issue plutôt qu'un écart.

| Ce que fait Gaupol | Ce que fait `subedit` | Pourquoi |
| :----------------- | :-------------------- | :------- |
| par numéro, une ligne en excédent fait naître un sous-titre aux **positions inventées** (3 s) | **les positions de la ligne**, quelle que soit la méthode | le fichier les énonce — D4 |
| ouvrir une traduction **n'entre pas** dans l'historique | **une seule entrée**, annulable | l'ADR 0010, et rouvrir par l'autre méthode — D4 |
| **trie** la traduction et demande confirmation | **n'en trie aucune**, parcourt dans l'ordre du temps, et dit le compte | l'ADR 0012 — D4 |
| l'alignement **n'annonce rien** | **le compte** : barre d'état ou boîte | l'ADR 0008 — D4 |
| les opérations de texte sont **grisées** hors d'une colonne de texte | **rien n'est grisé**, le document visé est le principal sans colonne de traduction | ne pas régresser pour qui n'ouvre pas de traduction — D5 |
| retirer les mentions retire le **sous-titre entier** quand le texte visé se vide, traduction comprise | **vide la traduction** et garde le sous-titre | le texte principal n'était pas visé — D5 |
| `Save All As…`, chercher dans « les deux » textes | **non livrés** | D7, D8 |
| `-t/--translation-file` | **phase 13** | D9 |
| *ajoutés en relecture de fin de phase* | | |
| `Close All` ferme les projets et **laisse la fenêtre vide** | `Close All` **ferme la fenêtre** | la fenêtre garde toujours un projet : fermer tous les projets et la fermer sont un seul acte — D7, #438 |
| le menu `Projects` **liste les onglets** et porte `Previous`, `Next` | **sans la liste** ni ces deux entrées | la barre d'onglets et `Ctrl+PageUp`/`Ctrl+PageDown` le font déjà — D2 |
| scinder **écrit** une position négative quand les moitiés se chevauchent | **refusé**, le sous-titre nommé | aucun fichier ne sait l'écrire : la règle de `firstBeforeOrigin` — D6, #439 |
| le projet né d'une scission **n'hérite de rien** : format et encodage par défaut | il **hérite du format, de l'encodage et de la cadence**, sans chemin | l'inverse exact de l'ajout, qui convertit vers le format du projet — D6, #439 |
| le projet né d'une scission a **une entrée d'historique**, l'insertion de ses sous-titres | **un historique vide**, et le projet **marqué modifié** | annuler cette insertion viderait le projet qu'on vient de créer ; ce qui compte est qu'il demande avant d'être perdu — #439 |
| toutes les colonnes se masquent, `Text` comprise | **`Text` ne se masque pas** ; la visibilité de la traduction n'est pas retenue | sans texte principal il n'y a plus rien à éditer ; la traduction suit sa règle propre — D5, #442 |
| déposer plusieurs vidéos : **toutes ignorées, sans un mot** | **ignorées avec un message** | dire plutôt que taire, l'ADR 0008 — D2, #453 |
| déposer un fichier illisible **arrête** les suivants | **les autres s'ouvrent**, une seule boîte dit les échecs | un dépôt est un geste, et dix fichiers ne font pas dix boîtes — D2, #453 |

> **Corrigé par [#462](https://github.com/Guyot-Bertrand/sub-edit/issues/462).** La ligne « scinder le
> dit » a quitté le tableau : ce n'est plus un écart. La barre d'état dit `split N subtitles into a new
> project` — sans le nom du projet, qui n'en a pas encore — et la boîte sélectionne la ligne qu'elle
> désigne à chaque changement du nombre, comme Gaupol ; l'annuler rend la sélection d'avant, ce que
> Gaupol ne fait pas.

> **Ajouté par [#474](https://github.com/Guyot-Bertrand/sub-edit/issues/474) : la barre d'outils.** Elle ne
> portait qu'`Open…`, `Save`, `Undo`, `Redo` et `Italic`, et l'utilisateur a lu « une seule opération, et pas
> la plus fréquente » comme un oubli. Confrontée à la barre de Gaupol (`gaupol/application.py`,
> `_init_header_bar`), elle porte désormais :
>
> | Gaupol | `subedit` | Pourquoi |
> | :----- | :-------- | :------- |
> | `Open` et un menu des fichiers récents | `Open` | `subedit` n'a pas de liste de fichiers récents : rien à y mettre |
> | `Save` | `Save` | — |
> | `Undo`, `Redo` | `Undo`, `Redo` | — |
> | `Find and Replace` | `Find` | — |
> | `Preview` : un lecteur **externe** à partir de la ligne choisie | `Play` : le lecteur **intégré** | le film se joue dans la fenêtre (ADR 0020) ; Gaupol donne en plus au sien une barre à part, montrée avec le film, et `subedit` n'en porte que le bouton qui sert le plus |
> | — | `New` | ouvrir un projet est un geste qu'on cherche, #473 |
> | — | `Insert`, `Remove` | les gestes les plus fréquents de l'édition, demandés par l'utilisateur ; ouvrir une boîte n'est pas une raison de rester hors de la barre |
> | — | `Italic` | déjà sur la barre, la seule opération sans réglage ; l'en retirer aurait été une régression |
>
> Les boutons lisent un mot court (`iconText`), les menus gardent l'entrée entière. Un test fige le contenu et
> l'ordre de la barre.

## Exigences

**Quatorze, toutes `prévues`** — le registre s'alimente en début d'issue. Les cinq de la troisième
tranche restent `prévues` jusqu'à la décision de #435 ; si elle est négative, elles sont
`abandonnées` avec leur raison.

> **Corrigé en relecture de fin de phase.** **Dix-huit, toutes `implémentées`.** Quatre sont nées en
> cours de route : `GUI-TABS-03` et `GUI-SAVE-04` avec #438, `GUI-TABLE-03` avec #442, `GUI-TABS-04`
> avec #453. `GUI-TABS-02` a été resserrée par #438 à la fermeture, `Save All` ayant reçu la sienne.

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-TRANS-01` | ouvrir un fichier de traduction l'aligne sur le principal, par position ou par numéro |
| `GUI-TRANS-02` | l'ouverture dit ce qui s'est passé : lignes rattachées, sous-titres nés, sous-titres sans traduction, lignes hors d'ordre |
| `GUI-TRANS-03` | ouvrir une traduction est une seule entrée d'historique, et l'annulation rend le document tel qu'il était |
| `GUI-TRANS-04` | la colonne de traduction apparaît avec la traduction, et se saisit |
| `GUI-TRANS-05` | une opération de texte vise le document de la colonne courante, et la barre d'état le dit |
| `GUI-TRANS-06` | la traduction s'enregistre à part du principal, chacun avec son état modifié |
| `GUI-CLOSE-01` | fermer avec plusieurs documents modifiés pose une seule question, une case par document |
| `GUI-SEARCH-03` | la recherche porte sur le document visé, et remplacer écrit dans son texte source |
| `GUI-PLAYER-04` | la réplique dessinée sur l'image est celle du document visé |
| `GUI-APPEND-01` | ajouter un fichier décale de la fin du dernier sous-titre, dit ce que la conversion perd, et s'annule d'un coup |
| `GUI-TABLE-03` | les colonnes, sauf le texte, se masquent ; toutes se déplacent ; la table s'en souvient |
| `GUI-TABS-01` | plusieurs projets s'ouvrent en onglets, chacun avec son historique, sa sélection et sa vidéo |
| `GUI-TABS-02` | fermer tout, ou quitter, avec une seule question pour tous les projets |
| `GUI-TABS-03` | l'étiquette d'un onglet dit que son projet est modifié |
| `GUI-TABS-04` | glisser des fichiers sur la fenêtre les ouvre : un onglet par sous-titre, une vidéo à l'onglet courant |
| `GUI-SAVE-04` | enregistrer tout écrit chaque document modifié, onglet par onglet, et s'arrête à un abandon en disant ce qui a été écrit |
| `GUI-PSPLIT-01` | scinder un projet en deux, et l'ajout de la suite rend le projet de départ |
| `GUI-SEARCH-04` | la recherche porte, au choix, sur tous les projets ouverts |

**Le noyau de l'alignement n'a pas d'exigence**, et c'est la règle du registre : une exigence est ce que
le binaire montre. `GUI-TRANS-01` et `GUI-TRANS-02` sont ce que la fenêtre en montre ; les huit cas de
`paires/` tiennent le noyau.

## Découpage

L'ordre va du fondateur au coûteux. **La traduction passe avant tout**, parce qu'elle est le seul morceau
dont la phase ne peut se passer, et que les deux autres tranches lui ajoutent.

| Issue | Ce qu'elle fait | Ce dont elle dépend |
| :---- | :-------------- | :------------------ |
| [#429](https://github.com/Guyot-Bertrand/sub-edit/issues/429) | un second fichier par projet, et un format par document | — |
| [#430](https://github.com/Guyot-Bertrand/sub-edit/issues/430) | ouvrir une traduction : l'aligner sur le principal | #429, #427 |
| [#431](https://github.com/Guyot-Bertrand/sub-edit/issues/431) | la colonne de traduction, et le document que les opérations visent | #429 |
| [#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) | ouvrir et enregistrer une traduction, et fermer avec deux documents modifiés | #430, #431 |
| [#433](https://github.com/Guyot-Bertrand/sub-edit/issues/433) | la recherche et la réplique suivent le document visé | #431 |
| [#434](https://github.com/Guyot-Bertrand/sub-edit/issues/434) | ajouter un fichier à la suite d'un autre | — |
| [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435) | **décider si la phase livre le multi-projets** | #432, #433, #434 |
| [#436](https://github.com/Guyot-Bertrand/sub-edit/issues/436) | extraire la page de projet de la fenêtre | #435 |
| [#437](https://github.com/Guyot-Bertrand/sub-edit/issues/437) | des onglets : plusieurs projets dans une fenêtre | #436 |
| [#438](https://github.com/Guyot-Bertrand/sub-edit/issues/438) | enregistrer tout, fermer tout | #437, #432 |
| [#439](https://github.com/Guyot-Bertrand/sub-edit/issues/439) | scinder un projet en deux | #437, #434 |
| [#440](https://github.com/Guyot-Bertrand/sub-edit/issues/440) | chercher dans tous les projets ouverts | #437, #433 |

La phase se clôt sur [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441), la relecture de fin.

> **Écrit en relecture de fin de phase.** Deux issues se sont ajoutées à ce tableau, après #440 :
> [#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442), les colonnes, sans milestone, et
> [#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453), le glisser-déposer que #437 avait laissé,
> dans la phase. Et #437 a pris un patch de documentation à elle seule, #454, qui a créé #453.

**#429 et #434 ne dépendent de rien** et peuvent se faire à tout moment ; #429 est placée en tête parce
que trois issues l'attendent. **#432 est la plus grosse de la première tranche**, et pour deux raisons
opposées : elle porte un dialogue d'ouverture à trois choix, un enregistrement à part, et la réponse
plus riche de la fermeture — qui sert aux trois tranches.

**#436 est un refactoring sans comportement neuf**, et c'est ce qui la rend risquée : tous les tests de la
fenêtre doivent passer sans qu'on en réécrive un. Si l'un d'eux doit changer, c'est que la séparation
n'était pas celle qu'on croyait.

## Les issues ouvertes par la relecture

Relecture de fin de phase, [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441), le
2026-09-24. **Quatre axes retenus**, chacun une issue de la milestone ; la phase se clôt après eux.

| Issue | Ce qu'elle dit |
| :---- | :------------- |
| [#460](https://github.com/Guyot-Bertrand/sub-edit/issues/460) | `MainWindow` : 1 951 lignes à l'ouverture, 2 968 à la relecture. `ProjectPage` a rendu l'état, pas le comportement — tous les gestes de la troisième tranche se sont ajoutés à la fenêtre |
| [#461](https://github.com/Guyot-Bertrand/sub-edit/issues/461) | les opérations visent la page de l'onglet courant ; `Save All`, la question de fermeture et `Replace All` sur tous les projets **changent donc d'onglet pour viser une page** — l'écran clignote et le lecteur change de film à chaque passage |
| [#462](https://github.com/Guyot-Bertrand/sub-edit/issues/462) | `Split Project…` ne dit rien et ne montre pas où il coupe — le seul écart avec Gaupol que personne n'a voulu |
| [#463](https://github.com/Guyot-Bertrand/sub-edit/issues/463) | le dépôt de fichiers **sur le film** — une fenêtre native que libmpv dessine — n'est vérifiable que sur un vrai bureau |

> **#460 livrée**, [ADR 0034](../adr/0034-trois-collaborateurs-de-la-fenetre.md) : trois collaborateurs —
> `TableColumns`, `ProjectSearch`, `ProjectFiles` — qui reçoivent la page qu'ils visent et se testent
> sans fenêtre. `main_window.cpp` est passé de 2 968 à 2 319 lignes, sans qu'un test de la fenêtre soit
> réécrit. Les gestes sur plusieurs projets changent encore d'onglet pour viser une page : c'est #461.
>
> **#461 livrée** : `applyOperation` et `ProjectSearch::View::apply` reçoivent la page visée, chaque page
> tient son propre onglet à jour (`refreshTabOf`), et `Save All`, la question de fermeture et `Replace
> All` n'amènent plus un onglet au premier plan que devant un `Save As…`. Deux tests de la fenêtre le
> prouvent — l'onglet ne bouge pas, le lecteur n'ouvre aucun film — sans qu'un test existant de la
> fenêtre soit réécrit.
>
> **#483 livrée**, ADR 0034 complétée : `WindowActions` construit les actions, leurs raccourcis, les
> menus et la barre d'outils ; la fenêtre le garde comme un seul membre et connecte chaque action à son
> slot. `main_window.cpp` passe de 2 531 à 2 216 lignes, et aucun test de la fenêtre n'est réécrit.
>
> **#484 livrée**, ADR 0034 complétée : `VideoPane` possède le lecteur, la surface, la bande, le
> minuteur et la page pour laquelle il joue, et reçoit la page visée. `main_window.cpp` passe de 2 216 à
> 1 923 lignes, et aucun test de la fenêtre n'est réécrit.
>
> **#485 livrée**, ADR 0034 complétée : `StatusLine` possède les quatre étiquettes permanentes de la barre
> d'état et se rafraîchit depuis le projet affiché. `main_window.cpp` passe de 1 923 à 1 851 lignes, et
> aucun test de la fenêtre n'est réécrit.

**Écartés, avec leur raison.**

- **Le nom d'un document — son fichier, ou `untitled` — est calculé à quatre endroits** (`titleFor`,
  `tabLabelFor`, `modifiedDocuments`, `mayReplaceTranslation`). Quatre lignes chacun, qui ne divergent pas :
  une fonction de plus ne rendrait rien de plus sûr. Si #460 découpe la fenêtre, la question se reposera
  d'elle-même.
- **La liste de fermeture ne dit pas à quel projet appartient un document** : deux `untitled` modifiés
  s'y lisent pareil. La liste de Gaupol non plus ; et les cases restent justes, puisqu'elles sont lues dans
  l'ordre de la liste et non par leur nom.
- **`Document::Main` en dur** : il n'en reste aucun à tort. Dans la fenêtre, ce sont `Save`, les boucles
  sur les deux documents et le défaut du document visé ; dans le noyau, les surcharges « principal » et
  l'ajustement des durées, qui reste au principal à dessein (D3) ; dans la ligne de commande, les trois
  que D9 laisse à la phase 13.

**Le banc.** Deux relevés pour toute la phase, 0.11.1 et 0.11.7 : chaque version suivante a trouvé la
machine au-dessus du seuil, et **la troisième tranche n'a aucune mesure**. La relecture ne l'a pas trouvée
plus calme — charge 5 pour un seuil de 1,5. Le relevé est dû à la clôture ; issue
[#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270).

