# 0041 — Afficher la vidéo par le rendu logiciel de libmpv

**Date :** 2026-10-06
**Statut :** acceptée

**Complète [0020](0020-libmpv-pour-le-lecteur-integre.md)** : libmpv reste le moteur, et rien de ce que 0020 a pesé n'est remis en cause. Ce qui change est **la façon dont l'image arrive dans la fenêtre**, que 0020 avait laissée « tranchée à l'implémentation » — #173 avait retenu la fenêtre native adoptée par `wid`. Décidée par l'utilisateur au cadrage de la phase 14 ([#190](https://github.com/Guyot-Bertrand/sub-edit/issues/190), [#609](https://github.com/Guyot-Bertrand/sub-edit/issues/609)).

## Contexte

La voie de #173 a coûté plus qu'elle ne promettait, et la mesure en est dans #190 :

- **adopter une fenêtre native est un mécanisme X11** — l'en-tête de libmpv le dit — : `subedit-gui` demande `xcb` quand la session le laisse faire, et refuse de construire un lecteur sur toute autre plateforme Qt ;
- le contexte doit être nommé, la fenêtre doit être à l'écran avant d'être adoptée, et sept lignes de couverture ne se parcourent qu'avec une session X11 ;
- **l'image n'est pas dans la fenêtre Qt** : ce que Qt photographie ne la contient pas, et sans écran il n'y a pas d'image du tout. Le manuel ne peut pas la montrer, un test ne peut pas la lire, et l'incrustation que la phase 14 ajoute (le timecode) ne peut pas être dessinée par Qt dessus.

La phase 14 rouvre ce code de toute façon — l'image par image, l'incrustation, la barre de lecture — : c'est le moment, et le seul bon marché.

**libmpv offre deux API de rendu** : OpenGL, que sa documentation recommande, et **logicielle** (`MPV_RENDER_API_TYPE_SW`), qui dessine dans un tampon que l'appelant fournit. Cette dernière n'a pas besoin de contexte graphique, ni de fenêtre, ni de X11.

**Mesuré à l'ouverture de ce cadrage**, sur une machine de développement, avec une vidéo H.264 1920×1080 à 25 images par seconde, sans décodage matériel, `BLOCK_FOR_TARGET_TIME` à zéro : le rendu coûte **2,2 à 3,4 ms par image en moyenne** pour un tampon de 640×360 à 1920×1080, **20 à 40 ms au pire** sur une machine chargée (charge 9 sur 12 cœurs), et **la lecture tient le temps réel** — 123 à 125 images en cinq secondes, aucune perdue. Ce qui n'a pas été mesuré : le 4K, le HDR, un codec lourd (HEVC, AV1) et une machine modeste.

## Décision

**L'image est rendue par l'API de rendu logicielle de libmpv, dans un tampon que `VideoSurface` — un widget Qt — peint.** `MpvPlayer` crée un contexte de rendu (`vo=libmpv`) ; le rappel de mise à jour, appelé d'un fil de mpv, **passe au fil de la fenêtre**, qui rend sans attendre l'heure cible et demande une peinture. Le tampon est à la taille du widget **en pixels physiques** ; libmpv conserve le rapport d'aspect. La fenêtre native adoptée, le contexte `xcb` imposé et l'identifiant de fenêtre de `PlayerFactory` **disparaissent**.

L'interface `VideoPlayer` ne change pas pour cela : elle ne sait toujours ni libmpv ni Qt, et l'image n'y passe pas. **La surface est l'affaire de `subedit_gui`.**

## Alternatives écartées

- **Rester sur la fenêtre native (`wid`).** Le moins de travail, et rien ne casse. Écartée : elle laisse l'image hors de Qt — donc ni capture pour le manuel, ni lecture de l'image par un test, ni incrustation dessinée par Qt — et fait dépendre `subedit-gui` d'XWayland pour toujours.
- **L'API de rendu OpenGL.** La recommandée, qui permet le décodage matériel sans copie. Écartée **pour cette phase**, pas pour toujours : elle demande un contexte OpenGL tenu à la main, des rappels de rendu câblés sur le fil de la fenêtre, et **une surface de test qui ne devient pas plus facile** — un contexte GL sans écran est un environnement de plus à tenir sur la machine de chacun. Le coût logiciel mesuré ne le justifie pas à la taille d'un éditeur de sous-titres.
- **Qt Multimedia.** Écartée par 0020 sur le calage fin, et rien n'a changé.

## Conséquences

**L'image est dans la fenêtre**, donc : photographiable par `make manual` ; lisible par un test sans écran, qui compare l'image affichée à l'image numérotée de [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610) ; surchargeable par Qt (le timecode) ; et **la même sous Wayland, X11 et ailleurs**. Le garde de plateforme et ses lignes de couverture partent.

**Tout passe par le processeur.** Le décodage matériel sans copie n'est pas disponible avec ce rendu — `hwdec-copy`, s'il convient, copie chaque image — et la mise à l'échelle comme les conversions de couleur sont celles de libmpv logiciel. Une image coûte une copie du tampon vers `QImage` : 3,7 Mo à 720p, 25 fois par seconde, sans conséquence mesurable.

**Un fil de plus à traverser** : l'appel de rappel vient d'un fil de mpv et ne touche à rien ; tout le reste — la demande de rendu, la peinture, les ordres au lecteur — reste sur le fil de la fenêtre, ce que `VideoPlayer` promet déjà (« un seul fil »).

**Un coût à défaire modeste** : la surface est un widget derrière `MpvPlayer`. Passer à OpenGL plus tard remplacerait le tampon par un contexte, sans toucher à l'interface ni aux appelants.

**Déclencheur pour reconsidérer :** un utilisateur dont la lecture perd des images à une taille et un codec ordinaires — la relecture de fin de phase mesure sur ce qu'on regarde vraiment —, ou le besoin d'un format que le rendu logiciel traite mal (HDR, 10 bits). La réponse est alors l'API OpenGL **derrière la même surface**.
