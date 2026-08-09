# Feuille de route

Quatorze phases, chacune associée à un milestone GitHub. Les huit premières
mènent à un **MVP livrable** ; les suivantes complètent l'iso-fonctionnalité
avec Gaupol.

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
  seule entrée d'annulation ?
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

**Fortement restreinte.** La ligne de commande n'apparaît pas dans les besoins
de l'utilisateur : elle sert ici de **harnais de validation et de mesure** du
noyau, avant qu'il existe une fenêtre.

Périmètre : inspection d'un fichier, conversion entre les deux formats du MVP,
décalage. Les sous-commandes destinées à un usage réel relèvent de la phase 13.

**Analyse préalable** — pas de source à reprendre : Gaupol n'a pas d'équivalent.
`bin/gaupol.in` montre les options existantes de l'application.

**Questions d'architecture**

- Bibliothèque d'analyse d'arguments : CLI11, cxxopts, ou implémentation propre.
  Décision par ADR — c'est une dépendance de plus.
- Codes de retour signifiants, et comportement en cas d'échec partiel sur un
  lot.
- **Ce que `Project::outOfOrder()` doit rendre — à ré-évaluer ici.** La phase 2
  l'a implémenté en comparant chaque sous-titre à son **prédécesseur immédiat**,
  ce que dit la spec. L'autre lecture — comparer au plus grand début rencontré
  jusque-là — rend un ensemble différent : sur les départs `0, 4000, 2000, 3000`,
  la première rend `{2}`, la seconde `{2, 3}`.

  Les deux s'accordent toujours sur le fait qu'il y a du désordre ou non : une
  suite dont chaque élément suit son prédécesseur est croissante, donc chaque
  élément suit aussi tous les précédents. Elles ne diffèrent que sur **la liste**.
  Rien avant l'inspection ne consomme cette liste — la politique stricte de la
  phase 2 se déclenche sur le `CommandKind`, pas sur une requête de désordre, et
  ses tests ne regardent que le vide ou le non-vide.

  L'inspection est donc le premier appelant à devoir trancher : « les lignes qui
  rompent l'ordre » ou « les lignes à déplacer pour rétablir l'ordre ». La
  phase 5 affichera le même ensemble et suivra ce choix.

---

## 4 — Suppression des mentions pour malentendants

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

**Questions d'architecture**

- Reprendre le format INI des motifs de Gaupol tel quel — ce qui permet de
  réutiliser ses fichiers sans conversion et de bénéficier de leurs mises à
  jour — ou définir le nôtre ? Deux détails de compatibilité à ne pas découvrir
  tard : chaque fichier de motifs est accompagné d'un `.conf` **XML** qui active
  ou désactive les motifs par nom, et un `\0` en tête de valeur sert à protéger
  les espaces initiaux (`patternman.py`).
- Comment le moteur de motifs est-il conçu pour que la phase 12 l'étende sans le
  reprendre ?

**Point difficile**

Supprimer `[Bruit de pas]` laisse une ligne vide, ou une ligne réduite à un
espace. Gaupol traite ce nettoyage à part (option `remove_blank`). Le
comportement attendu se spécifie, il ne s'improvise pas.

---

## 5 — Interface : édition tabulaire

Le cœur de l'usage : ouvrir, éditer dans la table, enregistrer sous, annuler.

**Restreint au contour du MVP :** une seule fenêtre, un seul projet à la fois,
colonnes numéro / début / fin / durée / texte, dialogues des opérations des
phases 2 et 4. Sont reportés : multi-projets en onglets, colonne de traduction,
colonnes configurables, coloration des différences.

**Analyse préalable** — `gaupol/` : `view.py`, `page.py`, `application.py`,
`renderers/*.py`, `dialogs/{open,save,position_shift,position_transform,framerate_convert}.py`.

**Questions d'architecture**

- `QAbstractTableModel` au-dessus du modèle du noyau : adaptateur mince, ou
  modèle propre synchronisé ? Le premier évite la duplication d'état, le second
  découple mais impose une synchronisation.
- **Ne pas dupliquer la pile d'annulation.** Qt propose `QUndoStack`. Le noyau a
  la sienne, et c'est elle qui fait autorité puisque la CLI en dépend aussi.
  L'interface doit s'y brancher, pas en tenir une seconde.
- Configuration typée et persistée, en remplacement du dictionnaire imbriqué de
  Gaupol. Format de fichier et stratégie de migration entre versions.
- Édition en place de texte multiligne dans une cellule.
- **Un changement de fréquence d'image n'est déclaré par aucun `ChangeKind` — à
  ré-évaluer ici.** La phase 2 a écarté d'en ajouter un : la fréquence n'est la
  propriété d'aucun sous-titre, et un énumérateur que personne ne lit est une
  promesse sans garant, règle que `DiagnosticKind` énonce déjà. `describe()` d'une
  conversion ne rapporte donc que les positions. Si la fenêtre affiche la
  fréquence courante, elle aura besoin de savoir qu'elle a changé — et ce sera
  alors un énumérateur avec un lecteur, ce qui lève l'objection.
- **D'où vient la fréquence d'entrée d'une conversion ?** Question ouverte, et
  elle se pose ici parce que c'est ici qu'un dialogue la demandera.

  **Le fichier ne la porte pas, et ne peut pas la porter.** SubRip n'a aucun
  en-tête ; l'en-tête WebVTT est du texte libre. Les deux formats du MVP sont
  temporels : une fréquence d'image n'y a pas de place. Seuls les formats
  *à images* — MicroDVD, phase 9 — en déclarent une, sur leur première ligne.

  **Le nom du fichier, lui, reste à examiner — et le corpus local ne peut pas le
  dire.** Les quinze fichiers de `src/data/` ont été renommés à la main avant
  d'entrer dans le dépôt : ils portent le titre, parfois l'édition (`Alien.DC`),
  parfois la langue (`eng`, `ger`, `fr`), mais ces noms sont le produit d'un
  nettoyage et non une observation. **Trancher cette question demande des
  fichiers neufs, aux noms intacts.**

  Ce qu'on peut déjà dire sans eux : les noms de publication en ligne portent
  `DVD`, `BluRay`, `1080p`, qui *corrèlent* avec 25 et 23,976 sans les nommer.
  Une corrélation n'est pas une donnée, et se tromper de fréquence décale tout le
  fichier sans rien signaler — le pire mode d'échec possible pour cette
  opération : silencieux et global. Une heuristique de nom ne peut donc être
  qu'une **proposition montrée comme telle**, jamais un choix appliqué en
  silence.

  **La piste sérieuse est la vidéo elle-même.** Son conteneur déclare sa
  fréquence : ce n'est pas une corrélation mais la donnée, lue à la source. La
  phase 6 associe déjà une vidéo au projet pour la prévisualisation, et la
  phase 14 en ouvre une pour de bon — l'information est donc à portée avant même
  le lecteur intégré. Voir la question posée à la [phase 6](#6--prévisualisation).

  Reste l'utilisateur pour les cas sans vidéo, et un défaut tiré du nom **montré
  comme une proposition**. À trancher au cadrage — après avoir regardé des noms
  de fichiers intacts, et vérifié ce que coûte la lecture des métadonnées.

**Point difficile**

Rester fluide sur plusieurs milliers de lignes : le modèle ne doit jamais
matérialiser ce qui n'est pas visible, et les signaux de modification doivent
être fins plutôt que globaux. C'est l'objectif de performance du projet, à
l'endroit où l'utilisateur le perçoit.

---

## 6 — Prévisualisation

**Re-cadrée.** Le lecteur vidéo intégré et le calage image par image
n'apparaissent pas dans les priorités et constituent la partie la plus coûteuse
du projet ; ils passent en phase 14.

Reste ce qui sert directement les opérations prioritaires : après un décalage ou
une transformation, **vérifier le résultat**. Gaupol le fait en écrivant un
fichier temporaire et en lançant un lecteur externe — mpv, VLC ou MPlayer —
positionné au sous-titre courant.

**Analyse préalable** — `aeidon/agents/preview.py`, `aeidon/enums.py` (les
commandes des trois lecteurs), `gaupol/agents/preview.py`.

**Questions d'architecture**

- Détection du lecteur disponible, et commande personnalisable.
- Association d'un fichier vidéo à un projet : par convention de nom, comme
  Gaupol (`find_video`), ou choix explicite ?
- **Lire la fréquence d'image dans la vidéo associée ?** La conversion de
  fréquence de la phase 2 a besoin d'une fréquence d'entrée que le fichier de
  sous-titres ne porte pas et ne peut pas porter — voir la question posée à la
  [phase 5](#5--interface--édition-tabulaire). Le conteneur vidéo, lui, la
  déclare : c'est la donnée et non une corrélation. Une vidéo est déjà associée
  au projet ici, avant le lecteur intégré de la phase 14.

  À vérifier au cadrage : ce que coûte cette lecture. La prévisualisation lance
  un lecteur externe et n'a donc aucune bibliothèque vidéo en mémoire ; lire des
  métadonnées demanderait `ffprobe` — un exécutable de plus à détecter — ou une
  dépendance. Le gain est réel, le prix reste à mesurer.
- Fichier temporaire : durée de vie, encodage forcé en UTF-8.

---

## 7 — Finitions et première livraison

Ce qui manque pour qu'un tiers installe et utilise l'outil.

Préférences persistées, thème clair et sombre suivant le système, manuel
utilisateur complet pour le contour livré, empaquetage Linux.

**Analyse préalable** — `gaupol/config.py`, `gaupol/style.py`, `data/`,
`PACKAGING.md`, `flatpak/`.

**Questions d'architecture**

- Format de fichier de configuration et migration entre versions.
- Empaquetage : Flatpak, `.deb`, AppImage — lequel pour une première livraison ?

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

**Question ouverte** — Gaupol s'appuie sur `charset-normalizer`, qui n'a pas
d'équivalent direct en C++. Candidats : ICU, `uchardet`, `compact_enc_det`.

## 9 — Formats complémentaires et balises riches

SubViewer 2, Sub Station Alpha, Advanced SSA — les trois formats cités comme
secondaires — puis MicroDVD, MPL2, TMPlayer et LRC.

**Point difficile** — ASS n'est pas un format de timing mais un format structuré
avec sections, styles nommés et événements typés. La conversion vers SubRip est
**structurellement à perte** : quelle politique de dégradation, et la
signale-t-on à l'utilisateur ? C'est ici que se vérifie la solidité du modèle de
balises conçu en phase 1.

## 10 — Opérations complémentaires

Ajustement des durées, casse, italiques, tirets de dialogue, fusion, scission,
recherche et remplacement, presse-papiers.

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
ajustement des durées, correction, inspection. Traitement par lot, sortie
lisible par un humain et sortie exploitable par un script.

Gaupol n'a pas d'équivalent : c'est une conception neuve, et un gain
fonctionnel réel.

## 14 — Lecteur vidéo intégré et calage

Lecture, incrustation des sous-titres et du timecode, sélection de piste audio,
définir début et fin depuis la position vidéo, insérer un sous-titre à la
position vidéo, avancer ou reculer par petits incréments.

**Questions d'architecture** — backend vidéo : libmpv, embarquable et très
tolérant aux formats, contre QtMultimedia, intégré mais plus limité. Décision
par ADR, en tenant compte de la portabilité Windows.

**Point difficile** — **précision de positionnement.** Caler un sous-titre exige
un `seek` exact à l'image près ; la plupart des backends ne le garantissent qu'au
mot-clé le plus proche. C'est la fonctionnalité la plus exigeante de tout le
projet, et celle qui décide de la qualité de l'outil pour le travail de timing.

## 15 — Internationalisation

Les 20 locales de Gaupol sont sous GPL, donc réutilisables.

**Question ouverte** — Qt Linguist ou gettext ? La conversion `.po` vers `.ts`
n'est fidèle que si les chaînes correspondent, ce qui ne sera pas le cas
partout. Évaluer le gain réel avant de s'engager.
