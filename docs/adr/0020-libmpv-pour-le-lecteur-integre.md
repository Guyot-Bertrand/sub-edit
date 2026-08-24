# 0020 — libmpv pour le lecteur intégré

**Date :** 2026-08-22
**Statut :** acceptée

## Contexte

La phase 6 était cadrée autour d'une **prévisualisation par lecteur externe** :
écrire un fichier temporaire, lancer mpv, MPlayer ou VLC positionné au
sous-titre courant, comme le fait `aeidon/agents/preview.py`. Le lecteur intégré
était en phase 14, où la feuille de route l'avait rangé comme « la partie la plus
coûteuse du projet ».

Ce cadrage a été repris en cours de route : **il faut un lecteur dans la
fenêtre**. L'iso-fonctionnalité le demandait d'ailleurs — Gaupol a les deux, et
`gaupol/player.py` est un lecteur GStreamer embarqué, distinct de la
prévisualisation externe. La feuille de route avait retenu le second et oublié
le premier.

Le choix du backend était identifié depuis le début comme une ADR à venir —
l'index le portait en toutes lettres : « Backend vidéo — libmpv ou
QtMultimedia. Phase 14. » Il est dû maintenant.

## Décision

**libmpv**, le paquet `libmpv-dev`, version 0.37 dans Ubuntu 24.04.

Le critère qui tranche n'est pas la phase 6, que les deux savent servir : c'est
**ce qui reste après elle**. La phase 14 ne garde plus que le calage fin —
l'avance image par image, la pose d'un repère depuis la position courante. Or
c'est précisément ce qu'un backend sait faire ou ne sait pas faire, et cela ne
se rattrape pas par du code au-dessus.

| Ce que la phase 14 demandera | libmpv | Qt Multimedia |
| :--------------------------- | :----- | :------------ |
| chercher à une position exacte | `seek <t> absolute+exact` | approximatif, dépend du backend |
| avancer et reculer d'une image | `frame-step`, `frame-back-step` | pas d'API |
| la fréquence du conteneur | propriété `container-fps` | non exposée |

**Choisir Qt Multimedia serait choisir de refaire ce travail** le jour où la
phase 14 arrive, ou d'y adjoindre un second backend — donc d'avoir deux moteurs
vidéo dans un projet qui n'en voulait qu'un.

Deux arguments de moindre poids vont dans le même sens : mpv est le lecteur que
Gaupol recommande en premier, donc le comportement que l'utilisateur connaît
déjà ; et mpv existe sur Windows et macOS, si bien que l'ADR 0003 — « à qualité
comparable, on retient systématiquement l'option portable » — n'y perd rien.

## Alternatives écartées

- **Qt Multimedia.** Déjà installé depuis #117, et `QVideoWidget` s'insère dans
  la fenêtre sans colle : c'est un vrai avantage, et il ne pèse pas assez.
  Écarté sur le calage fin, comme dit plus haut. Ubuntu 24.04 fournit Qt 6.4.2
  avec les deux greffons, `libffmpegmediaplugin` et `libgstreamermediaplugin` —
  ce n'est donc pas le moteur de décodage qui manque, c'est l'API.

- **GStreamer directement**, comme Gaupol. Écarté : c'est descendre d'un cran
  sous ce que Qt Multimedia offre déjà, pour hériter des mêmes limites d'API
  sur le calage, et écrire soi-même le pipeline.

- **Garder le lecteur externe et attendre**, en livrant la phase 6 telle qu'elle
  était cadrée. Écarté par la demande, et à raison : le lecteur externe n'était
  qu'un substitut posé quand l'intégré semblait trop cher. Le garder en plus
  coûterait deux surfaces, deux sections de manuel et deux jeux de tests pour un
  geste que le lecteur intégré rend mieux.

## Conséquences

**Une dépendance système de plus**, `libmpv-dev`, à faire entrer dans
`setup-toolchain.sh` et donc dans le cache de paquets de la CI. Le piège est
connu et déjà payé deux fois — Qt en #117, `ffmpeg` en #162.

**Une couture à écrire à la main.** libmpv expose une API C : un handle qu'on
crée, qu'on interroge par propriétés et qu'on pilote par commandes, et qu'il
faut détruire. C'est un propriétaire de ressource au sens du principe 2 du
projet, et il vaudra sa classe, comme `SpawnActions` a valu la sienne.

**Une question d'affichage tranchée à l'implémentation, et son prix.** Le choix
était entre rendre dans une fenêtre native que libmpv adopte (propriété `wid`)
et rendre dans un contexte OpenGL que Qt fournit. #173 a retenu la première,
comme la plus courte. Ce paragraphe annonçait que la décision ne serait « pas
coûteuse à défaire » ; elle s'est révélée coûteuse à **tenir**, et le constat
est inscrit ici parce qu'il change ce qu'on sait, non ce qu'on a décidé.

Ce que #176 a mesuré en la posant :

- **adopter une fenêtre native est un mécanisme X11.** L'en-tête de libmpv le
  dit — « This works on X11, win32, and OSX only » — et recommande l'API de
  rendu. `subedit-gui` demande donc `xcb` quand la session laisse le choix
  ouvert, et **refuse de construire un lecteur** sur toute autre plateforme Qt ;
- **le contexte doit être nommé.** Laissé sonder sur une machine où
  `WAYLAND_DISPLAY` est posé, mpv choisit Wayland, où `wid` ne veut rien dire,
  et ouvre une fenêtre à côté de la nôtre ;
- **la fenêtre doit être à l'écran avant d'être adoptée**, sans quoi mpv ne
  mappe jamais la sienne.

Trois lignes de code, et trois défauts que seul l'usage a montrés.

**Ce que cela laisse ouvert, et pour quand.** L'API de rendu marche sous
Wayland, et c'est celle que libmpv recommande. La phase 14 rouvre ce code de
toute façon — avance image par image, incrustation du timecode — et c'est là que
le changement coûterait le moins. La décision n'est pas prise ici : elle demande
de peser un contexte OpenGL maintenu à la main contre une dépendance à XWayland,
et cela se fait au cadrage de la phase 14.

**Le déclencheur qui la forcerait**, lui, est net : une session sans serveur X du
tout. `subedit-gui` n'y jouerait aucun film, et le dit.

**Le sous-titre affiché ne vient pas d'un fichier.** libmpv sait charger un
fichier de sous-titres externe, et ce serait le mauvais choix pour un éditeur :
il faudrait le réécrire à chaque frappe. La réplique courante est dessinée
depuis le modèle, qui est déjà ce que la table de la phase 5 affiche. Le fichier
temporaire de l'ancien cadrage disparaît avec le lecteur externe.

**`ffprobe` reste**, et pour une seule raison : il rend la fréquence d'image
comme **un rationnel exact** — `24000/1001` — là où `container-fps` de libmpv
est un flottant. `FrameRate` est un rationnel depuis la phase 1, et l'y convertir
par approximation serait perdre ce que ce choix protégeait. La durée, elle, vient
du lecteur, qui l'a déjà : deux sources pour la même donnée sans nécessité
seraient une de trop.

Le déclencheur qui rouvrirait cette décision : libmpv absent d'une distribution
visée, ou une API Qt Multimedia qui gagnerait le pas à l'image. Aucun des deux
n'est en vue.

Défaire cette décision coûte ce que coûte un backend : la couture est derrière
une interface au sens du principe 3 — « lecteur vidéo » y est nommé comme point
de variation depuis les fondations, précisément pour ce cas.
