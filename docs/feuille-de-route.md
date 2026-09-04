# Feuille de route

Dix-sept phases, chacune associée à un milestone GitHub. Neuf mènent à un
**MVP livrable** — les huit premières, plus la [16](#16--fréquences-dimage--déduction-et-correction)
intercalée avant la 7 ; les autres complètent l'iso-fonctionnalité avec Gaupol.

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
seule à le faire, et la seule à dépasser l'iso-fonctionnalité avec Gaupol.

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
  retenu : `src/scripts/subtitle-fixtures.py` engendre treize fichiers sur des
  grilles connues — les huit fréquences normalisées, une fréquence absurde, une
  grille décalée, une étendue insuffisante, et les deux visages du fichier
  partiel. `src/test/data/grilles/LISEZMOI.md` porte ce que chacun est et ce que
  chacun donne.

  Ce que ces fixtures ne portent pas encore, et qu'il faudra leur ajouter : des
  fins calculées par une règle de vitesse de lecture plutôt que posées sur une
  image, et un fichier écrit en millisecondes sans aucune grille.
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
| la langue des commentaires repose sur une mesure fausse | [#312](https://github.com/Guyot-Bertrand/sub-edit/issues/312) |
| la fenêtre ne dit l'encodage que quand il a été deviné | [#313](https://github.com/Guyot-Bertrand/sub-edit/issues/313) |
| écrire en UTF-8 passe par une conversion qui ne fait rien — un tiers de plus, invisible pendant deux versions | [#318](https://github.com/Guyot-Bertrand/sub-edit/issues/318) |

**Le dernier est le plus lourd, et il corrige une issue de la relecture
précédente.** #273 avait retourné la règle de langue des commentaires en
concluant que « les commentaires de ce dépôt sont français partout, sans
exception ». Compté : **6 339 lignes anglaises contre 1 142 françaises** sur
`src/`. La mesure de #273 cherchait les fichiers portant *au moins un* caractère
accentué, donc elle ne pouvait rendre que « il y a du français partout » — c'est
le défaut de #268, vérifier avec l'outil qui ne compte pas, commis une issue
après avoir été inscrit.

## 9 — Formats complémentaires et balises riches

SubViewer 2, Sub Station Alpha, Advanced SSA — les trois formats cités comme
secondaires — puis MicroDVD, MPL2, TMPlayer et LRC.

**Un renvoi du cadrage de la phase 5 atterrit ici : le mode d'édition en
images.** Gaupol laisse basculer la table entre positions temporelles et numéros
d'image ; la phase 5 s'en est passée pour une raison qui tombe précisément
ici — **aucun format à images n'existe avant MicroDVD**, et les deux formats du
MVP sont temporels. Une bascule vers des images qu'aucun fichier ne porte
n'aurait affiché qu'une conversion, calculée contre une fréquence que rien ne
déclare. Le premier format à images arrive dans cette phase, et avec lui la
question de ce que la table montre.

**Point difficile** — ASS n'est pas un format de timing mais un format structuré
avec sections, styles nommés et événements typés. La conversion vers SubRip est
**structurellement à perte** : quelle politique de dégradation, et la
signale-t-on à l'utilisateur ? C'est ici que se vérifie la solidité du modèle de
balises conçu en phase 1.

## 10 — Opérations complémentaires

Ajustement des durées, casse, italiques, tirets de dialogue, fusion, scission,
recherche et remplacement, presse-papiers.

**Un renvoi du cadrage de la phase 5 atterrit ici : l'édition de la durée dans
la table.** La colonne `Duration` s'affiche et ne se saisit pas, parce que le
noyau n'a pas de `SetDurationCommand` — la phase 2 s'est arrêtée à « un texte,
un début, une fin » — et qu'en inventer une dans une phase d'interface aurait
été du travail de noyau glissé ailleurs. La commande naît ici, où l'ajustement
des durées en a besoin de toute façon ; la colonne devient éditable du même
coup, et il faudra dire laquelle des deux bornes elle déplace.

**Deux renvois de la phase 16 atterrissent ici.**

- **Retrouver la paire d'une conversion faite avec la mauvaise fréquence.** La
  phase 16 déduit la grille d'un fichier ; quand celle-ci n'est aucune des huit,
  c'est parfois qu'une conversion a été faite avec une fréquence d'entrée
  fausse, et le rationnel qui remettrait le fichier sur une grille normalisée se
  cherche. Sa spec l'a nommée « le plus utile et le moins sûr » des trois
  mécanismes de correction, et l'a écartée pour cette raison.
- **Deux fixtures de grille qui manquent** :
  `src/test/data/grilles/LISEZMOI.md` le dit lui-même — des fins calculées par
  une règle de vitesse de lecture plutôt que posées sur une image, et un fichier
  écrit en millisecondes **sans aucune grille**. `grille-absurde.srt` est
  régulier ; le cas du bruit pur n'a donc aucune fixture.

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

**Un renvoi de la phase 6 atterrit ici : le lecteur pour un document de
traduction.** La réplique dessinée sur l'image vient du document principal, et
`Subtitle` porte les deux textes pour une seule paire de positions depuis la
phase 1 — il faudra donc dire lequel des deux s'affiche, et si le choix est un
réglage ou suit l'onglet actif. La relecture de fin de phase 6 a constaté que ce
renvoi ne tombait nulle part ; il tombe ici.

**Réserve** — le besoin n'est pas confirmé. Le modèle de données de la phase 1
l'accueille ; cette phase construit l'interface et les opérations associées.

## 12 — Moteur de correction complet

Motifs déclaratifs par script, langue et pays — erreurs courantes classées
Humain et OCR, remise en majuscule, mentions pour malentendants restantes —
découpage de lignes et correcteur orthographique.

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
- Correcteur orthographique : hunspell, nuspell, ou service système.

**Point difficile** — le découpage de lignes est une variante de Knuth–Plass
avec boîtes, pénalités et démérites, où les pénalités viennent des motifs
`line-break` par langue. Coûteux, subjectif, et central dans la qualité perçue.
Appliquer des dizaines de motifs à des milliers de sous-titres est **le**
benchmark de référence du projet.

## 13 — CLI complète

Sous-commandes destinées à un usage réel : conversion, décalage, transformation,
alignement sur une grille, ajustement des durées, correction, inspection.
Traitement par lot, sortie lisible par un humain et sortie exploitable par un
script.

Gaupol n'a pas d'équivalent : c'est une conception neuve, et un gain
fonctionnel réel.

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

**Ce qui s'affiche** : incrustation du timecode, sélection de piste audio.

**Ce que la phase 6 a déjà livré, et qui n'est donc pas ici** : associer un
film, l'ouvrir dans la fenêtre, jouer et s'arrêter, la réplique dessinée depuis
le modèle, la ligne courante qui suit la lecture, la recherche exacte au
sous-titre sélectionné.

**Le backend est décidé** — libmpv, voir
[l'ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md). Et c'est cette phase
qui l'a décidé : les deux candidats savaient servir la phase 6, seul libmpv sait
avancer d'une image.

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
