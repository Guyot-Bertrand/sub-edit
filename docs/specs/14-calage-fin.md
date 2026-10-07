# Phase 14 — Calage fin

Cadrage de l'issue [#609](https://github.com/Guyot-Bertrand/sub-edit/issues/609),
après l'initialisation [#608](https://github.com/Guyot-Bertrand/sub-edit/issues/608).

## Ce que la feuille de route promet, et ce qui est déjà là

Trois familles. **Le pilotage de la lecture** — position, sauts, sous-titre précédent
et suivant, début et fin de la sélection, jouer la sélection, volume. **Le calage lui-même**
— poser le début ou la fin depuis la position vidéo, insérer à cette position, avancer et
reculer image par image, décaler par petits incréments. **Ce qui s'affiche** — le timecode,
la piste audio, la réplique sans ses balises brutes. Plus cinq renvois : le mode d'édition
en images, la traduction décalée, la paire d'une conversion à la mauvaise fréquence, le suivi
centré de la table, et la voie d'affichage de #190.

**Le point difficile est la précision de positionnement** : un `seek` exact à l'image près.
**Elle n'est plus une inconnue** : l'initialisation l'a mesurée (voir « Ce que l'initialisation
a mesuré »), et ce qui reste est de la **dire** — ce que « exact » désigne — et de la **prouver**
avec un oracle qui ne repose pas sur le lecteur.

**Gaupol a une partie de ce contour, et pas l'autre.** Ses actions `Video` et `Edit` portent le
pilotage et les cinq gestes de repère ; **elle n'a ni l'image par image, ni le décalage d'une image,
ni la détection d'un décalage**. La phase est donc moitié iso-fonctionnelle, moitié gain.

| Ce qui est déjà là | Où |
| :----------------- | :- |
| **le lecteur intégré** : libmpv, `VideoPlayer` (`open`, `duration`, `position`, `seek` exact, `play`, `pause`, `showSubtitle`, `isPlaying`), le double `FakeVideoPlayer`, `VideoPane` (le lecteur partagé, l'image, le suiveur de 100 ms) | phase 6, [ADR 0020](../adr/0020-libmpv-pour-le-lecteur-integre.md) |
| associer un film, l'ouvrir, jouer et s'arrêter (`Ctrl+P`), la réplique dessinée depuis le modèle, la ligne courante qui suit, **l'avertissement quand une opération dépasse la fin du film** | phase 6 |
| la fréquence que la vidéo déclare, et la **déduction d'une grille** depuis les positions, avec son verdict `Clean`, `Partial`, `Silent` | phases 6 et 16 |
| les commandes que les gestes de calage rejouent : `SetPositionCommand` (un bord d'un sous-titre), `InsertCommand`, `ShiftCommand`, `ConvertFrameRateCommand` | phases 2 et 10 |
| l'ouverture d'une traduction, **son rattachement par position ou par numéro** et son compte (rattachées, nées, sans sous-titre, hors d'ordre) | phase 11 |
| le pivot de balises, qui traduit un texte d'un format vers un autre | [ADR 0031](../adr/0031-pivot-de-balises-a-la-conversion.md) |
| le type fort `Frame`, et les positions en millisecondes entières | ADR [0011](../adr/0011-numero-d-image-en-type-fort.md), [0006](../adr/0006-positions-en-millisecondes.md) |
| **les réglages** de la fenêtre, tolérants option par option | [ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md) |

**Ce qui n'y est pas.** Aucun pilotage au-delà de lecture et pause ; aucun geste de repère ;
aucune image par image ; **aucune image dans la fenêtre Qt** (voir D2) ; aucune notion de ce que
« la position » désigne à l'intérieur d'une image.

## Ce que l'initialisation a mesuré

Mesures de [#608](https://github.com/Guyot-Bertrand/sub-edit/issues/608), libmpv sans écran, sur une
machine de développement. **Des observations, pas un banc** : #610 et #611 les rendent rejouables.

- **La recherche exacte est exacte.** Sur une vidéo dont chaque image porte son numéro et dont l'image-clé
  est unique, sept positions de 0 à 249 rendent l'image demandée, y compris à 720p. L'oracle est
  `screenshot-raw`, qui rend la vraie image sans écran.
- **mpv place le `seek` sur l'image la plus proche de la position**, non sur l'image affichée à cet instant.
  À l'intérieur de l'intervalle d'une image, la première moitié rend cette image, la seconde **la suivante**
  (à 23,976 : l'image 10 occupe 417,08 à 458,79 ms, et 437 ms rend l'image 11).
- **Après un `seek`, `time-pos` est le début de l'image affichée**, non la position demandée : 437 ms demandées,
  458,8 ms rendues. **`position()` est donc toujours un début d'image**, à l'arrondi à la milliseconde près.
- **En millisecondes entières, toutes les images démarrent juste.** Les 24 images essayées à 23,976 rendent la
  bonne image, que la milliseconde soit arrondie vers le haut ou vers le bas : la finesse du `Timestamp` n'est
  pas le défaut.
- **Le coût du saut croît avec la distance à l'image-clé** (≈ 7 ms à 0, ≈ 100 ms à 249 images, 720p), **et le pas
  arrière ne coûte pas plus que le pas avant** (≈ 40 ms l'un et l'autre) : l'hypothèse « reculer est beaucoup plus
  cher » ne se vérifie pas pour ce codec. #611 le rejoue sur une vidéo à la taille d'un film — voir plus bas.
- **`frame-step` n'émet aucun événement** : on ne sait pas quand l'image est affichée. #614 en tire que **le pas est
  un `seek` d'un nombre entier d'images**, qui attend comme tous les autres ordres ; c'est ce que le banc de #611
  mesure, et il vérifie l'image obtenue par l'oracle de #610.
- **Le rendu logiciel de libmpv tient le temps réel** : 2,2 à 3,4 ms par image pour du H.264 1080p, dans un
  tampon de 640×360 à 1920×1080 ([ADR 0041](../adr/0041-afficher-la-video-par-le-rendu-logiciel.md)).
- **L'encodeur `mpeg4` ignore `-g`** quand le contenu change à chaque image : une image-clé toutes les 32 images
  sans `-keyint_min 250 -sc_threshold 1000000000 -bf 0`.
- **Le suivi de la table sonde le lecteur toutes les 100 ms** (`kFollowIntervalMs`) ; les tests pilotent le suiveur
  à la main, et un seul cas paie l'horloge.
- **Le corpus privé ne contient aucune vidéo** : aucune mesure sur un vrai film n'est faite ici, et c'est celle que
  l'utilisateur fait avec le sien.

### Ce que #611 a mesuré

`make bench` fabrique, dans l'arbre de construction, **une vidéo de 1280×720, 250 images, une seule image-clé**
(`video-fixtures.sh --film`) et la donne au banc ; sans ffmpeg, le banc le dit et s'abstient. Chaque geste est vérifié
contre le numéro que porte l'image avant d'être chronométré. Observations d'une machine de développement, `mpeg4`,
au calme relatif — **le relevé au journal est celui qui fait foi** :

| Geste | Image visée | Coût moyen |
| :---- | ----------: | ---------: |
| `seek` | 0 (l'image-clé) | ≈ 8 ms |
| `seek` | 100 | ≈ 45 ms |
| `seek` | 249 | ≈ 107 ms |
| pas avant | 100 | ≈ 51 ms |
| pas avant | 249 | ≈ 106 ms |
| pas arrière | 99 | ≈ 46 ms |
| pas arrière | 248 | ≈ 95 ms |

- **Le pas arrière ne coûte pas plus que le pas avant**, à la même distance de l'image-clé : l'hypothèse « reculer est
  beaucoup plus cher » est écartée pour ce codec, et **le coût ne dépend que de la distance à l'image-clé**, du geste
  non. Un film réel (H.264, HEVC, 1080p) peut dire autre chose ; la mesure sur le sien est celle de l'utilisateur.
- **Ce que cela change au geste (D6)** : à 25 images par seconde, une image dure 40 ms, et une touche maintenue répète
  plus vite que 100 ms. **Loin de l'image-clé, le pas ne suit donc pas la répétition** : celle-ci ne doit pas
  s'empiler — c'est déjà la règle du geste, « un pas à la fois » — et #618 le tient, en ne gardant que le dernier pas
  demandé pendant qu'un pas s'exécute. Le coût de la dernière image d'une vidéo à une seule image-clé est le pire cas.
- **Le banc a trouvé un défaut de #613** : avec `vo=libmpv`, la sortie attend que chaque image soit rendue, jusqu'à
  200 ms, et un `seek` qui attendait sur le fil de la fenêtre calait 400 ms quelle que soit la distance. Le lecteur
  prend désormais lui-même les images pendant qu'il attend ; un cas de `mpv_player_test.cpp` le tient.

## D1 — Le périmètre : ce que Gaupol fait, ce qu'il ne fait pas, et ce qui a un sens ici

Recompté **poste par poste** contre `gaupol/actions/video.py`, `audio.py` et `edit.py`.

| Poste | Gaupol | Ici | Décision |
| :---- | :----- | :-- | :------- |
| position (curseur) | `Gtk.Scale` de la barre de lecteur | **retenu** | D5 |
| reculer, avancer d'un pas | `Seek Backward`, `Seek Forward` — `video_player.seek_length`, **30 s** | **retenu**, pas réglable | D5, D12 |
| sous-titre précédent, suivant | `Seek Previous`, `Seek Next` | **retenu** | D5 |
| début, fin de la sélection | `Seek Selection Start`, `Seek Selection End` — `context_length`, **1 s** d'avance | **retenu** | D5, D12 |
| jouer la sélection | `Play Selection` | **retenu**, jusqu'à sa fin | D5 |
| volume | `Volume Up`, `Volume Down` | **retenu**, retenu d'une session à l'autre | D5, D12 |
| piste audio | `Set Audio Language` (un choix radio) | **retenu** | D5 |
| poser début, fin depuis la position vidéo | `Set Start / End from Video Position` | **retenu** | D6 |
| insérer à la position vidéo | `Insert Subtitle at Video Position` | **retenu** | D6 |
| sélectionner le voisin depuis la position | `Select Previous / Next from Video Position` | **retenu** | D6 |
| **image par image** | **aucun** | **retenu** — la raison du choix de libmpv | D3, D6 |
| **décaler un bord d'une image** | **aucun** | **retenu** | D6 |
| incrustation du timecode | `time_alpha`, `time_background` | **retenu**, dessinée par Qt | D9 |
| la réplique sans balises brutes | rendue par GStreamer | **retenu** ([#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408)) | D9 |
| positions en images | mode `Frames` | **retenu** | D8 |
| **reconnaître une traduction décalée** | **aucun** | **retenu** | D10 |
| **retrouver la paire d'une conversion fausse** | **aucun** | **retenu** ([#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)) | D11 |
| lecture automatique à l'ouverture (`autoplay`) | réglage | **écarté** | rien ne le demande ; déclencheur : une demande |
| style de la réplique (police, couleur, position, fond, transparence) | sept réglages `subtitle_*` | **écarté** | la réplique garde le style que la phase 6 a posé ; déclencheur : une demande, et le rendu logiciel permettrait de les honorer |
| aucune sous-commande | — | **écarté** | le lecteur est de la fenêtre ; une invocation n'a ni image ni position |

**Quatre gains sur Gaupol**, dans la colonne de droite : l'image par image, le décalage d'une image, et les
deux détections.

## D2 — La voie d'affichage : le rendu logiciel de libmpv

**Décidé par l'utilisateur le 2026-10-06 ; l'[ADR 0041](../adr/0041-afficher-la-video-par-le-rendu-logiciel.md) le
porte.** L'image est dessinée par l'API de rendu **logicielle** de libmpv dans un tampon que `VideoSurface` — un
widget Qt — peint. La fenêtre native adoptée par `wid` disparaît, avec la garde de plateforme et ses sept lignes
de couverture. **#190 est tranchée par cette décision**, et la phase la met en œuvre.

**Ce que cela change pour le reste de la phase** : l'image est photographiable et lisible sans écran, donc **le manuel
du calage montre l'image**, et un test compare l'image affichée à l'image numérotée de #610 ; le timecode se dessine
**par Qt**, sur la surface, sans demander au lecteur de le faire. Ce que cela ne change pas : `VideoPlayer` ne connaît
toujours ni libmpv ni Qt.

## D3 — L'interface du lecteur gagne cinq ordres, et pas de notification

`VideoPlayer` gagne :

- **`stepFrames(int)`**, positif en avant, négatif en arrière ;
- **`volume()` et `setVolume(int)`**, de 0 à 100 ;
- **`audioTracks()` et `selectAudioTrack(id)`** : l'identifiant, la langue, le titre ;
- **`playUntil(Timestamp)`** : jouer et s'arrêter à une position, à l'image près. `Play Selection` en a besoin, et
  un suiveur de 100 ms qui arrête la lecture la laisserait dépasser jusqu'à trois images.

**Pas de notification de position.** L'interface dit « un seul fil » et « attend », parce que le noyau n'a pas de
boucle d'événements ; une notification en demanderait une. Le **suivi reste une sonde**, celle de `VideoPane`, que les
tests pilotent à la main : c'est ce qui les garde hors d'une horloge, et un seul cas paie le minuteur. **Une marque ou
un pas se montre tout de suite**, sans attendre le tick suivant — comme `VideoPane` le fait déjà pour une sélection.

Le double suit, sans horloge : il garde sa position à la main, et gagne les mêmes ordres.

## D4 — « La position », « l'image », « exact »

**Dit, parce que c'était faux dans un commentaire** (`seek` : « l'image à son propre début »).

- **`seek(t)` place la lecture sur l'image la plus proche de `t`.** À l'intérieur d'une image, la première moitié rend
  cette image, la seconde la suivante. C'est ce que mpv fait, et ce que l'on promet ; l'écrire dans l'en-tête est
  une issue ([#614](https://github.com/Guyot-Bertrand/sub-edit/issues/614)).
- **`position()` est le début de l'image affichée**, arrondi à la milliseconde. Après `seek(437)`, c'est 459 : le
  début de l'image 11. **Un repère posé depuis la position est donc toujours le début d'une image**, et y revenir
  rend la même image — un aller-retour qui se prouve.
- **Un sous-titre qui débute au milieu d'une image montre, à son début, l'image la plus proche**, parfois la suivante.
  Ce n'est pas une erreur du lecteur : une image n'a pas de milieu qu'un sous-titre puisse viser, et le calage au
  repère, lui, ne s'y trompe pas.
- **La finesse du `Timestamp` n'est pas en cause** : toutes les images de 23,976 démarrent juste en millisecondes
  entières. Aucune représentation fractionnaire n'est introduite.
- **La preuve est #610**, avec un oracle qui lit l'image affichée et non les compteurs du lecteur.

## D5 — Le pilotage

Au menu `Video`, **les gestes de Gaupol** — les mêmes noms, et leurs raccourcis tant qu'ils ne se heurtent pas à un
raccourci que la fenêtre a déjà (le contrôle d'unicité tranche) :

- **reculer et avancer** d'un pas (30 s par défaut), **sous-titre précédent et suivant** (la lecture se place à leur début),
  **début et fin de la sélection** (avec l'avance, 1 s par défaut, avant le début), **jouer la sélection** — qui
  s'arrête à la fin du dernier sous-titre sélectionné, par `playUntil` ;
- **le volume**, par deux gestes et un curseur ;
- **un sous-menu `Audio`**, qui liste les pistes et marque celle qui joue.

**Une barre de lecture sous l'image** : le curseur de position, la position et la durée, lecture/pause **encadrée de deux boutons de pas** (reculer, avancer d'une image — #618), le volume.
**Le curseur ne rend pas la main à chaque pixel** : la recherche est limitée en fréquence, et **la dernière position
demandée est toujours celle qu'on atteint**.

**Les pistes audio ne sont pas retenues** d'une vidéo à l'autre : un numéro de piste est propre à son fichier.
**Les gestes sont éteints sans vidéo**, comme l'est la lecture aujourd'hui.

## D6 — Le calage

**Les cinq gestes de repère de Gaupol, rejoués sur des commandes qui existent.** Poser le début ou la fin du
sous-titre sélectionné depuis la position vidéo : `SetPositionCommand`. Insérer à la position : `InsertCommand`.
Sélectionner le voisin : un déplacement de sélection. **Les mêmes règles et les mêmes messages que la saisie dans une
cellule** — l'ordre strict, une fin avant son début — parce qu'un geste qui aurait ses propres règles dirait autre chose
que la cellule pour la même valeur. **Une entrée d'historique par geste.**

**Les boutons de pas de la barre déclenchent les actions du menu** : même pas, même extinction sans film, même infobulle (avec le raccourci), et un bouton tenu enchaîne les pas, un à la fois.

**Les raccourcis** (#617, #618) : `Alt+Gauche` et `Alt+Droite` pour le pas ; `Alt+Q`, `Alt+W` pour le début plus tôt, plus tard, `Alt+Maj+Q`, `Alt+Maj+W` pour la fin — le `E` de Gaupol est la lettre de `Edit` ; pour les repères, `Alt+U` le début, `Alt+K` la fin, `Alt+J` l'insertion — les `U`, `K` et `J` de Gaupol, que la fenêtre prendrait avant une cellule en cours d'édition — et `Alt+Haut`, `Alt+Bas` pour le voisin. Le `Ctrl+Y` de Gaupol est un *rétablir* sur certains systèmes.

**Le gain sur Gaupol : l'image par image.**

- **Avancer et reculer d'un pas**, deux gestes qui s'enchaînent sous une touche maintenue. **Le pas est un nombre
  d'images, et son minimum est une image** : c'est l'image par image, ce qui permet de placer un sous-titre à l'image
  près. Le pas se règle **vers le haut**, en nombre entier d'images, pour des sauts plus importants (5, 10, 24…) ; il ne
  descend pas sous un, et il n'est jamais en millisecondes. Un pas à la fois : la répétition ne s'empile pas.
- **Décaler le début ou la fin du sous-titre sélectionné d'un pas**, pour régler fin au clavier : une commande
  annulable, qui dit le chevauchement et l'ordre comme la saisie. **Le même pas** que celui du lecteur, donc une
  image par défaut.
- **La durée d'une image** vient, dans cet ordre, de **la fréquence que la vidéo déclare**, de **celle du document
  quand il est compté en images**, de **la grille déduite** (phase 16) — **et sans aucune, le geste refuse en le disant**,
  comme `shift --to-grid` : choisir une fréquence au hasard déplacerait tout le fichier.

**Le pas est compté en images, jamais en millisecondes** : une image est ce que le lecteur sait montrer, et un millième
de seconde ne se voit pas. **Ce qu'il vaut en temps est la durée d'une image de cette vidéo** — 40 ms à 25 images par
seconde, 41,7 ms à 23,976 —, de sorte qu'un pas de N images est toujours N images, quelle que soit la fréquence. C'est ce
qui garde le geste juste pour le calage fin, où l'on pose un bord sur une image et non sur un instant. Un réglage en
millisecondes est un déclencheur, non une promesse.

## D7 — Le suivi de la table

Précisé par l'utilisateur le 2026-09-30 : **la table se centre sur le sous-titre courant**, ce qu'elle ne fait pas
— elle le ramène dans la zone visible.

- **Elle se centre quand la ligne change**, non à chaque tick : une ligne dure en moyenne des secondes, un
  recentrage à chaque tick de 100 ms serait un défilement sans objet.
- **Un défilement à la main suspend le suivi** — la molette, la barre de défilement, un clic dans la table.
- **Le suivi reprend sur un geste du lecteur** : la lecture lancée, un saut, une marque, un pas — **ou à la demande**.
  **Pas de minuterie** : ce qui reprend le suivi est un geste, donc un test le pilote sans horloge.
- **Un bouton de la bande vidéo**, coché quand la table suit, dit l'état et le rétablit.

## D8 — Les positions en images

**Le mode d'édition en images** — renvoyé de la phase 5, repassé par la phase 9. **Un réglage de `View`**, un seul pour
la fenêtre, qui montre `Start` et `End` en numéros d'image et accepte des numéros à la saisie. **La fréquence qui
compte** est celle du document quand il est compté en images (MicroDVD porte des numéros et pas de fréquence), sinon
celle de la vidéo, sinon la grille déduite ; **sans aucune, le réglage est éteint et dit pourquoi**. La bascule redessine
des colonnes : **elle ne touche pas au modèle**, et ce que le fichier écrit ne change pas. Un numéro saisi est converti
par la mise à l'échelle exacte ([ADR 0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md)), arrondi une fois.

## D9 — La réplique et le timecode

- **La réplique sans balises brutes** ([#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408)) : `showSubtitle`
  reçoit le texte avec les balises de son format. Le **pivot de balises** (ADR 0031) le traduit en balises ASS — italique,
  gras, souligné, couleur —, **et une balise qui n'a pas d'équivalent à l'écran est retirée, jamais dessinée telle
  quelle**. Aucun second analyseur : celui de la conversion sert.
- **Le timecode** est dessiné par Qt sur la surface, **à partir de `position()`** ; il ne demande rien au lecteur. Son
  style est celui de la fenêtre.

## D10 — Reconnaître une traduction décalée

**Décidé par l'utilisateur le 2026-10-06 : elle est de cette phase.** Une traduction calée deux secondes trop tard
s'aligne sur les mauvais sous-titres sans que rien la distingue d'un rattachement juste : la phase 11 la rend
**visible** (le compte), pas **corrigeable**.

- **Le noyau cherche un décalage constant.** Les écarts entre le début de chaque ligne et celui du sous-titre le plus
  proche donnent des candidats, regroupés à la dizaine de millisecondes.
- **Chaque candidat est jugé par ce que le rattachement dirait après lui** — lignes rattachées, nées, sans sous-titre,
  hors d'ordre —, **par le même algorithme que l'ouverture**. La confiance est donc **ce que l'ouverture rapporterait
  déjà**, jamais un nombre inventé : « décalée de +2,000 s, quatre lignes sur quatre se rattacheraient, contre une ».
- **Rien n'est proposé** quand le rattachement ne s'améliore pas nettement, **quand deux candidats se valent**, ou
  quand l'écart n'est pas constant (une dérive, un décalage partiel) — **et la phrase ne dit pas alors qu'il y en a un**.
- **La fenêtre** : quand l'ouverture n'est pas propre et qu'un décalage la rendrait propre, **la phrase le dit** et
  **propose de rouvrir décalée** (annuler l'ouverture, puis rouvrir avec le décalage) : une seule entrée d'historique, et
  l'annulation suffit à revenir.
- **Pas de ligne de commande** : ni sous-commande ni option. Si `inspect -t` doit le dire un jour, la clé est additive.
- **Les huit paires de `src/test/data/paires/`** servent de garde : aucune ne se voit proposer un décalage qu'elle n'a
  pas. L'éprouver demande un **générateur de paires avec leur vérité**, écrit avec la détection.

## D11 — Retrouver la paire d'une conversion à la mauvaise fréquence

**Décidé par l'utilisateur le 2026-10-06 : elle est de cette phase** ([#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)),
la spec de la phase 16 l'ayant nommée « le plus utile et le moins sûr » et renvoyée ici, parce que la phase 14 dispose
d'un lecteur capable de **montrer le résultat à l'image près**. Les trois questions que la phase 10 avait laissées :

- **Jusqu'où chercher : un ensemble fermé** — les **cinquante-six paires ordonnées** des huit fréquences normalisées.
  Un rationnel quelconque donnerait toujours une grille à qui cherche assez longtemps ; huit fréquences en donnent
  cinquante-six, qu'on énumère et qu'on départage.
- **La confiance : celle de la déduction.** Chaque rapport est appliqué aux positions, **la déduction de la phase 16
  juge le résultat** (`Clean`, `Partial`, `Silent`), et ce qu'on rend est **la meilleure paire avec l'écart qui la
  sépare de la deuxième et de « ne rien faire »**. **Quand deux paires se valent, aucune n'est proposée**, et la phrase
  le dit. Le déclenchement : la déduction rend `Silent` ou `Partial`.
- **Ce que l'interface montre avant d'appliquer** : la modale d'analyse (`GUI-GRID-02`) montre la proposition ;
  **`Convert Frame Rate…` s'ouvre préremplie** ; **rien n'est appliqué sans geste**, l'annulation suffit à revenir, et
  la vidéo permet de juger à l'image.

## D12 — Les réglages

Quatre réglages, dans `Preferences…`, sous [ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md)
— tolérants option par option, une valeur absurde ne casse pas l'ouverture : **le pas des sauts** (30 s, comme
Gaupol), **le pas d'image** (1 image, de 1 à une borne raisonnable ; D6), **l'avance avant une sélection** (1 s, comme
Gaupol) et **le volume**. **Rien d'autre ne se retient** : ni la
piste audio, ni le suivi, ni le mode en images (un réglage de fenêtre, non d'un document — il suit `View`).


## D13 — Le manuel et les captures

Une page du calage et du lecteur, **la table des raccourcis tenue par [#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612)**,
et des captures **de la barre de lecture et de l'image** : l'ADR 0041 les rend possibles, l'image étant dans la
fenêtre. `make manual` les photographie **sous le vrai lecteur, avec une fixture vidéo versionnée** — celle de #610 ou une
image choisie pour le manuel — plutôt que sous le double, qui ne peint rien ; la relecture de fin de phase relira le
manuel d'un bloc.

## Comment la phase se prouve

- **L'exactitude du positionnement a un oracle indépendant du lecteur** ([#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610)) :
  des vidéos de 13 ko dont chaque image porte son numéro, et une lecture de l'image affichée. **Une seule image-clé**,
  parce que c'est le cas difficile. Après la surface (D2), le test lit l'image **dans la fenêtre**.
- **Le coût du saut et du pas se mesure** ([#611](https://github.com/Guyot-Bertrand/sub-edit/issues/611)) sur une vidéo à
  la taille d'un film, versé au journal comme les autres relevés ; **un banc du rendu** s'y ajoute avec l'ADR 0041.
- **Les raccourcis sont confrontés au manuel** ([#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612)) : une quinzaine
  de gestes au clavier ne se tiennent pas à l'œil.
- **Les gestes de repère se prouvent par l'aller-retour** : poser un repère, y revenir, et lire la même image.
- **Les détections se prouvent par une vérité posée** : un décalage ou une conversion fausse qu'on a introduits, et
  qu'on retrouve ; et **par le silence** — une dérive, deux candidats égaux, un fichier propre ne proposent rien.
- **Le suivi se prouve sans horloge** : le suiveur est piloté à la main, un seul cas paie le minuteur.
- **Un cas sur un vrai film** est celui de la relecture de fin de phase, sur ce que l'utilisateur regarde : le corpus
  privé n'a aucune vidéo, et aucun chiffre ne lui est attribué.

## Ce que la phase ne livre pas

Chacun avec un destinataire.

- **Le décodage matériel sans copie** — **écarté par l'ADR 0041**, avec son déclencheur : un utilisateur dont la lecture
  perd des images à une taille et un codec ordinaires. La réponse serait l'API OpenGL **derrière la même surface**.
- **Le 4K, le HDR, un codec lourd** : **non mesurés**. La relecture de fin de phase le dit.
- **Le style de la réplique et du timecode** (police, couleur, position, fond) — **écarté**, déclencheur : une demande.
- **La lecture automatique à l'ouverture** — **écartée**, même déclencheur.
- **Une ligne de commande du lecteur ou des détections** — **écartée** : D10 et D1.
- **Des incréments en millisecondes** — **écartés**, D6 ; déclencheur : un appelant qui règle au millième.
- **Une proposition de décalage pour une dérive ou un décalage partiel** — **écartée** : D10 ne dit rien plutôt que
  mal. Une dérive linéaire est une conversion de fréquence, et D11 la traite.
- **Des notifications de position** par le lecteur — **écartées**, D3 ; déclencheur : un besoin que la sonde de 100 ms
  ne sert pas.
- **Un son** : la transcription audio, l'alignement automatique — phases 22 et 23.

## Écarts avec Gaupol

| Ce que fait Gaupol | Ce que fait `subedit` | Pourquoi |
| :----------------- | :-------------------- | :------- |
| l'image est rendue par GStreamer dans un widget GTK | **par libmpv, en logiciel, dans un widget Qt** | ADR 0041 |
| `Seek Backward / Forward` de 30 s, sans réglage de l'interface | **le pas est un réglage de `Preferences…`**, 30 s par défaut | D12 |
| aucun pas d'une image | **avancer et reculer d'une image**, et décaler un bord d'une image | la raison du choix de libmpv |
| le sous-titre dessiné a sept réglages de style | **le style de la phase 6, sans réglage** | D1 |
| `autoplay` | **aucune lecture automatique** | D1 |
| aucune détection de décalage | **deux détections, proposées et jamais appliquées** | D10, D11 |
| le suivi de la lecture ramène la ligne à l'écran | **la table se centre, et se suspend à un défilement à la main** | D7, demande de l'utilisateur |
| `Set Audio Language` : un choix de langue | **un sous-menu des pistes**, par langue, titre ou numéro | une piste n'a pas toujours de langue |

## Exigences

**Vingt-huit**, toutes `prévues`, **inscrites au registre avec cette spec** : `check-requirements.sh` confronte la
table d'une spec de phase au registre dans les deux sens. Chaque issue en **relit** les siennes avant son code — l'état
passe à `implémentée` quand un test les cite, jamais avant. **Douze sujets neufs**, dont aucun identifiant n'était pris :
`SURFACE`, `STEP`, `NUDGE`, `SEEK`, `VOLUME`, `TIMECODE`, `AUDIO`, `MARK`, `FOLLOW`, `REPLICA`, `DRIFT`, `REPAIR` ; `FRAMES`
continue de `GUI-FRAMES-01`.

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-SURFACE-01` | l'image de la vidéo est dessinée dans la fenêtre, redimensionnable, son rapport d'aspect conservé |
| `GUI-SURFACE-02` | après un saut, l'image affichée est celle de la position demandée, la plus proche |
| `GUI-SURFACE-03` | la fenêtre n'adopte aucune fenêtre native et n'exige pas X11 |
| `GUI-STEP-01` | avancer d'une image affiche l'image suivante |
| `GUI-STEP-02` | reculer d'une image affiche l'image précédente |
| `GUI-STEP-03` | la durée d'une image vient de la vidéo, à défaut du document, à défaut de la grille ; sans aucune, le geste refuse en le disant |
| `GUI-STEP-04` | le pas se règle en nombre d'images, au minimum une ; une valeur absurde ne casse pas l'ouverture |
| `GUI-NUDGE-01` | décaler le début ou la fin d'un sous-titre d'une image est annulable en une entrée |
| `GUI-SEEK-01` | le curseur de position se lit et se déplace |
| `GUI-SEEK-02` | reculer et avancer d'un pas réglable |
| `GUI-SEEK-03` | le sous-titre précédent et le suivant placent la lecture à leur début |
| `GUI-SEEK-04` | le début et la fin de la sélection placent la lecture, avec l'avance réglée |
| `GUI-SEEK-05` | jouer la sélection s'arrête à sa fin |
| `GUI-VOLUME-01` | le volume se règle et se retient d'une session à l'autre |
| `GUI-TIMECODE-01` | le timecode est incrusté sur l'image |
| `GUI-AUDIO-01` | la piste audio se choisit parmi celles de la vidéo |
| `GUI-MARK-01` | poser le début d'un sous-titre depuis la position de la vidéo, en une entrée d'historique |
| `GUI-MARK-02` | poser sa fin depuis la position de la vidéo |
| `GUI-MARK-03` | insérer un sous-titre à la position de la vidéo |
| `GUI-MARK-04` | sélectionner le sous-titre précédent ou suivant depuis la position de la vidéo |
| `GUI-FOLLOW-01` | la table centre le sous-titre courant pendant la lecture |
| `GUI-FOLLOW-02` | un défilement à la main suspend le suivi |
| `GUI-FOLLOW-03` | le suivi reprend sur un geste du lecteur ou à la demande |
| `GUI-REPLICA-01` | la réplique dessinée sur l'image n'a plus de balises brutes |
| `GUI-FRAMES-02` | les positions s'affichent et se saisissent en numéros d'image, la fréquence dite |
| `GUI-DRIFT-01` | l'ouverture d'une traduction dit qu'un décalage constant la rattacherait mieux, sans le dire d'une dérive |
| `GUI-DRIFT-02` | rouvrir décalée rattache selon le décalage proposé, en une entrée d'historique |
| `GUI-REPAIR-01` | la modale d'analyse propose la conversion de fréquence qui remet le fichier sur une grille, ou rien quand deux se valent |
| `GUI-REPAIR-02` | `Convert Frame Rate…` s'ouvre préremplie par cette proposition |

## Découpage

**Cinq tranches**, le fond d'abord — la surface et l'interface du lecteur, dont tout dépend —, puis ce que
l'utilisateur touche, puis les deux détections, qui ne dépendent du lecteur que pour juger à l'image. **Les issues
d'outillage de l'initialisation (#610 à #612) passent avant ; #610 avant toute issue d'implémentation.**

**Les issues sont ouvertes**, dans le milestone 14 : [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610)
à [#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623), avec [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)
et [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408), plus anciennes, qui y trouvent leur place.

| Tranche | Issue | Ce qu'elle livre | Dépend de | Taille |
| :------ | :---- | :--------------- | :-------- | :----- |
| 0 — l'outillage | [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610) | **des vidéos qui portent le numéro de leurs images**, et un oracle | — | M |
| | [#611](https://github.com/Guyot-Bertrand/sub-edit/issues/611) | **le banc du saut et du pas**, selon la distance à l'image-clé | #610 | S |
| | [#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612) | **le manuel dit les raccourcis** de la fenêtre | — | S |
| 1 — le fond | [#613](https://github.com/Guyot-Bertrand/sub-edit/issues/613) | **la surface** : le rendu logiciel de libmpv, dans la fenêtre Qt (ADR 0041, #190) | #610 | L |
| | [#614](https://github.com/Guyot-Bertrand/sub-edit/issues/614) | **l'interface du lecteur** : pas d'image, volume, pistes, `playUntil`, et ce que « la position » désigne | #610 | M |
| 2 — le pilotage | [#615](https://github.com/Guyot-Bertrand/sub-edit/issues/615) | **la barre de lecture** : curseur, sauts, voisins, sélection, volume, timecode, trois réglages | #613, #614 | L |
| | [#616](https://github.com/Guyot-Bertrand/sub-edit/issues/616) | **la piste audio** | #614 | S |
| 3 — le calage | [#617](https://github.com/Guyot-Bertrand/sub-edit/issues/617) | **les repères** : début, fin, insertion, voisin, depuis la position | #614 | M |
| | [#618](https://github.com/Guyot-Bertrand/sub-edit/issues/618) | **l'image par image**, et le décalage d'un bord d'une image | #614, #617 | M |
| | [#619](https://github.com/Guyot-Bertrand/sub-edit/issues/619) | **le suivi de la table** : centré, suspendu à la main, repris sur un geste | — | M |
| 4 — l'affichage | [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408) | **la réplique sans balises brutes**, par le pivot de balises | — | S |
| | [#620](https://github.com/Guyot-Bertrand/sub-edit/issues/620) | **les positions en images** : un réglage de `View` | — | M |
| 5 — les détections | [#621](https://github.com/Guyot-Bertrand/sub-edit/issues/621) | **reconnaître une traduction décalée**, et la rouvrir décalée | — | L |
| | [#622](https://github.com/Guyot-Bertrand/sub-edit/issues/622) | **proposer la conversion qui remet un fichier sur une grille** (#386) | #613, #615 | L |
| | [#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623) | **relecture de fin de phase** | tout | M |

**#613 est la pierre d'angle de la phase, et la plus risquée** : elle change la façon dont l'image arrive, donc
ce que chaque issue du lecteur touche, et c'est elle qui engage l'ADR 0041. **#614 est la plus étroite et la plus
bloquante** : tout ce que l'on pose ou pilote passe par `VideoPlayer`.

**Ce qui peut avancer en parallèle** : #619, #620, #621 et #408 ne dépendent d'aucune issue du lecteur ; #611 et #612
dès #610 et à tout moment.

## Points ouverts

**Deux sont tranchés par l'utilisateur le 2026-10-06** (1 et 2). Les autres sont ce que le cadrage ne peut pas trancher
seul, **chacun avec ce qui est supposé en attendant**.

| N° | Point | Supposé | Qui, quand |
| :- | :---- | :------ | :--------- |
| 1 | **la voie d'affichage** : fenêtre native, rendu OpenGL, rendu logiciel | le rendu logiciel (ADR 0041) | **tranché le 2026-10-06** : la supposition est retenue |
| 2 | **les deux détections** : dans la phase, ou renvoyées | les deux dans la phase (D10, D11) | **tranché le 2026-10-06** : dans la phase, contre la supposition du cadrage, qui renvoyait #386 |
| 3 | **ce qu'une image « est »** : la plus proche d'une position, ou celle qui est affichée à cet instant | la plus proche, ce que mpv fait (D4) | #614 ; une décision contraire demanderait de corriger la position demandée avant le `seek` |
| 4 | **le pas des sauts** : 30 s, comme Gaupol | 30 s | #615, sur demande réelle |
| 5 | **la reprise du suivi** : un geste du lecteur, ou aussi un délai | un geste seulement, pas de minuterie (D7) | la relecture, sur usage |
| 6 | **un décalage de traduction proposé pour une dérive** | non (D10) | la relecture, sur demande réelle |
| 7 | **la forme de « rouvrir décalée »** : un bouton de la boîte de l'ouverture, ou une action de menu | un bouton de la boîte, qui existe déjà quand l'ouverture n'est pas propre | #621 |
| 8 | **le 4K, le HDR, HEVC** sous le rendu logiciel | non mesurés ; l'ADR dit son déclencheur | la relecture de fin de phase, sur ce que l'utilisateur regarde |
| 9 | **le décodage matériel** : `hwdec-copy` sous le rendu logiciel | non | #613, si la mesure le justifie |
