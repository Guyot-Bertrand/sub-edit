# Feuille de route

Vingt-cinq phases, chacune associée à un milestone GitHub. Neuf mènent à un
**MVP livrable** — les huit premières, plus la [16](#16--fréquences-dimage--déduction-et-correction)
intercalée avant la 7 ; les suivantes, jusqu'à la 15, complètent
l'iso-fonctionnalité avec Gaupol ; les huit dernières, de la
[17](#17--formats-texte-complémentaires) à la [24](#24--workflow-et-intégrations),
vont [au-delà de Gaupol](#troisième-partie--au-delà-de-gaupol).

**Le numéro d'une phase l'identifie, il ne dit pas son rang.** L'ordre est celui
de ce document, et lui seul. Une phase ajoutée en cours de route prend le
premier numéro libre plutôt que de décaler les suivantes — c'est déjà la règle
des identifiants d'exigences, et pour la même raison : un renvoi qui change de
référent égare plus qu'un numéro dans le désordre. Six ADR, sept fichiers de
`src/` et neuf milestones citent des numéros de phase ; les décaler les rendrait
tous silencieusement faux.

Ce document tient le cadrage amont : ce qu'il faut analyser, ce qu'il faut
trancher, et ce qui sera difficile. Il est révisé au fil du projet — une phase
terminée voit son cadrage remplacé par sa spec dans [`specs/`](specs/).

## Le contour du MVP

Les priorités viennent de l'utilisateur final, transmises le 2026-08-05 :

- **ouvrir** — surtout SubRip (`.srt`) et WebVTT (`.vtt`) ; plus rarement
  SubViewer (`.sub`), Sub Station Alpha (`.ssa`), Advanced SSA (`.ass`) ;
- **enregistrer sous** — surtout `.srt` et `.vtt`, en UTF-8, avec des fins de
  ligne Unix ;
- **éditer les sous-titres** dans la table centrale ;
- **décaler les positions**, **transformer les positions**, **convertir la
  fréquence d'image** ;
- **enlever les textes pour malentendants** — sons entre crochets et entre
  parenthèses.

Les phases 1 à 7 sont donc **restreintes à ce contour**, interface comprise.
Elles ne changent ni d'identité ni d'ordre : elles couvrent moins de terrain.
La [16](#16--fréquences-dimage--déduction-et-correction), elle, l'élargit — la
seule à le faire, et la seule à dépasser l'iso-fonctionnalité avec Gaupol avant
la troisième partie.

Trois conséquences qui ne se lisent pas directement dans la liste :

- **Le modèle de balises reste nécessaire dès la phase 1.** SubRip porte `<i>`,
  `<b>`, `<font>`, WebVTT les siennes. Il doit de plus être conçu pour
  accueillir ASS en phase 9, sans quoi cette phase imposerait de le reprendre.
- **L'architecture d'annulation se pose dès la phase 1.** Elle n'est pas dans
  la liste, mais éditer dans une table l'implique, et c'est précisément ce qu'on
  ne peut pas ajouter après coup.
- **Le document de traduction n'est pas tranché.** Le modèle de données est donc
  conçu pour l'accueillir, sans que l'interface le construise : l'ajouter au
  modèle plus tard coûterait cher, l'ajouter à l'interface ne coûte rien.

## Déroulé d'une phase

Trois issues encadrent chaque phase — une pour l'équiper, une pour la cadrer,
une pour la relire — et les issues d'implémentation viennent entre les deux
dernières.

### 1. Initialisation — l'outillage (`type:task`)

**Avant le cadrage.** La question qu'elle pose : *qu'est-ce qui, dans cette
phase, se vérifiera à la main faute d'outil ?* Ce qui se vérifie tout seul n'a
pas à se vérifier à la main, et l'outil coûte moins cher construit au début de
la phase qu'à la fin.

Elle produit une décision par outil : ceux qui sont retenus ont leur issue, ceux
qui sont écartés ont **leur raison écrite** — un outil écarté sans trace revient
à chaque phase.

Les issues d'outillage qu'elle ouvre passent **avant la première issue
d'implémentation**. C'est une contrainte d'ordre sur la phase et non une tâche
du ticket d'initialisation : quand celui-ci se termine, rien n'est encore en
place, par construction. L'écrire comme un critère de fin donnerait une case
qu'on ne peut jamais cocher.

### 2. Cadrage (`type:task`)

1. **Analyse préalable** — lecture ciblée du code de Gaupol correspondant, dans
   `reference/gaupol`, pour comprendre ce qui est fait et pourquoi.
2. **Discussion des choix d'architecture** — les questions listées ci-dessous
   sont le point de départ, pas la liste complète.
3. **Production de la spec** dans `docs/specs/NN-<sujet>.md`, et des ADR pour
   les décisions coûteuses à revenir dessus.
4. **Découpage en issues d'implémentation**, rattachées au milestone.

### 3. Relecture de fin de phase (`type:task`)

**Après la dernière issue d'implémentation, avant de clore.** Elle ne produit
pas de fonctionnalité : elle vérifie que la phase tient.

- **Confronter la spec au réalisé.** Un écart est soit corrigé, soit inscrit ;
  un écart non consigné est un écart oublié.
- **Répercuter les renvois.** Tout « à voir plus tard » doit atterrir sur une
  phase ou une issue nommée. Sans cela une promesse finit par désigner une phase
  déjà passée — c'est arrivé en phase 2, où la borne de l'historique promettait
  d'être mesurée « en phase 2 » et ne l'a pas été.
- **Relire le manuel d'un bloc.** Il se met à jour ticket par ticket, donc en
  morceaux, et par des gens qui connaissent déjà la réponse. C'est le seul
  moment où l'on voit ce que ça donne pour quelqu'un qui découvre : ce qui
  manque, ce qui a vieilli, ce qui est exact mais rangé là où personne ne le
  cherchera.
- **Regard critique sur l'ensemble du code de la phase**, et non fichier par
  fichier : duplications, abstractions manquantes ou de trop, tests qui
  promettent plus qu'ils ne prouvent. Ce qui en sort **n'est pas corrigé là** :
  chaque axe se discute et devient une issue s'il est retenu.

Une phase n'est close que lorsque sa spec, ses tests, ses benchmarks, sa section
de manuel, son entrée de CHANGELOG et sa relecture existent.

**L'analyse se fait au démarrage de la phase concernée, pas maintenant.** Les
questions et points difficiles listés ci-dessous sont des repères relevés lors
de l'exploration initiale de Gaupol : ils servent à ne pas partir d'une page
blanche et à ne pas découvrir tard un obstacle connu. Ce ne sont ni des
conclusions, ni une liste close.

---

# Première partie — vers le MVP

## 0 — Fondations

**Terminée le 2026-08-05.** Voir [`specs/00-fondations.md`](specs/00-fondations.md).

Structure `lib`/`exe`/`test`, CMake et presets, façade `make`, porte `make check`
en cinq étapes prouvée dans les deux sens par `make verify-gates`, CI qui
n'exécute rien d'autre que cette porte, hooks git, CHANGELOG généré, cinq ADR,
et verrouillage du dépôt.

---

## 1 — Noyau : modèle de données et formats

**Terminée le 2026-08-08.** Voir [`specs/01-noyau.md`](specs/01-noyau.md).

Sous-titre, document, projet, positions, SubRip et WebVTT, balises, et
l'architecture de commandes réversibles.

**Restreint au contour du MVP :** deux formats sur neuf, UTF-8 seul, fins de
ligne Unix seules, pas de détection automatique d'encodage.

**Analyse préalable** — `aeidon/` : `subtitle.py`, `position.py`,
`calculator.py`, `project.py`, `containers.py`, `revertable.py`, `file.py`,
`files/{subrip,webvtt}.py`, `markup.py`, `markups/{subrip,webvtt}.py`,
`parser.py`.

**Questions d'architecture**

- **Représentation des positions.** Millisecondes entières en interne, frames
  dérivées de la fréquence d'image ? La conversion de fréquence figure dans les
  priorités, donc les allers-retours ne doivent pas dériver. Quelle politique
  d'arrondi, et quelle garantie sur `frames → ms → frames` ?
- **Gestion d'erreurs.** Exceptions, codes de retour, ou type résultat.
  `std::expected` est disponible avec le GCC 13 installé, en `-std=c++23` :
  retenir le type résultat reviendrait à passer le projet en C++23, sans changer
  de compilateur. Décision par ADR, elle imprègne toute l'API.
- **Architecture d'annulation.** Gaupol stocke dans chaque action une *fonction
  inverse* et ses arguments (`RevertableAction.revert_function`). Alternative :
  des commandes qui capturent l'état antérieur. La première est économe en
  mémoire mais impose que chaque opération sache s'inverser exactement ; la
  seconde est robuste mais coûteuse sur un remplacement global. À trancher par
  mesure.
- **Modèle de balises.** SubRip et WebVTT suffisent au MVP, mais le modèle doit
  accueillir ASS en phase 9 — positionnement, styles nommés, effets — sans être
  repris. Où placer la frontière entre ce qui est commun et ce qui est propre à
  un format ?
- **Tolérance au parsing.** Les fichiers réels sont malformés. Échec net, ou
  récupération avec rapport de diagnostics ? Le choix conditionne la signature
  de toutes les fonctions de lecture.

**Points difficiles**

- Le modèle *projet* porte **deux documents**, principal et traduction, qui
  **partagent les positions**. La traduction n'a pas de temps propres. Le modèle
  doit l'accueillir même si l'interface ne l'expose pas encore.
- Une interface de format unique doit accommoder : avec ou sans en-tête, temps
  ou frames, jeux de balises disjoints. Si une implémentation doit lever « non
  supporté », le découpage est mauvais. Le risque est faible avec deux formats
  proches, et c'est justement le piège : la conception doit tenir avec neuf.

---

## 2 — Opérations d'édition

**Terminée le 2026-08-09.** Voir
[`specs/02-operations-d-edition.md`](specs/02-operations-d-edition.md).

Ce qu'exige l'édition dans la table, plus les trois opérations de positions
listées en priorité.

**Restreint au contour du MVP :** modifier un texte, un début, une fin ;
insérer, supprimer ; décaler, transformer, convertir la fréquence d'image. Sont
reportés en phase 10 : ajustement des durées, casse, italiques, tirets de
dialogue, fusion, scission, recherche et remplacement, presse-papiers.

**Analyse préalable** — `aeidon/agents/` : `set.py`, `edit.py`, `position.py`.

**Questions d'architecture**

- Toute opération est-elle une commande annulable de premier ordre, y compris la
  frappe dans une cellule ? Comment se fait le **regroupement** d'actions en une
  seule entrée d'annulation ? — *close, sans mécanisme : un délégué valide une
  fois, donc une cellule éditée produit une commande. Voir
  [la spec de la phase 2](specs/02-operations-d-edition.md#le-groupement-dactions--question-close).*
- Modèle de **cible** : sélection, plage, projet entier. Gaupol le traite par un
  paramètre `target` répété dans chaque signature — on peut faire mieux.
- Où passe la frontière entre opération du noyau et logique d'interface ?

**Points difficiles**

- Transformation affine des positions à partir de deux points de repère, avec
  les cumuls d'erreur d'arrondi que cela suppose.
- La conversion de fréquence d'image s'applique à des sous-titres en temps :
  elle rééchelonne toutes les positions. Vérifier au cadrage ce que
  l'utilisateur en attend exactement — resynchroniser un fichier calé sur
  23,976 vers 25 images par seconde est le cas courant.

---

## 3 — CLI

**Terminée le 2026-08-14.** Voir [`specs/03-cli.md`](specs/03-cli.md).

La ligne de commande n'apparaît pas dans les besoins de l'utilisateur : elle
sert de **harnais de validation et de mesure** du noyau, avant qu'il existe une
fenêtre. Les sous-commandes destinées à un usage réel relèvent de la phase 13.

**Le périmètre annoncé ici a été élargi au cadrage.** Il disait inspection,
conversion et décalage ; la spec y ajoute la transformation par deux points de
repère et la conversion de fréquence d'image. La raison tient en une phrase : le
noyau les implémente déjà, et un harnais qui ne les expose pas ne les valide
pas — elles resteraient sans aucun test de bout en bout jusqu'à la phase 13.

Deux questions ouvertes ici y sont tranchées : CLI11 pour l'analyse d'arguments
([ADR 0016](adr/0016-cli11-pour-l-analyse-d-arguments.md)), et quatre codes de
retour distinguant l'échec total de l'échec partiel sur un lot.

**Ce que `Project::outOfOrder()` doit rendre reste ouvert, délibérément.** La
phase 2 compare chaque sous-titre à son prédécesseur immédiat ; l'autre lecture
compare au plus grand début rencontré. Sur les départs `0, 4000, 2000, 3000`, la
première rend `{2}`, la seconde `{2, 3}`. Les deux s'accordent toujours sur
l'existence d'un désordre et ne diffèrent que sur la liste.

L'inspection est le premier appelant à consommer cette liste. Plutôt que de
trancher sans données, elle **expose les deux** sous une option, le temps de les
comparer sur des fichiers réels. **La phase 5 hérite du choix et fait
disparaître l'option** — c'est son déclencheur, inscrit comme tel dans la spec.

---

## 4 — Suppression des mentions pour malentendants

**Cadrée.** Voir [`specs/04-mentions-pour-malentendants.md`](specs/04-mentions-pour-malentendants.md).

**Fortement restreinte.** Le moteur de correction complet — motifs par langue,
découpage de lignes, correcteur orthographique — relève de la phase 12. Ici,
seuls les deux motifs demandés :

```
Sound in brackets      \[.*?\]    → chaîne vide
Sound in parentheses   \(.*?\)    → chaîne vide
```

Aucune référence arrière : **l'arbitrage entre PCRE2 et RE2 ne bloque pas cette
phase**, et peut être différé à la phase 12 où il se posera vraiment.

**Analyse préalable** — `aeidon/` : `pattern.py`, `patternman.py`,
`agents/text.py` (méthode `remove_hearing_impaired`), et
`data/patterns/Latn.hearing-impaired`.

**Ce que la lecture a donné**, relevé à l'issue #88 pour ne pas la refaire :

| Constat | Détail |
| :------ | :----- |
| cinq motifs, pas deux | crochets, parenthèses, paroles entre `#`, paroles sur une ligne, nom du locuteur avant deux-points — les trois derniers relèvent de la phase 12 |
| sections homonymes | `[Hearing Impaired Pattern]` se répète, ce qu'aucun lecteur INI standard n'accepte |
| `Flags=DOTALL;MULTILINE;` | le motif traverse le saut de ligne, et c'est nécessaire : dans de vrais fichiers, une mention est souvent coupée par lui |
| `Replacement=\0` | le `\0` protège la valeur, ici vide ; `-\040` donne un tiret et une espace |
| `.conf` XML | `enabled="false"` pour tous les motifs — rien n'est actif par défaut |

**Les données de motifs ne sont pas reprises.** Gaupol est en GPL-3 comme ce
projet, donc la copie serait licite ; elle n'est pas utile. Les deux motifs de
la phase tiennent en deux expressions, la spécification écrite dans
`src/test/data/textes/mentions.cas` en dit déjà bien davantage — références,
crochets vides, tirets de dialogue, mentions à cheval — et le format des
fichiers de motifs reste une décision de cadrage. Reprendre une donnée dans un
format qu'on n'a pas choisi serait s'engager avant d'avoir décidé.

**Les deux questions d'architecture posées ici sont tranchées au cadrage**, et
la spec les développe.

*Reprendre le format INI des motifs, ou définir le nôtre ?* **Ni l'un ni
l'autre : aucun fichier de motifs dans cette phase.** La transformation est
décidée, pas configurable ; les fichiers, leur `.conf` et l'activation par nom
arrivent avec le moteur de la phase 12, qui choisira le format en sachant ce
qu'il doit porter.

*Comment le moteur est-il conçu pour que la phase 12 l'étende ?* **Il n'y a pas
de moteur** — [ADR 0017](adr/0017-analyseur-de-mentions-ecrit-a-la-main.md). La
règle du projet n'est pas une substitution : elle laisse *exactement un espace
entre ce qui entourait la mention*, ce qui se décide au site du retrait, quand
une passe d'expression rationnelle est globale et réécrit du texte qu'on ne lui
a pas demandé de toucher. Gaupol le paie en sept passes de rattrapage. Un
balayage écrit à la main tient la règle exactement, et la phase 12 reste libre
de son moteur.

**Le point difficile est tranché lui aussi.** Supprimer `[Bruit de pas]` ne
laisse pas une ligne vide : une ligne que le retrait vide disparaît, et un texte
entièrement vidé emporte son sous-titre — sans option, là où Gaupol offre
`remove_blank`. La vacuité ignore les balises de format, parce que neuf
sous-titres réels du corpus s'écrivent `<i>[PEOPLE SCREAMING]</i>` et
laisseraient sinon un `<i></i>` à l'écran.

---

## 5 — Interface : édition tabulaire

**Cadrée.** Voir [`specs/05-interface-tabulaire.md`](specs/05-interface-tabulaire.md).

Le cœur de l'usage : ouvrir, éditer dans la table, enregistrer sous, annuler.

**Restreint au contour du MVP,** et resserré au cadrage : une seule fenêtre, un
seul projet à la fois, colonnes numéro / début / fin / durée / texte, dialogues
des opérations des phases 2 et 4. Sont reportés : multi-projets en onglets,
colonne de traduction, colonnes configurables, coloration des différences —
et, décidés au cadrage, **l'insertion et la suppression de sous-titres** ainsi
que **la configuration persistée**, tous deux en phase 7.

**Analyse préalable** — `gaupol/` : `view.py`, `page.py`, `application.py`,
`renderers/*.py`,
`dialogs/{open,save,position_shift,position_transform,framerate_convert}.py`.

**Ce que la lecture a donné**, relevé pour ne pas la refaire :

| Constat | Détail |
| :------ | :----- |
| la table est intégralement dupliquée | `page.py` recopie chaque sous-titre dans un `Gtk.ListStore` et le resynchronise par huit gestionnaires de signaux |
| et le prix est écrit dans le code | `reload_view_all()` à chaque ouverture, le modèle **débranché** au-delà de 50 lignes retirées, un `iterate_main()` après chaque rafraîchissement |
| le numéro n'est pas une donnée | une fonction de cellule le calcule depuis l'indice de ligne |
| aucune migration de configuration, jamais | une clé inconnue est effacée à l'écriture, une valeur illisible laisse le défaut, une option au défaut est réécrite **commentée** ; `general.version` est écrit et jamais relu |
| la fréquence d'entrée vient d'une préférence | `conf.editor.framerate`, 23,976 par défaut — aucune heuristique de nom, aucune lecture de vidéo, en vingt ans |
| l'édition multiligne coûte un widget | une `Gtk.TextView` implémentant `Gtk.CellEditable`, `Entrée` et `Échap` à la main, un contournement à la perte de focus |

**Les questions d'architecture posées ici sont tranchées au cadrage**, et la
spec les développe.

*Adaptateur mince ou modèle propre synchronisé ?* **Adaptateur mince** —
[ADR 0019](adr/0019-table-en-adaptateur-mince.md). Le choix ne se discute pas
par goût : les trois cicatrices ci-dessus sont ce que coûte la duplication, et
elles sont dans le code de la référence.

*Comment l'interface se branche-t-elle sur l'historique du noyau sans en tenir
un second ?* `Session::apply`, `undo` et `redo` **rendent ce qu'ils ont
changé**, au lieu de `void`. Aucun signal n'entre dans le cœur. Pas de
`QUndoStack`. Et la question de groupement laissée ouverte en phase 2 se referme
sans mécanisme : un délégué valide une fois, donc une cellule éditée produit une
commande.

*Configuration typée et persistée ?* **Pas ici.** La phase 7 la porte déjà, et
rien de persistant n'est requis pour que la table fonctionne.

*Un `ChangeKind` pour la fréquence d'image ?* **Toujours non.** La phase 2 avait
posé la condition — « si la fenêtre affiche la fréquence courante ». Elle ne
l'affiche pas, donc l'énumérateur n'aurait toujours aucun lecteur.

*D'où vient la fréquence d'entrée d'une conversion ?* **De l'utilisateur, et
sans heuristique.** Le dialogue la demande, pré-remplie par celle du projet.
L'heuristique de nom est écartée faute de données : les fichiers de `src/data/`
ont été renommés à la main, aucun fichier aux noms intacts n'est disponible, et
se tromper de fréquence décale tout le fichier sans rien signaler. Gaupol ne
fait pas mieux.

**La question a toutefois trouvé sa réponse pendant ce cadrage, et elle a fait
naître une phase.** Convertir une fréquence suppose que les positions étaient
calées sur une grille d'images ; quand c'est le cas, la grille se mesure — huit
des quinze fichiers du corpus privé la donnent sans ambiguïté, contre un bruit
de fond de quelques pour cent sur les fréquences qu'ils n'ont pas.
Le fichier ne *déclare* pas sa fréquence, mais il la *trahit*. C'est une
donnée et non une corrélation, et c'est l'objet de la
[phase 16](#16--fréquences-dimage--déduction-et-correction), programmée juste
après la 6. La phase 5 n'en dépend pas : son dialogue demande la fréquence, et
recevra la proposition mesurée quand elle existera.

**Le cadrage a par ailleurs ouvert le noyau,** ce qui n'était pas prévu :

- **le vocabulaire des formats rejoint le modèle** — [ADR 0018](adr/0018-vocabulaire-des-formats-dans-le-modele.md).
  `SourceFile` ne retenait pas le format du fichier, et une fenêtre qui ouvre
  puis enregistre en a besoin. `format/` ne garde que des opérations, `io/` et
  `text/` accueillent ce qui ignore jusqu'au mot « sous-titre » ;
- **les diagnostics se scindent** — ce qu'une lecture a rencontré se repère par
  une ligne, ce qu'un document est se repère par un indice. Une ligne n'existe
  qu'au moment de la lecture ; un indice survit à l'édition, et **une table
  surligne des rangs**. `inspect` désignera donc un numéro de sous-titre là où
  il donnait un numéro de ligne ;
- **la lecture du désordre est tranchée : `Breaks`**, et `--order-report`
  disparaît. Le corpus ne l'a pas départagée — aucun de ses quinze fichiers
  n'est en désordre, les deux lectures s'y accordent trivialement. La décision
  est prise par raisonnement, et la spec le dit plutôt que de laisser croire à
  une mesure.

**Le point difficile est tranché lui aussi.** Rester fluide sur plusieurs
milliers de lignes tient à ce que le modèle ne matérialise jamais ce qui n'est
pas visible — l'adaptateur mince l'assure par construction — et à des signaux
fins plutôt que globaux, ce que `Change` porte déjà. Une réserve subsiste et
elle est écrite : **un changement de structure passe par une réinitialisation du
modèle**, parce que Qt exige d'encadrer avant et que `Session` ne rapporte
qu'après. Tenable tant que son seul producteur est le retrait des mentions ; à
reprendre en phase 7, qui apporte l'insertion et la suppression.

[#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) passe **avant** la
table, et non après : le modèle traduit un `Change` en `dataChanged(topLeft,
bottomRight)`, donc Qt veut des plages et non des indices. La représentation
compacte doit exposer des intervalles utilisables tels quels.

---

## 6 — Le lecteur intégré

**Cadrée.** Voir [`specs/06-lecteur-integre.md`](specs/06-lecteur-integre.md).

**Re-cadrée deux fois, et la seconde annule la première.** Elle était d'abord
tout le lecteur vidéo ; puis restreinte à une prévisualisation par lecteur
externe, l'intégré passant en phase 14 comme « la partie la plus coûteuse du
projet » ; puis rendue au lecteur intégré au cadrage.

Ce que la première restriction avait manqué : **Gaupol a les deux.**
`gaupol/player.py` est un lecteur GStreamer embarqué, distinct de
`aeidon/agents/preview.py`. La feuille de route avait retenu le second et oublié
le premier. Le lecteur externe disparaît ; la vidéo se regarde dans la fenêtre,
avec la réplique courante dessinée par-dessus, et le backend est libmpv — voir
[l'ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md).

Ce qui suit décrit le cadrage abandonné, gardé parce qu'il montre d'où l'on
partait.

**Analyse préalable** — `aeidon/agents/preview.py`, `aeidon/enums.py` (les
commandes des trois lecteurs), `gaupol/agents/preview.py`.

**Questions d'architecture — toutes tranchées au cadrage**, et la spec porte les
réponses. Ce qui suit reste écrit tel qu'il l'était avant, parce que la valeur
de ces lignes est de montrer d'où l'on partait.

- Détection du lecteur disponible, et commande personnalisable.
- Association d'un fichier vidéo à un projet : par convention de nom, comme
  Gaupol (`find_video`), ou choix explicite ?
- **Lire la fréquence d'image dans la vidéo associée ?** La conversion de
  fréquence de la phase 2 a besoin d'une fréquence d'entrée que le fichier de
  sous-titres ne *déclare* pas. Le conteneur vidéo, lui, l'annonce, et une vidéo
  est déjà associée au projet ici, avant le lecteur intégré de la phase 14.

  **C'est la seconde source de la même donnée.** La
  [phase 16](#16--fréquences-dimage--déduction-et-correction) la déduit des
  positions elles-mêmes, sans vidéo. Deux mesures indépendantes valent une
  vérification croisée — et un désaccord entre elles est une information, à
  condition de savoir la présenter.

  À vérifier au cadrage : ce que coûte cette lecture. La prévisualisation lance
  un lecteur externe et n'a donc aucune bibliothèque vidéo en mémoire ; lire des
  métadonnées demanderait `ffprobe` — un exécutable de plus à détecter — ou une
  dépendance. Le gain est réel, le prix reste à mesurer.
- **Valider les opérations contre la durée de la vidéo.** Même mécanisme que la
  question précédente, et donc même prix : la durée est une métadonnée du
  conteneur, lue en même temps que la fréquence ou pas du tout.

  Ce qu'elle permettrait : refuser, ou du moins signaler, une opération qui
  pousse des sous-titres au-delà de la fin du film. Aujourd'hui le noyau ne peut
  vérifier qu'une borne, celle de zéro — un décalage négatif trop grand rend une
  position négative, et c'est tout ce qu'il sait dire. La borne haute n'existe
  pas pour lui, faute de savoir où le film s'arrête.

  Le décalage est le cas le plus net. Les autres opérations sont concernées
  **sous d'autres formes** : une transformation dont le second repère tombe
  après la fin, une conversion de fréquence qui étire l'ensemble au-delà. Chacune
  demande sa propre formulation, et aucune ne se déduit de celle du décalage.

  Relevé au cadrage de la phase 3, où rien ne pouvait en être fait : la CLI n'a
  aucune notion de vidéo, et lui en donner une avant que le projet en ait une
  serait bâtir la vérification avant la donnée.
- Fichier temporaire : durée de vie, encodage forcé en UTF-8.

---

## 16 — Fréquences d'image : déduction et correction

**Cadrée.** Voir [`specs/16-frequences-d-image.md`](specs/16-frequences-d-image.md)
et l'[ADR 0021](adr/0021-analyse-du-document-a-l-ouverture.md).

**Dans le MVP, programmée entre la [6](#6--le-lecteur-intégré) et la
[7](#7--finitions-et-première-livraison).** Son numéro est le premier libre au
moment où elle a été ajoutée ; il l'identifie et ne dit pas son rang.

**Elle dépasse l'iso-fonctionnalité, et c'est délibéré** — la première à le
faire. Gaupol ne déduit aucune fréquence d'image : son dialogue de conversion
pré-remplit ses deux listes avec `conf.editor.framerate`, une préférence globale
à 23,976, et n'a jamais fait mieux en vingt ans. Ce qui suit n'a donc pas de
contrepartie à lire dans `reference/gaupol`.

### D'où elle vient

Le cadrage de la phase 5 butait sur une question que la feuille de route porte
depuis le début : **d'où vient la fréquence d'entrée d'une conversion ?** Le
fichier ne la déclare pas — SubRip n'a pas d'en-tête, celui de WebVTT est du
texte libre — et une heuristique tirée du nom de fichier avait été écartée comme
une corrélation, non une donnée.

L'observation qui ouvre cette phase est ailleurs : **convertir une fréquence
suppose que les positions étaient calées sur une grille d'images.** Quand c'est
vrai, chaque position vaut `round(n × 1000 / R)`, et cette grille se mesure.

### Ce que la mesure a déjà donné

Relevé au cadrage de la phase 5, puis **refait** après que les jeux d'exemples ont
montré que la première méthode était la mauvaise.

**Aucun de ces chiffres n'est reproductible ailleurs.** `src/data/` est ignoré
par git — corpus privé de chaque machine, comme `reference/gaupol` est un clone
privé. Les relevés qui suivent sont donc des observations consignées, pas des
mesures qu'un tiers peut rejouer, et **aucun test de cette phase ne pourra lire
ces fichiers.** Il lui faudra des fixtures versionnées ; il n'en existe pas.

#### La méthode : chercher une grille *et sa phase*

Pour chacune des huit fréquences normalisées, on calcule la phase de chaque
début sur la grille d'images correspondante, et on mesure **la concentration de
ces phases** — la longueur du vecteur résultant. Cent pour cent : une grille
parfaite, quelle que soit sa phase. Près de zéro : aucune structure.

Elle n'a **aucun paramètre de tolérance**, et elle est insensible à un décalage.

**C'est un jeu d'exemples qui l'a imposée.** Une première version cherchait une
grille de phase nulle, et notait à zéro un fichier qui est un 24 images par
seconde parfait, décalé d'un millième de seconde et demi. Ses onze cent dix-sept
débuts tombent dans exactement **trois** classes de résidu — ce que produit une
grille à 24 images écrite en millisecondes entières — et le décalage appliqué par
Gaupol translate les trois de la même quantité.

#### Ce que les opérations de Gaupol font à la grille

Vérité terrain : les jeux d'exemples 004, 005 et 006 du corpus privé sont des
fichiers produits par Gaupol, l'opération appliquée étant consignée à côté.

| Opération | Ce qu'elle fait à la grille |
| :-------- | :-------------------------- |
| conversion de fréquence, 25 → 24 et 25 → 23,976 | **la transpose** — chaque sortie annonce sa nouvelle fréquence à 100, et l'ancienne retombe au bruit |
| décalage, de −7,001 s et de +2,999 s | **la préserve**, seule la phase change — 100 avant comme après, dans les deux sens |
| transformation par deux repères | **la détruit** — 100 avant, 3 après |

La conversion et le décalage sont donc réversibles du point de vue de la
déduction ; la transformation, non. C'est une propriété utile : elle dit ce
qu'on peut espérer retrouver d'un fichier et ce qui est perdu.

#### Sur les quinze fichiers du corpus privé

| Verdict | Fichiers | Fréquences trouvées |
| :------ | -------: | :------------------ |
| grille nette — concentration ≥ 90 | **8** | cinq à 23,976, un à 24, un à 25, un à 29,97 |
| indice partiel — de 50 à 90 | 4 | trois à 29,97, un à 23,976 |
| muet — moins de 50 | 3 | — |

**L'échec est bruyant.** Un fichier écrit en millisecondes sans grille reste sous
quelques pour cent sur les huit candidates : c'est un « je ne sais pas » sans
équivoque, et non une mauvaise réponse. C'est la propriété qui autorise à s'en
servir, là où une heuristique de nom de fichier ne l'aurait jamais offerte.

**L'ambiguïté harmonique est réelle et apparaît dès le premier fichier.** Une
grille à 25 est incluse dans une grille à 50 : deux fichiers sortent à 100 sur
les deux. Dans l'ensemble normalisé, les seules paires ambiguës sont celles dont
l'une est le multiple entier de l'autre — **25 et 50, 29,97 et 59,94, 30 et 60**.
Aucune autre : 24 et 60 sont dans un rapport de deux et demi, donc une image sur
deux seulement coïncide.

**Les débuts sont le signal, les fins corroborent.** Partout où une grille
existe, les débuts sont à 100 sans exception ; les fins vont de 55 à 100. Le
*cue-in* est posé sur une image, le *cue-out* est souvent calculé par une règle
de vitesse de lecture.

### Questions d'architecture

- **Que rend la déduction ?** Un classement de candidates avec leur
  concentration, ou une réponse et une confiance ? Le second est plus simple à
  consommer, le premier ne cache rien — et un fichier partiel n'est ni l'un ni
  l'autre.
- **L'ensemble des candidates doit être clos et petit** — les huit fréquences
  normalisées. Résoudre pour un `R` quelconque est un tout autre problème, et
  sans objet : personne ne masterise à 26,3 images par seconde.
- **La phase est une information, pas un déchet.** Un fichier dont les positions
  sont sur la grille à une constante près a été décalé, et cette constante se
  mesure. Faut-il la rendre ? La corriger ?
- **Où vit la fonction ?** Elle est pure et ne dépend que des positions :
  `core/time/`, ou un `core/analysis/` qui accueillerait aussi les anomalies
  d'un document.
- **Que recouvre « corriger » ?** Trois mécanismes distincts, à trier : ramener
  la phase à zéro ; refuser ou signaler une conversion dont la fréquence
  d'entrée contredit la mesure ; retrouver la paire d'une conversion faite avec
  la mauvaise fréquence, en cherchant le rationnel qui remet le fichier sur une
  grille normalisée. Le troisième est le plus utile et le moins sûr.
- **Surface exposée.** Une sous-commande de la ligne de commande, ou une lecture
  de plus dans `inspect` ? Et dans la fenêtre : le dialogue de conversion
  pré-remplit sa fréquence d'entrée **en montrant sa mesure**, jamais en
  l'appliquant en silence.
- **Articulation avec la [phase 6](#6--le-lecteur-intégré),** qui pose la même
  question par l'autre bout : la vidéo associée *déclare* sa fréquence. Deux
  sources indépendantes pour la même donnée, donc une vérification croisée
  gratuite — et un désaccord à savoir présenter.

### Points difficiles

- ~~**Les fichiers de test n'existent pas.**~~ **Réglé.** Les deux corpus qui
  ont servi à tout ce qui précède sont ignorés par git, et la plus fournie des
  fixtures versionnées portait six horodatages sur quinze secondes. C'était la
  première question du ticket d'initialisation, et c'est le seul outil qu'il a
  retenu : `src/scripts/subtitle-fixtures.py` engendre quinze fichiers sur des
  grilles connues — les huit fréquences normalisées, une fréquence absurde, une
  grille décalée, une étendue insuffisante, les deux visages du fichier
  partiel, des fins calculées par une règle de vitesse de lecture, et un fichier
  écrit en millisecondes sans aucune grille. Les deux derniers manquaient à la
  phase 16 et ont été écrits en phase 10, issue #373.
  `src/test/data/grilles/LISEZMOI.md` porte ce que chacun est et ce que chacun
  donne.
- **L'édition manuelle sort de la grille.** Dès qu'un utilisateur corrige une
  position dans la table, elle cesse d'être alignée. Un détecteur naïf
  signalerait le travail de l'utilisateur comme une anomalie. C'est la raison
  pour laquelle la phase 5 s'interdit d'utiliser la grille pour marquer quoi que
  ce soit.
- **Distinguer 23,976 de 24 demande de l'étendue.** Elles divergent de 3,6 s par
  heure : écrasant sur un film de deux heures, invisible sur un extrait de trente
  secondes. La déduction doit rendre l'étendue qu'elle a eue sous les yeux, et
  pas seulement sa concentration.
- **Les fichiers partiels sont le cas intéressant et le plus dur.** Ni grille ni
  bruit — quatre des quinze du corpus privé, entre 50 et 83. Dire « les deux
  tiers de vos débuts sont sur une grille 29,97 » est vrai et n'aide personne ;
  dire *lesquels* et *où ils se groupent* aide, et demande de décider ce qui
  compte comme un bloc. L'observation faite au cadrage : les écarts isolés
  ressemblent à des anomalies ponctuelles, les écarts groupés à une section
  retimée ou à un fichier assemblé.

---

## 7 — Finitions et première livraison

**Livrée en `v0.8.0`.** Voir
[`specs/07-finitions-et-premiere-livraison.md`](specs/07-finitions-et-premiere-livraison.md),
l'[ADR 0022](adr/0022-configuration-au-noyau-et-tolerance-par-option.md) et
l'[ADR 0023](adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

Ce qui manque pour qu'un tiers installe et utilise l'outil.

Préférences persistées, thème clair et sombre suivant le système, manuel
utilisateur complet pour le contour livré, empaquetage Linux.

**Le thème ne suit pas le système, et c'est D3 qui le dit.** Qt 6.4 n'a aucune
API de schéma de couleurs — `QStyleHints::colorScheme` est arrivée en 6.5 —
donc « système » laisse la palette au thème de plate-forme au lieu de la lire.
C'est ce que fait Gaupol sous GTK antérieur à 4.20, et c'est ce qui rend les
deux autres valeurs éprouvables.

**Un renvoi de la phase 16 y attend son contenu :** l'entrée `Help ▸ Manual` de
la fenêtre existe et est éteinte depuis #211. Le manuel qu'elle ouvrira et
l'endroit où il sera installé se décident avec l'empaquetage, pas avant.

**Trois renvois du cadrage de la phase 5 atterrissent ici**, et le premier n'y
était pas prévu :

- **insérer et supprimer des sous-titres depuis la fenêtre.** Les commandes
  existent au noyau depuis la phase 2 et ne sont exposées nulle part — ni par la
  ligne de commande, ni par la table. La phase 7 est leur première surface, et
  le premier moment où elles auront une preuve de bout en bout ;
- **la configuration persistée elle-même**, que la phase 5 a laissée entière :
  elle n'a besoin de rien de persistant pour que sa table fonctionne. Ce qu'elle
  aurait voulu retenir arrive donc ici — géométrie de la fenêtre, largeur des
  colonnes. **La fréquence d'image par défaut, elle, n'est pas venue et ne
  viendra pas** : rien ne la lirait, et une préférence dont personne ne se sert
  est une case qui ment. Instruite puis écartée en
  [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267) ;
- **un `Session` qui annonce un changement de structure avant de le faire**, si
  la mesure le demande. La phase 5 réinitialise le modèle pour toute insertion ou
  suppression de lignes ; son seul producteur y est global et rare, ce qui ne
  sera plus vrai dès qu'un menu ajoutera une ligne à la fois.
  [ADR 0019](adr/0019-table-en-adaptateur-mince.md) porte le déclencheur.

**Ce que la lecture de `gaupol/config.py` a donné**, relevé au cadrage de la
phase 5 pour ne pas la refaire : **il n'y a aucune migration, et c'est un
dispositif.** Le fichier est lu option par option ; une clé inconnue est acceptée
puis effacée à l'écriture si elle n'est plus dans les défauts ; une valeur
illisible imprime sur la sortie d'erreur et laisse le défaut en place ; toute
option restée à sa valeur par défaut est réécrite **commentée**, si bien qu'un
changement de défaut prend effet chez qui ne l'a jamais surchargé.
`general.version` est écrit et n'est jamais relu : une trace, pas un
déclencheur. La question « format de fichier et migration » ci-dessous se pose
donc avec une réponse possible déjà sur la table — la tolérance par option,
plutôt qu'une migration versionnée.

**Analyse préalable** — `gaupol/config.py`, `gaupol/style.py`, `data/`,
`PACKAGING.md`, `flatpak/`.

**Ce que la phase a laissé, et où chaque chose atterrit.** La relecture de fin
de phase a donné un endroit à quatre renvois qui n'en avaient pas, et sept axes
sont sortis de son regard critique :

**Trois d'entre eux ont été instruits et refermés aussitôt**, et c'est un
résultat plutôt qu'un échec : un « non » écrit ne se repropose pas.

| Ce qui a été tranché | La décision |
| :------------------- | :---------- |
| le déclencheur de l'ADR 0019 | **abandonné** — [#264](https://github.com/Guyot-Bertrand/sub-edit/issues/264) : D8 se suffit, et la mesure qui manquait ne changerait rien |
| Flatpak et AppImage | **écartés** — [#265](https://github.com/Guyot-Bertrand/sub-edit/issues/265) : `.deb` et `.rpm`, et ailleurs on construit depuis les sources |
| la fréquence d'image par défaut | **écartée** — [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267) : aucun lecteur n'en avait besoin |

| Ce qui reste | Où |
| :----------- | :- |
| un conteneur Fedora pour éprouver le `.rpm` | [#266](https://github.com/Guyot-Bertrand/sub-edit/issues/266) |
| vérifier avec l'outil qui compte — le manuel, la page de manuel | [#268](https://github.com/Guyot-Bertrand/sub-edit/issues/268) |
| le `Makefile` force l'analyse complète pour un commentaire | [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) |
| le seuil de charge du journal des mesures | [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270) |
| `check-installation.sh` a triplé | [#271](https://github.com/Guyot-Bertrand/sub-edit/issues/271) |
| `subedit-gui` sans page de manuel | [#272](https://github.com/Guyot-Bertrand/sub-edit/issues/272) |
| la langue des commentaires et des intitulés de tests | [#273](https://github.com/Guyot-Bertrand/sub-edit/issues/273) |
| deux raccourcis que Qt ne pose pas sous X11 | [#274](https://github.com/Guyot-Bertrand/sub-edit/issues/274) |

**Questions d'architecture**

- Format de fichier de configuration et migration entre versions.
- Empaquetage : Flatpak, `.deb`, AppImage — lequel pour une première livraison ?
  **Répondu : `.deb` et `.rpm`**, les deux autres écartés — ADR 0023 et #265.
- **Règles `install()` dans CMake, et cibles `install` / `uninstall`.** Le
  projet n'en a aucune : l'outil se lance depuis l'arbre de construction, et
  `docs/manual/subedit-cli/installation.md` le dit. Relevé au cadrage de la
  phase 3 et laissé ici volontairement — une cible écrite avant que le format
  d'empaquetage soit tranché préjugerait de la réponse.

  Deux conséquences à traiter en même temps : les exemples du manuel montrent
  `$ subedit-cli` comme si l'outil était dans le `PATH`, ce qui ne deviendra
  vrai qu'ici ; et `make manual` l'exécute depuis `build/dev/bin`, ce qui restera
  le bon choix pour la génération même une fois l'installation possible.

**À l'issue de cette phase, le MVP est livrable.**

---

# Seconde partie — couverture complète

Ces phases sont transverses : chacune touche la bibliothèque, la ligne de
commande et l'interface. Leur ordre est indicatif et sera revu avec l'utilisateur
une fois le MVP en service — c'est l'usage réel qui doit le déterminer, pas une
prévision faite maintenant.

## 8 — Encodages et fins de ligne

Détection automatique de l'encodage, jeu complet d'encodages, fins de ligne
Windows et Mac, forçage à l'enregistrement.

**Cadrée** — [`docs/specs/08-encodages.md`](specs/08-encodages.md), issue #288.

**La question ouverte est tranchée, et par la mesure.** Elle demandait un
équivalent C++ à `charset-normalizer`, entre ICU, `uchardet` et
`compact_enc_det`. Les trois voies ont été passées au même corpus étiqueté avant
qu'aucune soit choisie — ICU 9/9, `uchardet` 9/9, un prototype de tables écrites
6/9 — et **ce qui a décidé n'est pas la détection mais la conversion** : ICU est
le seul des trois à convertir, et l'iso-fonctionnalité demande les
quatre-vingt-dix-sept encodages de Gaupol, qui ne s'écrivent pas à la main.
[ADR 0027](adr/0027-icu-pour-les-encodages.md).

**Deux choses que le cadrage a trouvées et que ce paragraphe promettait mal :**

- **les fins de ligne sont déjà livrées.** `Newline` porte `Lf`, `CrLf` et `Cr`
  depuis la phase 1, `--line-endings unix|windows|mac` existe depuis la phase 3,
  et un fichier lu est réécrit avec les siennes. La moitié du titre était faite ;
- **la fenêtre n'offre rien de tout cela.** `Save As…` choisit un chemin et un
  format ; ni encodage, ni fin de ligne, ni BOM. La ligne de commande sait faire
  ce que la fenêtre ne propose pas, et cet écart entre dans le périmètre.

**Livrée**, en six issues d'implémentation — #294 à #299 — et close par
`v0.9.0`. Ce qui a été fait : l'encodage entre dans le modèle et le BOM devient
sa variante, la lecture dans un encodage donné, la détection, l'écriture,
`--encoding` et `--to-encoding` en ligne de commande, et `Save As…` qui choisit
l'encodage, la fin de ligne et la marque. Dix exigences, dix au registre.

**Quatre écarts entre la spec et le réalisé**, tous inscrits dans
[la spec](specs/08-encodages.md) plutôt que corrigés dans le code — le réalisé
avait raison les quatre fois :

| La spec disait | Le réalisé |
| :------------- | :--------- |
| D6 : « `--encoding` à la lecture **et à l'écriture** » | deux options, parce qu'une seule invocation fait les deux |
| « la phase ne livre pas la conversion en lot » | `--to-encoding` avec `--output-dir` réencode un répertoire |
| D4 : « **l'encodage retenu** entre dans les diagnostics » | seulement s'il a été deviné et n'est pas de l'UTF-8 |
| « UTF-32 : ICU le lira si on le lui demande » | **il ne le lira pas** — mesuré : sa marque est prise pour celle de l'UTF-16LE |

**Ce que la relecture a tranché.** Le seuil de charge du banc, laissé ouvert par
#270, l'est : **ce n'est ni le seuil ni le délai**, mais l'endroit et le moment
où la charge est lue. Deux changements, le second trouvé parce que le premier a
échoué à sa première exécution — `make check-local` passe avant `make check`, et
`bench.sh` lit la charge **avant** de compiler l'arbre Release au lieu d'après,
c'est-à-dire avant d'avoir occupé lui-même la machine sur laquelle il
s'interroge. [ADR 0015](adr/0015-memoire-des-mesures.md).

**Ce que la phase laisse, et où chaque chose atterrit.** Quatre renvois avaient
un « plus tard » sans référent ; six axes sortent du regard critique.

| Ce qui était renvoyé | Où |
| :------------------- | :- |
| une lecture décode le fichier deux fois | [#314](https://github.com/Guyot-Bertrand/sub-edit/issues/314) |
| `Encoding::name()` invente `UTF-16LE-sig` | [#315](https://github.com/Guyot-Bertrand/sub-edit/issues/315) |
| quatorze encodages au menu contre quatre-vingt-dix-sept | [#316](https://github.com/Guyot-Bertrand/sub-edit/issues/316) |
| `file.write-encoding` ne sert qu'au document sans fichier | [#317](https://github.com/Guyot-Bertrand/sub-edit/issues/317) |

| Ce que le regard critique a sorti | Où |
| :-------------------------------- | :- |
| un encodage dont le nom ne fixe pas l'ordre écrit sa marque, et `--no-bom` est désobéi | [#308](https://github.com/Guyot-Bertrand/sub-edit/issues/308) |
| un `Save As…` qui échoue déplace quand même le document | [#309](https://github.com/Guyot-Bertrand/sub-edit/issues/309) |
| la détection se trompe d'écriture sur un texte court | [#310](https://github.com/Guyot-Bertrand/sub-edit/issues/310) |
| personne ne rejoue le score de détection | [#311](https://github.com/Guyot-Bertrand/sub-edit/issues/311) |
| la langue des commentaires repose sur une mesure fausse | [#312](https://github.com/Guyot-Bertrand/sub-edit/issues/312) — **tranchée** : le C++ en anglais, un cliquet le tient, la traduction est [#325](https://github.com/Guyot-Bertrand/sub-edit/issues/325) |
| la fenêtre ne dit l'encodage que quand il a été deviné | [#313](https://github.com/Guyot-Bertrand/sub-edit/issues/313) |
| écrire en UTF-8 passe par une conversion qui ne fait rien — un tiers de plus, invisible pendant deux versions | [#318](https://github.com/Guyot-Bertrand/sub-edit/issues/318) |

**Le plus lourd est #312, et il corrige une issue de la relecture
précédente.** #273 avait retourné la règle de langue des commentaires en
concluant que « les commentaires de ce dépôt sont français partout, sans
exception ». Sa mesure cherchait les fichiers portant *au moins un* caractère
accentué, donc elle ne pouvait rendre que « il y a du français partout » — c'est
le défaut de #268, vérifier avec l'outil qui ne compte pas, commis une issue
après avoir été inscrit.

**Compté ligne à ligne, la frontière est celle du fichier** : 6 460 lignes de
commentaire anglaises contre 1 161 sur le C++, et l'inverse exact ailleurs —
1 971 françaises contre 6 sur les scripts, 386 contre 4 sur le système de
construction. La part de français dans les commentaires C++, par semaine
d'écriture, donne à la dérive une date nette : 0,0 %, 0,3 %, 2,7 %, puis 30,2 %
et 32,8 % — trois semaines d'anglais, puis les phases d'interface.

**Le C++ se commente donc en anglais**, un cliquet le tient
(`check-comment-language.py`, appelé par la porte), et les 1 161 lignes qui
restent sont l'issue [#325](https://github.com/Guyot-Bertrand/sub-edit/issues/325).

> **Tenu, relu en fin de phase 12.** #325 a été fermée le 2026-09-06 et le cliquet n'a pas bougé depuis : le
> C++ ne porte plus **aucune** ligne de commentaire française. Les 1 161 lignes ci-dessus sont celles du jour
> de la relecture, non un reste à traduire.

## 9 — Formats complémentaires et balises riches

SubViewer 2, Sub Station Alpha, Advanced SSA — les trois formats cités comme
secondaires — puis MicroDVD, MPL2, TMPlayer et LRC.

**Cadrée, et sa spec est dans [`specs/09-formats.md`](specs/09-formats.md)** —
issue #337. Trois ADR en sortent : les fins déduites pour les deux formats qui
n'en portent pas ([0029](adr/0029-fins-deduites-et-annoncees.md)), la fréquence
retenue par le document pour MicroDVD
([0030](adr/0030-ce-qu-un-document-retient-de-son-fichier.md)), et le pivot de
balises que l'ADR 0009 promettait depuis la phase 1
([0031](adr/0031-pivot-de-balises-a-la-conversion.md)).

**Le renvoi du cadrage de la phase 5 — le mode d'édition en images — repart vers
la phase 14**, mieux posé qu'il n'est arrivé. La phase 5 l'avait renvoyé faute
de format à images ; celui-ci arrive. Ce qui reste vrai est que MicroDVD porte
des numéros d'image **et pas de fréquence** : la bascule montrerait les chiffres
du fichier pour un MicroDVD, et une conversion contre une fréquence choisie pour
les huit autres.

**Point difficile** — ASS n'est pas un format de timing mais un format structuré
avec sections, styles nommés et événements typés. La conversion vers SubRip est
**structurellement à perte** : quelle politique de dégradation, et la
signale-t-on à l'utilisateur ? C'est ici que se vérifie la solidité du modèle de
balises conçu en phase 1.

## 10 — Opérations complémentaires

**Cadrée.** Voir [`specs/10-operations.md`](specs/10-operations.md), qui porte
les huit décisions et le découpage — et qui corrige deux affirmations de ce
cadrage-ci : le presse-papiers de Gaupol ne transporte que des textes, et son
menu n'offre qu'un seul basculement de style, l'italique.

Ajustement des durées, casse, italiques, tirets de dialogue, fusion, scission,
recherche et remplacement, presse-papiers.

**Un renvoi du cadrage de la phase 5 a atterri ici : l'édition de la durée dans
la table.** La colonne `Duration` s'affichait sans se saisir, faute d'une
`SetDurationCommand` au noyau. Elle est née en #381, et **elle déplace la fin** —
décision D3 de la spec.

**Deux renvois de la phase 16 y étaient posés, et un seul y a été tenu.**

- **Deux fixtures de grille qui manquaient**, écrites par l'issue #373 : des
  fins calculées par une règle de vitesse de lecture plutôt que posées sur une
  image, et un fichier écrit en millisecondes **sans aucune grille** —
  `grille-absurde.srt` étant régulier, le cas du bruit pur n'en avait aucune.
- **Retrouver la paire d'une conversion faite avec la mauvaise fréquence** a été
  **renvoyé à la phase 14**, par le cadrage de celle-ci — issue
  [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386). Rien de ce que
  la phase 10 apporte ne rend ce mécanisme plus sûr.

**Points difficiles**

- **Ajustement des durées** : contraintes simultanées de durée minimale,
  maximale, écart minimal entre sous-titres et vitesse de lecture en
  caractères par seconde. Elles sont **potentiellement contradictoires** ;
  l'ordre de résolution doit être spécifié.
- **Recherche dans du texte balisé** : chercher dans le texte visible tout en
  remplaçant dans le texte source, sans casser les balises qui chevauchent la
  correspondance.

## 11 — Traduction et multi-projets

Second document en regard du principal, alignement du fichier de traduction par
numéro ou par position, onglets, sauvegarde et fermeture groupées, scission d'un
projet, ajout d'un fichier à la suite d'un autre.

**Cadrée.** Voir [`specs/11-traduction.md`](specs/11-traduction.md), qui porte neuf
décisions, deux ADR ([0032](adr/0032-un-document-un-fichier.md), acceptée, et
[0033](adr/0033-un-projet-est-une-page.md), **acceptée**) et le découpage en **trois
tranches** — la traduction, l'ajout d'un fichier, le multi-projets. **La réserve
ci-dessous est levée** : la porte [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435)
a tranché pour le multi-projets le 2026-09-22, et la troisième tranche se livre.

**Un renvoi de la phase 6 atterrit ici : le lecteur pour un document de
traduction.** La réplique dessinée sur l'image vient du document principal, et
`Subtitle` porte les deux textes pour une seule paire de positions depuis la
phase 1 — il faudra donc dire lequel des deux s'affiche, et si le choix est un
réglage ou suit l'onglet actif. La relecture de fin de phase 6 a constaté que ce
renvoi ne tombait nulle part ; il tombe ici.

**Un renvoi de la phase 10 atterrit ici : la recherche au-delà du document
courant.** Gaupol cherche dans « tous les projets ouverts » et dans le texte de
traduction ; la phase 10 a réduit la portée à la cible habituelle — la sélection
ou le document — et au seul texte principal, faute d'un second projet et d'une
colonne de traduction à l'écran. Il faudra dire si la portée s'élargit avec les
onglets, et si le champ cherché suit la colonne visible.

**Réserve levée par #435, le 2026-09-22.** Le modèle de données de la phase 1
accueillait déjà le multi-projets sans l'engager ; la décision l'engage.

**Livrée, relue par [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441).** Les trois tranches,
et deux issues de plus : les colonnes masquables et réordonnables
([#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)), qu'aucune phase ne portait, et le
glisser-déposer ([#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453)), que #437 avait laissé.
**Les deux renvois ci-dessus sont tombés** : la réplique suit la colonne courante, sans réglage, et la
recherche porte au choix sur tous les projets ouverts, dans le texte de la colonne courante — D8 de la
spec. Ceux que la phase émet ont chacun leur destinataire : la ligne de commande de la traduction en
phase 13, la détection d'une traduction décalée en phase 14. **La relecture a ouvert quatre issues, #460
à #463**, qui tiennent la phase ouverte jusqu'à la clôture en 0.12.0.

**Close en 0.12.0, le 2026-09-26.** Après la relecture, deux vagues de plus, toutes deux fermées avant la
clôture :

- **les retours d'un usage sur un vrai bureau** — [#468](https://github.com/Guyot-Bertrand/sub-edit/issues/468)
  à [#474](https://github.com/Guyot-Bertrand/sub-edit/issues/474) et
  [#477](https://github.com/Guyot-Bertrand/sub-edit/issues/477) : un second film qui ne se chargeait pas,
  une image à zéro pixel, des erreurs X11 en quittant, la position de lecture par onglet, la croix et le
  « + » des onglets, la barre d'outils, le projet vierge remplacé à l'ouverture ;
- **l'allègement de `MainWindow`**, demandé à la fusion de #460 et analysé avant la clôture :
  [#483](https://github.com/Guyot-Bertrand/sub-edit/issues/483) à
  [#486](https://github.com/Guyot-Bertrand/sub-edit/issues/486) sortent les actions et les menus
  (`WindowActions`), la vidéo (`VideoPane`), la barre d'état (`StatusLine`) et les opérations de `Tools`
  (`ProjectOperations`, [ADR 0035](adr/0035-les-operations-sortent-de-la-fenetre.md)). `main_window.cpp`
  passe de 2 531 à 1 506 lignes, sans qu'un test de la fenêtre soit réécrit.

**Le banc de la phase** reste maigre : **cinq relevés pour trente-deux versions** — 0.11.1, 0.11.7,
0.11.21, 0.11.25 et 0.11.30 —, la machine ayant trouvé le seuil de charge dépassé toutes les autres fois.
Le relevé de 0.12.0 est dû à la clôture — issue [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270).

## 12 — Moteur de correction complet

Motifs déclaratifs par script, langue et pays — erreurs courantes classées
Humain et OCR, remise en majuscule, mentions pour malentendants restantes —
découpage de lignes et correcteur orthographique, avec la jonction et la
scission de mots qu'il permet — renvoi de la phase 10.

**Cadrée.** Voir [`specs/12-correction.md`](specs/12-correction.md), qui porte dix
décisions, deux ADR — [0036](adr/0036-icu-pour-les-motifs-de-correction.md), **ICU**
pour appliquer les motifs, et [0037](adr/0037-lire-les-motifs-de-gaupol-tels-quels.md),
les fichiers de Gaupol **lus tels quels** — et le découpage en **quatre tranches** : le
moteur, le découpage, l'assistant, le correcteur ([#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) à [#509](https://github.com/Guyot-Bertrand/sub-edit/issues/509)).
**Pas de porte** : aucune réserve n'est écrite sur le besoin, et le correcteur, le seul
morceau qui ajoute une dépendance, vient en dernier.

**La question du moteur, ci-dessous, est tranchée, et pas comme elle était posée.**
Le candidat retenu n'est ni PCRE2 ni RE2 mais ICU, déjà au noyau et déjà moteur de la
recherche. La mesure l'a mis à 0,13 s par film contre 0,05 s pour PCRE2 compilé à la
volée — une différence que personne ne verra —, et elle a trouvé plus important que le
temps : **ni l'un ni l'autre ne lit `\w` comme Python** dès qu'une lettre est
décomposée, PCRE2 depuis sa 10.43. La réécriture de `\w` est due quel que soit le
moteur, et l'argument de la compatibilité tombe avec elle. Le correcteur est **Enchant**,
et non hunspell : la liste de mots personnelle d'un utilisateur de Gaupol est la même.

**La phase 10 a laissé ici un parseur conscient des balises** —
`core/text/markup_parser` — que la casse, les tirets, la recherche et
l'ajustement des durées empruntent. Les motifs de correction s'appliqueront au
texte source et y rencontreront le défaut que l'ADR 0009 décrit ; le parseur est
la pièce faite pour l'éviter, et le retrait des mentions de la phase 4 ne passe
pas encore par lui.

**Questions d'architecture**

- **Moteur d'expressions régulières.** C'est ici que l'arbitrage se pose. Les
  motifs sont écrits en syntaxe Python et utilisent abondamment les **références
  arrière** (`\1 \2`), que RE2 ne gère pas ; PCRE2 est compatible mais peut
  exploser en temps sur certains motifs. Mesurer avant de trancher.

  **La phase 4 s'en est passée, et son [ADR 0017](adr/0017-analyseur-de-mentions-ecrit-a-la-main.md)
  est à lire avant de trancher ici.** Elle explique pourquoi un moteur n'aurait
  pas suffi pour ses deux motifs — la règle de couture est locale au site du
  retrait, une substitution est globale — et pose son propre déclencheur de
  réouverture : « le troisième motif demandé, quel qu'il soit ». Ce troisième
  motif, c'est cette phase-ci qui le demande.
- **Mesure de longueur de texte.** Point d'attention majeur : Gaupol mesure les
  lignes en *ems*, et le fait en demandant à **un widget GTK de mesurer le rendu
  du texte** (`gaupol/ruler.py`). L'algorithme de découpage dépend donc du
  toolkit. Chez nous, ce doit être une abstraction injectée : implémentation
  triviale par caractères pour la CLI et les tests, implémentation Qt pour
  l'interface.
- Correcteur orthographique : hunspell, nuspell, ou service système. **Tranché par la spec : Enchant**,
  dont la liste de mots personnelle est celle de Gaupol — D6.

**Point difficile** — le découpage de lignes est une variante de Knuth–Plass
avec boîtes, pénalités et démérites, où les pénalités viennent des motifs
`line-break` par langue. Coûteux, subjectif, et central dans la qualité perçue.
Appliquer des dizaines de motifs à des milliers de sous-titres est **le**
benchmark de référence du projet.

**Livrée, relue par [#523](https://github.com/Guyot-Bertrand/sub-edit/issues/523).** Douze issues, #498 à
#509, en quinze versions : les motifs de Gaupol lus tels quels et appliqués par ICU, les erreurs courantes, la
remise en majuscule, les mentions restantes, le découpage de lignes en caractères et en *ems*, l'assistant
`Correct Texts…` et sa confirmation, la jonction et la scission de mots, et `Check Spelling…`. **Les quatre
renvois qu'elle recevait sont tombés** : les trois motifs de mentions et le choix du moteur, de la phase 4 ;
PCRE2 ou RE2 et hunspell, de la phase 0 ; la jonction, la scission et les erreurs courantes, de la phase 10 ;
le parseur conscient des balises, que la phase 10 avait écrit pour cela. **Douze exigences, toutes
`implémentées` et citées** — quatorze à la clôture, avec `GUI-SPELL-04` et `GUI-EDIT-04` ; **vingt et un écarts avec Gaupol**, dont dix que le tableau de la spec ne portait
pas. Ceux que la phase émet ont chacun leur destinataire : la ligne de commande de la correction en
phase 13, la traduction des noms de motifs en phase 15. **Deux renvois restaient sans phase** ; la relecture les a soumis à décision, et ils sont livrés dans la phase : la vérification orthographique au fil de la frappe par [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525) (`GUI-SPELL-04`, treizième exigence), la longueur des lignes affichée dans les cellules par [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526). La spec en tient le compte, à la fin.

**Close en 0.13.0, le 2026-10-01.** La relecture a ouvert sept issues, toutes fermées avant la clôture :

- **les deux fonctions de Gaupol qu'elle avait renvoyées** — [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525),
  l'orthographe au fil de la frappe dans l'éditeur de cellule, et
  [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526), la longueur de chaque ligne dans les cellules et
  dans un liseré de l'éditeur (trois réglages `editor.*`, dans `Preferences…`) ;
- **les défauts qu'elle avait vus dans le code** — [#527](https://github.com/Guyot-Bertrand/sub-edit/issues/527),
  le cache de longueurs enfin posé devant l'assistant ;
  [#528](https://github.com/Guyot-Bertrand/sub-edit/issues/528), deux cases de la liste des motifs qui ne faisaient
  rien ; [#529](https://github.com/Guyot-Bertrand/sub-edit/issues/529), un nom de motif à virgule qui faisait perdre
  toutes les dérogations ; [#530](https://github.com/Guyot-Bertrand/sub-edit/issues/530), deux des quatre constats
  de robustesse de #509, les deux autres écartés avec leur raison ;
- **un défaut de GitHub, non du dépôt** — [#531](https://github.com/Guyot-Bertrand/sub-edit/issues/531) : des
  pull requests du 30 septembre n'ont pas lié leur `Closes #N`, puis cela a cessé sans qu'aucune différence de
  forme ait été trouvée. Fermée sans contrôle ajouté.

**Le banc de la phase**, comme celui de la précédente, est resté maigre : la machine a trouvé le seuil de
charge dépassé presque chaque fois. Le relevé de 0.13.0, pris au calme à la clôture, est versé au journal.

## 13 — CLI complète

Sous-commandes destinées à un usage réel : conversion, décalage, transformation,
alignement sur une grille, ajustement des durées, correction, inspection.
Traitement par lot, sortie lisible par un humain et sortie exploitable par un
script.

Gaupol n'a pas d'équivalent : c'est une conception neuve, et un gain
fonctionnel réel.

**Un renvoi de la phase 10 atterrit ici : recouper l'ajustement des durées avec
ce que le script de mesure en prédisait.**
[`measure-duration-constraints.py`](../src/scripts/measure-duration-constraints.py)
(issue [#371](https://github.com/Guyot-Bertrand/sub-edit/issues/371)) recense,
avant tout ajustement, les sous-titres pour lesquels aucune fin ne satisfait deux
des contraintes de durée ; `adjustDurations` déclare, lui, ce qu'il a sacrifié.
Le second mode qu'annonçait #371 — appliquer l'ajustement, puis compter — n'a
pas été écrit : un script Python ne peut pas appeler le noyau, la ligne de
commande n'expose pas l'ajustement avant cette phase, et un outil lié au noyau
ne servirait qu'à ce seul recoupement. **Une fois l'ajustement exposé**, on
relèvera par la ligne de commande les deux comptes sur les mêmes fichiers et les
mêmes réglages ; s'ils diffèrent, l'un des deux a tort. Le script compte, par
paire de contraintes, les sous-titres sans fin satisfaisante ; `adjustDurations`
compte, par contrainte, ceux où elle reste violée : dire quels comptes se
comparent fait partie du travail. Issue
[#407](https://github.com/Guyot-Bertrand/sub-edit/issues/407).

**Ce renvoi est tenu** ([#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560)). Les
comptes se comparent comme la spec D7 le dit désormais : `sacrificed.minimum` à « minimum contre
écart », `sacrificed.speed` à **l'union** de « vitesse contre écart » et de « vitesse contre
maximum », `sacrificed.gap` aux places négatives. La relecture a trouvé **deux erreurs du côté du
script, aucune du côté du noyau** : il comparait le besoin de lecture en flottants là où le noyau
l'arrondit à la milliseconde, et la place négative n'avait aucune colonne (le cadrage supposait à
tort qu'elle était « hors sujet »). La preuve est une fixture versionnée (`src/test/data/durees/`,
neuf fichiers) dont les comptes sont écrits à la main, et à laquelle le noyau (cas de bout en bout)
et le script (`--check-fixtures`, dans `make check-local`) sont confrontés. **Observation sur le
corpus privé**, rejouée avec `--crosscheck` : **les deux comptes s'accordent sur les soixante et onze
fichiers SubRip et WebVTT, sous quatre réglages différents** (les défauts de Gaupol avec un maximum
de 6 s, puis trois variantes de l'écart, du minimum, de la vitesse et du maximum). Rien n'a divergé,
donc le point ouvert 9 de la spec — savoir *quels* sous-titres sont sacrifiés — reste fermé.

**Un renvoi de la phase 11 atterrit ici : la traduction en ligne de commande.**
`-t/--translation-file` existe chez Gaupol, et la spec de la phase 4 avait renvoyé
à la phase 11 « nettoyer le document de traduction depuis la ligne de commande ».
Le cadrage de la phase 11 ([`specs/11-traduction.md`](specs/11-traduction.md), D9)
les donne à celle-ci : **rien de ce que la phase 11 écrit n'est propre à la
fenêtre** — l'ouverture d'une traduction, son alignement et la phrase du compte
rendu vivent au noyau et dans `core/wording/` —, et il ne reste ici qu'une
grammaire à écrire.

**Un renvoi de la phase 12 atterrit ici : la correction en ligne de commande.** Le calcul des changements, les
motifs, le correcteur et les phrases du compte rendu vivent au noyau ([`specs/12-correction.md`](specs/12-correction.md),
D8) ; il ne reste qu'une grammaire — `correct`, avec ses tâches, sa langue et ses réglages de découpage **en
caractères**, la mesure en *ems* étant celle de la fenêtre. La jonction et la scission de mots s'y joignent, le
correcteur étant au noyau et la dépendance à Enchant déjà tirée par `subedit-cli`. **La confirmation n'a pas
d'équivalent** : la spec laisse à cette phase de dire si un mode qui écrit les changements proposés sans les
appliquer la remplace.

**Cadrée par [#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542)** — la spec est
[`specs/13-cli.md`](specs/13-cli.md), et trois ADR en portent les décisions coûteuses à défaire :
[0038](adr/0038-sortie-json-lines-versionnee.md) (la sortie lisible par un script),
[0039](adr/0039-le-lot-collisions-dossiers-et-arborescence.md) (le lot) et
[0040](adr/0040-correct-ecrit-directement-dry-run-propose.md) (écrire directement, proposer par
`--dry-run`). Ce que le cadrage a tranché, et que la spec développe :

- **Une sortie lisible par un script : `--format json`**, opt-in, option globale. Sur la sortie standard,
  **un objet JSON par fichier d'entrée** (JSON Lines), échec compris, versionné par un champ `schema` ; le
  texte reste le défaut, la narration reste sur la sortie d'erreur. Elle se prouve par des **attendus
  versionnés**, écrits à la main, que `MatchesFile` (#543) compare octet pour octet. Aucun nombre à
  virgule : millisecondes entières, cadences en chaînes.
- **Un lot sûr.** Deux entrées de même destination, ou une destination qui est une entrée, sont **refusées
  avant tout écrit** (code `1`) ; le dossier de sortie est **créé** ; une destination existante reste
  **écrasée** — et le manuel le dit ; **`--recursive`** prend des répertoires et **conserve l'arborescence**
  sous `--output-dir`. Le libellé « cannot be read » d'une écriture qui échoue est corrigé.
- **`correct` écrit directement ; `--dry-run` propose.** Il n'écrit rien et imprime le texte avant et après,
  par sous-titre ; il est accepté par **toute** sous-commande qui écrit. C'est le « mode qui propose sans
  appliquer » que la phase 12 laissait ici : il **remplace** la page de confirmation.
- **Sept sous-commandes de plus** : `adjust`, `correct` (tâches nommées, découpage **en caractères**, jonction
  et scission de mots), `replace`, `case`, `italics`, `dialogue-dashes`, `sort` ; plus `--list-encodings` et
  la traduction comme options (`-t`, `--document`, `--align-method`). **Trois autres — `append`,
  `split-file`, `pair` — sont derrière une porte**, comme le multi-projets l'était en phase 11 : l'usage est
  réel, aucun appelant n'est connu. **Écartés, avec leur raison** : la fusion et la scission d'un
  sous-titre, le presse-papiers, l'édition à la main, la vérification orthographique interactive.
- **Une invocation se suffit** : aucun réglage n'est lu, ni la langue du système ; la langue, le code des
  motifs et la longueur des lignes **se nomment**. Seuls se lisent les motifs livrés, ceux de l'utilisateur
  et le dictionnaire d'Enchant.
- **Le renvoi de #407** est précisé : `sacrificed.minimum` se compare à « minimum contre écart »,
  `sacrificed.speed` à **l'union** de « vitesse contre écart » et de « vitesse contre maximum »,
  `sacrificed.gap` aux places négatives ; `adjust --format json --dry-run` expose les comptes et les
  contraintes employées pour que la comparaison se fasse, sur des fixtures versionnées d'abord.

**Vingt et une issues, en cinq tranches** — les fondations (le lot, le format, `--dry-run`, `--range`), les
sous-commandes dont le noyau est prêt, la traduction, la correction, la porte —, la relecture de fin de
phase comprise. **Un constat de l'analyse** : la résolution des emplacements des motifs, que l'ADR 0037
disait hors de `gui`, y est restée et appelle Qt ; la correction en ligne de commande commence par la
réécrire sans lui.

**Livrée, relue par [#573](https://github.com/Guyot-Bertrand/sub-edit/issues/573).** Vingt issues, #553 à #572, en
vingt-huit versions, de 0.13.1 à 0.13.28 : **dix-sept sous-commandes** — les sept de départ, plus `adjust`,
`replace`, `case`, `italics`, `dialogue-dashes`, `sort`, `correct` et, derrière la porte, `append`, `split-file`
et `pair` —, le lot sûr et `--recursive`, `--format json`, `--dry-run` sur toute sous-commande qui écrit,
`--range`, `-t`, `--document`, `--align-method` et `--list-encodings`. **Soixante-six exigences `CLI-*`, toutes
`implémentées` et citées par un test**, aucune `abandonnée` ; trois ADR ([0038](adr/0038-sortie-json-lines-versionnee.md),
[0039](adr/0039-le-lot-collisions-dossiers-et-arborescence.md),
[0040](adr/0040-correct-ecrit-directement-dry-run-propose.md)) ; **onze écarts avec Gaupol**, dont deux de plus qu'au
cadrage (`--list-encodings`, et les trois gestes de la fenêtre devenus sous-commandes). **La porte de D12 s'est ouverte
le 5 octobre 2026, pour les trois** : l'utilisateur les a voulues, une issue chacune, et chacune avec la grammaire que son
arité demande — N entrées une sortie, une entrée deux sorties, deux entrées une sortie. **Les onze points ouverts de la
spec sont tous tranchés**, trois d'entre eux à cette relecture (l'appariement dans un lot, `--range` sur les sept
d'origine, `--include`), chacun avec son déclencheur. **Le renvoi de #407 est tenu** (#560) : les deux comptes
s'accordent.

**La relecture n'a pas trouvé de défaut de fond, et a corrigé le manuel.** Le tableau de ce que la ligne de commande
ne fait pas disait « non » à cinq lignes de choses qu'elle fait ; `invocation.md` donnait trois valeurs pour le nombre des
sous-commandes qui écrivent et ne disait ni les `counts` d'`append`, de `split-file` et de `pair`, ni que ces trois
écrivent **un objet par lancement** et non par entrée ; six pages n'avaient pas de section JSON ; `lots.md` et la page
des plusieurs fichiers disaient « toutes ». **Le regard critique sur le code** a ouvert trois issues, dans le
milestone, rien n'ayant été corrigé dans la relecture :
[#598](https://github.com/Guyot-Bertrand/sub-edit/issues/598), une seule préparation des options pour dix-sept
sous-commandes (la validation y est écrite dix-sept fois, l'ouverture d'un fichier cinq fois, la garde « destination =
entrée » en deux libellés) ;
[#599](https://github.com/Guyot-Bertrand/sub-edit/issues/599), de la logique de décision restée dans `src/exe/cli`
(`pairingOf`, la préparation de `correct`, la liste des formats de `convert` qui double le noyau) ;
[#600](https://github.com/Guyot-Bertrand/sub-edit/issues/600), des tests qui promettent plus qu'ils ne prouvent
(`--dry-run` n'est éprouvé avec une destination à créer que pour six sous-commandes sur dix-sept). **`application.cpp` est
resté mince.** **La parité des phrases tient** : aucune phrase de `core/wording/` n'est recopiée dans `cli/`, et la phrase
de refus d'une coupure, que la fenêtre écrivait en dur, y est passée (#571).

**Le banc de la phase** est resté maigre, comme celui des deux précédentes : des relevés en 0.13.0, 0.13.10, 0.13.13,
0.13.14, 0.13.21, 0.13.25 et 0.13.27, et **aucun pour 0.13.28 ni pour la relecture** — la machine a trouvé le seuil de
charge dépassé. **Ce qui manque** : un banc propre à `append`, `split-file` et `pair`, qui n'en ont pas — ils reposent
sur des opérations du noyau déjà mesurées, et `make bench` ne mesure que le lot (parcours de deux cents fichiers,
décalage d'un fichier de quatre mille sous-titres). Rejouer `make bench` au calme reste dû.

**Close en 0.14.0, le 2026-10-06.** Les trois issues de la relecture ont été livrées, une de plus avec elles, et le
milestone — trente-deux issues, de #541 à #605 — se ferme sans rien d'ouvert :

- **#598** (0.13.30) : une seule préparation des options, `prepare` et `prepareWriting` ; l'ouverture d'un fichier,
  la narration d'un fichier réécrit et la garde « destination = entrée » ne sont plus écrites cinq, quatre et deux
  fois ; **un seul libellé** pour la destination qui est une entrée, qui ne renvoie plus à une option que `append`
  et `split-file` n'ont pas ;
- **#599** (0.13.31) : ce que `src/exe/cli` décidait seul — les options partagées et la règle de `-t`, la préparation
  de `correct`, la forme d'une conversion, le montant d'un décalage, la liste des formats de `--to`, dérivée du noyau
  — vit dans `subedit/cli`, où des tests unitaires l'atteignent ;
- **#600** (0.13.32) : `--dry-run` éprouvé pour les seize sous-commandes qui écrivent et non six, et les tests qui
  promettaient « rien n'est écrit » l'observent ;
- **#605** (0.13.33), **ouverte par un retour d'usage** et non par la relecture : le panneau des diagnostics de
  lecture, qui prenait une rangée de la fenêtre pour une information rare, devient un bouton de la barre d'état et
  une liste flottante. En retirant la rangée, un test a montré un défaut ancien — la part de la table, lue et posée
  en arrondissant vers le bas, redescendait d'un point à chaque relecture —, corrigé et prouvé sur huit hauteurs.

**Le banc de la phase** n'a pas de relevé pour 0.14.0 : la machine a trouvé le seuil de charge dépassé, et la
mesure de la clôture n'a pas été reprise. Le dernier relevé versé est celui de 0.13.33. **Ce qui reste dû** est
dit plus haut : un banc propre à `append`, `split-file` et `pair`, et un `make bench` au calme.

## 14 — Calage fin

**Réduite, mais pas vidée.** Le lecteur lui-même est passé en phase 6, qui l'a
repris au cadrage : associer un film, l'ouvrir dans la fenêtre, jouer, s'arrêter,
chercher, dessiner la réplique sur l'image. Ce qui reste ici est ce que la
phase 6 ne fait pas — et cela va au-delà du calage proprement dit, parce que
**la phase 6 ne livre du pilotage que jouer et s'arrêter.**

Le contour a été recompté contre le menu **Video** de Gaupol et sa barre de
lecteur, poste par poste, après une relecture qui a montré que la moitié
manquait au cadrage. Trois familles, et rien d'autre.

**Le pilotage de la lecture.** Ce qu'une barre de lecteur porte, et ce que la
phase 6 laisse à qui n'a que `Ctrl+P` :

| Commande | Ce qu'elle fait | Chez Gaupol |
| :------- | :-------------- | :---------- |
| position | une barre qu'on lit et qu'on déplace | `Gtk.Scale` de la barre de lecteur |
| saut avant, saut arrière | d'un pas réglable | `Seek Forward`, `Seek Backward` |
| sous-titre précédent, suivant | se placer à son début | `Seek Previous`, `Seek Next` |
| début, fin de la sélection | s'y placer, avec un peu d'avance | `Seek Selection Start`, `Seek Selection End` |
| jouer la sélection | et s'arrêter au bout | `Play Selection` |
| volume | monter, baisser, un réglage | `Volume Up`, `Volume Down`, `Gtk.VolumeButton` |

**Le calage lui-même**, qui est le cœur du travail de *timing* :

| Commande | Ce qu'elle fait |
| :------- | :-------------- |
| début, fin depuis la position vidéo | poser un repère sur ce qu'on regarde |
| insérer un sous-titre à la position vidéo | idem, pour une réplique qui n'existe pas encore |
| avancer, reculer image par image | ce pour quoi l'ADR 0020 a choisi libmpv |
| décaler début ou fin par petits incréments | le réglage fin, au clavier |

**Ce qui s'affiche** : incrustation du timecode, sélection de piste audio, et
**la réplique dessinée sans ses balises brutes** — renvoi de la relecture de fin
de phase 10, issue [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408).
Le manuel du lecteur le promettait « avec les formats riches », une phase déjà
passée ; le parseur et le pivot existent désormais, et rien du noyau ne manque.

**Ce que la phase 6 a déjà livré, et qui n'est donc pas ici** : associer un
film, l'ouvrir dans la fenêtre, jouer et s'arrêter, la réplique dessinée depuis
le modèle, la ligne courante qui suit la lecture, la recherche exacte au
sous-titre sélectionné.

**Le backend est décidé** — libmpv, voir
[l'ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md). Et c'est cette phase
qui l'a décidé : les deux candidats savaient servir la phase 6, seul libmpv sait
avancer d'une image.

**Un renvoi du cadrage de la phase 9 atterrit ici : le mode d'édition en
images.** Il vient de la phase 5, qui l'avait renvoyé faute de format à images,
et la phase 9 le repasse en l'ayant mieux posé — MicroDVD porte des numéros
d'image et pas de fréquence, donc la bascule montrerait les chiffres du fichier
pour lui, et une conversion contre une fréquence choisie pour les huit autres.
C'est une question d'édition fine, et cette phase porte déjà « avancer, reculer
image par image ».

**Un renvoi de la phase 10, venu de la phase 16, atterrit ici : retrouver la
paire d'une conversion faite à la mauvaise fréquence** — issue
[#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386). Quand la grille
déduite n'est aucune des huit, c'est parfois qu'une conversion a été faite avec
une fréquence d'entrée fausse, et le rationnel qui remettrait le fichier sur une
grille normalisée se cherche. La spec de la phase 16 l'a nommé « le plus utile
et le moins sûr » des trois mécanismes de correction ; ni jusqu'où chercher, ni
comment dire sa confiance n'ont encore de réponse.

**Un renvoi de la phase 11 atterrit ici : reconnaître une traduction décalée.**
Une traduction calée deux secondes trop tard s'aligne sur les mauvais sous-titres
sans que rien la distingue d'un rattachement juste — le cas `positions-decalees`
de `src/test/data/paires/` le montre. Gaupol ne le détecte pas non plus. La phase
11 rend le décalage **visible**, par le compte de l'ouverture (trois lignes sur
quatre y font naître un sous-titre), pas **corrigeable** : c'est une question de
synchronisation, et elle se pose ici.

**Précisé par l'utilisateur, le 2026-09-30 : la table suit la lecture, mais ne
se centre pas.** Le sous-titre courant est mis en évidence pendant la lecture, et
la table défile bien jusqu'à lui — une première observation disait le contraire,
corrigée depuis. Ce qui reste : **la table ne se centre pas sur le sous-titre
courant**, elle le ramène seulement dans la zone visible. La phase 14 le centre.
Le point à trancher au cadrage est le geste de l'utilisateur — un défilement à la
main pendant la lecture ne doit pas être défait à chaque image, et il faut dire
quand le suivi reprend.

**Point difficile** — **précision de positionnement.** Caler un sous-titre exige
un `seek` exact à l'image près ; la plupart des backends ne le garantissent qu'au
mot-clé le plus proche. C'est la fonctionnalité la plus exigeante de tout le
projet, et celle qui décide de la qualité de l'outil pour le travail de timing.
La phase 6 pose déjà la recherche exacte, ce qui la rend éprouvable avant
d'arriver ici.

## 15 — Internationalisation

Les 20 locales de Gaupol sont sous GPL, donc réutilisables.

**Question ouverte** — Qt Linguist ou gettext ? La conversion `.po` vers `.ts`
n'est fidèle que si les chaînes correspondent, ce qui ne sera pas le cas
partout. Évaluer le gain réel avant de s'engager.

**Un renvoi de la phase 12 atterrit ici : les noms et les descriptions des motifs de correction.** Gaupol les
traduit par gettext ; les fichiers de motifs sont lus tels quels
([ADR 0037](adr/0037-lire-les-motifs-de-gaupol-tels-quels.md)) et leurs intitulés restent en anglais tant que
l'interface l'est. Ils passent avec le reste de l'interface, et la question de la conversion `.po` vers `.ts`
s'y pose pour eux aussi.

**Précisé par l'utilisateur, le 2026-09-30 : la documentation embarquée passe ici,
internationalisée.** Deux livrables, tous deux dans les langues de l'interface :

- **La page d'aide de la fenêtre.** Le manuel que `Help` ouvre aujourd'hui
  (`docs/manual/`, en français) devient une aide **internationalisée** : elle
  explique les diverses fonctions que le programme offre, et suit la langue
  choisie comme le reste de l'interface. Ce que le manuel dit déjà en est la
  matière ; ce que la phase change est qu'il se traduit avec l'interface, et non
  après elle.
- **Des pages de manuel (`man`) pour la ligne de commande**, elles aussi
  internationalisées : une page pour `subedit-cli` et une pour chaque
  sous-commande dont l'usage l'exige, installées avec les paquets (`.deb`,
  `.rpm`) là où `man` les cherche.

Au cadrage : d'où viennent les pages (une source unique, le manuel actuel, dont
on tire l'aide de la fenêtre et les pages `man`, ou deux sources), comment leur
traduction se tient à jour quand le manuel bouge, et si les captures du manuel se
prennent par langue. **Une seule source** est la voie qui évite que deux textes
divergent ; c'est aussi la plus coûteuse à outiller.

# Troisième partie — au-delà de Gaupol

Ces phases ne sont plus de l'iso-fonctionnalité : elles ajoutent ce que Gaupol
ne fait pas. Elles sont nées d'un échange de cadrage, le 2026-10-06, et **leur
ordre est celui que l'utilisateur a choisi** ; il ne tient pas à une prévision
d'effort.

**Public visé : des amateurs de séries et de films qui cherchent à automatiser
leur workflow.** La ligne de commande est donc de rang égal à l'interface, et
« je lance une commande et le sous-titre tombe juste » est le critère à tenir.

**Ces phases sont des brouillons.** Chacune sera affinée à son cadrage — c'est
là que se tranchent le périmètre exact, les exigences et le découpage en
issues. Ce qui suit fixe l'ordre, l'intention et les décisions à prévoir, rien
de plus.

```
17 ─► 18 ─► 19 ─► 20 ─► 21
                   │
                   └────────► 22 ─► 23 ─► 24
```

**Ce que les phases se transmettent.** La 19 pose le mécanisme de sous-processus
et de détection d'un outil externe, que 21, 22 et 24 réutilisent. La 20 pose
l'OCR, que la 21 branche sur les conteneurs. La 22 pose le moteur audio, que la
23 réutilise pour aligner.

**Une règle pour tous les outils externes** (`mkvmerge`, `ffmpeg`, Tesseract,
whisper.cpp), dans le prolongement de la tolérance à l'absence de `ffprobe` :
**leur absence ne casse rien.** La fonction qui en dépend se désactive avec un
message clair, ou l'utilisateur fournit la donnée lui-même. Aucun de ces outils
n'est livré avec les paquets.

## 17 — Formats texte complémentaires

Les formats de sous-titres texte que ni Gaupol ni les phases 1 et 9 ne
couvrent. La liste est à établir au cadrage, selon l'usage réel du public visé.

Chaque format déplace les trois mesures habituelles de la phase 9 : une ligne de
la table de promesses, le score de détection, la matrice de conversion.

## 18 — Formats texte avancés

Styles nommés, positionnement, karaoké : lecture, écriture et conversion entre
les formats qui les portent (ASS, TTML, WebVTT…).

**C'est la phase qui touche le plus de code existant.** Le modèle d'entrée est
aujourd'hui du texte balisé ; il doit accueillir des styles, une position par
entrée et des segments minutés. Cela traverse le noyau et chaque format déjà
écrit, et appelle un ADR.

**À trancher au cadrage :**

- la forme du modèle de style et de position, et sa place dans le noyau ;
- ce que devient une information que le format cible ne porte pas : perte
  signalée, refus, ou repli — dans le prolongement de la matrice de conversion ;
- ce que l'interface en montre et en permet d'éditer, ou si cette phase se
  limite d'abord à la bibliothèque et à la ligne de commande.

**Phase lourde, à scinder au cadrage** si la mesure le demande.

## 19 — Pistes texte embarquées

Lister, extraire et injecter des pistes de sous-titres texte dans les
conteneurs MKV et MP4, avec leurs métadonnées : langue, drapeaux « forcé » et
« malentendants », piste par défaut.

**À trancher au cadrage — et la décision vaut pour les phases 21, 22 et 24 :**
lier une bibliothèque (libavformat) ou appeler `mkvmerge` / `ffmpeg` en
sous-processus. **Le sous-processus est la pente attendue** : il garde les
exécutables minimaux et laisse l'utilisateur maître de ses outils. Il suppose
la détection de la présence de l'outil, et sa tolérance à l'absence.

Autres questions : ouvrir directement un conteneur depuis la fenêtre et la
ligne de commande, ou seulement extraire ; réécrire le conteneur (remux) ou
exporter à côté.

## 20 — Formats image et OCR

Lire les sous-titres image — **VobSub** (`.idx` / `.sub`), **PGS** (`.sup`,
Blu-ray) et **DVB-sub** (flux TV, `.ts` enregistrés) —, les passer à
l'OCR, et produire des sous-titres texte.

**Deux parties, qui ne vont pas du même pas :** la lecture des formats image,
qui est de l'analyse binaire et se teste sur fixtures, et l'OCR proprement dit.

**À trancher au cadrage :**

- le moteur d'OCR — Tesseract en bibliothèque ou en exécutable — et la
  provenance de ses données de langue, que l'utilisateur télécharge ;
- **l'interface de correction humaine**, qui n'est pas un détail : un OCR se
  relit. Elle s'appuie sur les motifs « OCR » du moteur de correction de la
  [phase 12](#12--moteur-de-correction-complet) ;
- la version en ligne de commande : OCR sans relecture, avec un compte rendu
  des entrées douteuses.

**Phase lourde, à scinder au cadrage** : lecture des formats d'un côté, OCR et
interface de relecture de l'autre.

## 21 — Pistes image embarquées

Extraire les pistes image des conteneurs et les chaîner à l'OCR de la phase 20,
de bout en bout : du fichier vidéo aux sous-titres texte.

Elle n'ajoute presque rien de neuf — c'est la jonction de la 19 et de la 20 —, et
c'est précisément ce qui la rend courte.

## 22 — Transcription audio

Produire des sous-titres, avec leurs positions, à partir de l'audio d'une vidéo.

**Moteur pressenti : whisper.cpp**, qui s'exécute sur un processeur ordinaire,
sans carte graphique ; celle-ci n'accélère que le traitement. Les modèles pèsent
de quelques centaines de Mo à plusieurs Go : **ils ne sont pas livrés**, et
l'utilisateur les récupère, par une commande dédiée ou un chemin à renseigner.

**À trancher au cadrage :**

- exécutable en sous-processus ou bibliothèque — même pente que pour la
  phase 19 : l'exécutable, qui laisse le choix de la variante matérielle ;
- le build livré dans les paquets : processeur seul par défaut ;
- la sortie : phrases minutées, ou mots minutés, que la phase 23 réutilise ;
- la détection de la langue de l'audio.

## 23 — Alignement automatique

Recaler un sous-titre existant sur l'audio : à partir du texte et de la voix,
retrouver où chaque phrase est réellement prononcée. C'est le cas du `.srt`
récupéré en ligne, décalé au début et en fin, que la correction par deux points
ne rattrape pas.

**Elle dépend de la phase 22**, dont elle réutilise le moteur et les mots
minutés, et reste optionnelle comme elle. Elle est livrée en ligne de commande
et dans l'interface.

**À trancher au cadrage :** le comportement quand l'alignement est douteux
(entrées non retrouvées, dérive progressive) et la façon de le rendre à
l'utilisateur.

## 24 — Workflow et intégrations

Ce qui permet d'enchaîner les phases précédentes sans les rejouer à la main :
traitement par lots, dossier surveillé, chaînage de commandes, sortie JSON
stable pour les scripts.

**Phase à cadrer en dernier**, parce qu'elle automatise ce que les autres
livrent : son périmètre exact dépend de ce que l'usage aura montré pendant les
phases 17 à 23. Le `--recursive` de la [phase 13](#13--cli-complète) en est le
point de départ.

## Pour une v3

Retenu mais écarté pour l'instant, le 2026-10-06 :

- **le mode serveur** (`subedit serve`) ;
- **la forme d'onde interactive** — le dessin du son sous la vidéo, avec des
  blocs de sous-titres qu'on tire à la souris pour les caler. L'alignement
  automatique de la phase 23 couvre l'essentiel du besoin sans elle ;
- **les sous-titres incrustés dans l'image** (« hardsubs »), par OCR sur la
  vidéo ;
- **la traduction automatique** et la **qualité** : conformité aux normes de
  diffuseurs, mémoire de traduction, fusion de versions ;
- **la détection de coupures de plan.**
