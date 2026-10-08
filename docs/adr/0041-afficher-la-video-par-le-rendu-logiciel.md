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

## Mesuré à la relecture de fin de phase 14 ([#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623))

**La décision tient pour ce qu'elle a mesuré, et une limite est maintenant chiffrée.** Trois films de dix secondes, 25 images par seconde, du bruit ajouté pour qu'ils pèsent comme de vrais films, **décodés sous la limite de deux cœurs de la porte** (`limit-cores.sh`) :

| Film | Débit | Décodage de 10 s de film | Vitesse |
| :--- | ----: | -----------------------: | ------: |
| H.264 1080p | 69 Mb/s | 4,0 s | 2,5 × le temps réel |
| HEVC 4K, 8 bits | 117 Mb/s | 16,1 s | **0,62 ×** |
| HEVC 4K, 10 bits | 70 Mb/s | 10,9 s | **0,92 ×** |
| HEVC 4K, 8 bits, VAAPI (ffmpeg) | 117 Mb/s | 4,7 s | 2,1 × |

- **Le 1080p H.264 tient le temps réel avec de la marge**, comme l'ADR l'avait mesuré. Le rendu lui-même — la mise à l'échelle et la conversion dans le tampon — ne dépend pas du film.
- **Le 4K HEVC lourd ne le tient pas sur deux cœurs**, en 8 comme en 10 bits. Le pas d'une image, qui est une recherche exacte et décode depuis l'image-clé, coûte de 0,4 s (25 images de l'image-clé) à 5 s (225 images) sur ces films, qui n'ont qu'une image-clé ; un vrai film en a une toutes les deux à dix secondes.
- **Le décodage matériel de cette machine** ramène le 4K à plus de deux fois le temps réel — mesuré par ffmpeg, pas à travers libmpv. Le déclencheur ci-dessus est atteint pour qui regarde du 4K sur une machine modeste, et **la réponse est `hwdec=auto-copy` sous la même surface, pas l'API OpenGL** : [#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647).
- **Ce qui n'est pas mesuré** : le décodage matériel à travers libmpv, la lecture suivie avec comptage des images perdues, le HDR, AV1, une machine sans carte graphique. Les films ne sont pas versionnés ; le banc `seeking and stepping on a real film` les prend par `SUBEDIT_BENCH_REAL_FILM` et s'abstient sans.

## Addendum — le décodage matériel avec copie ([#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647))

**`MpvPlayer` demande `hwdec=auto-copy`.** La carte décode, l'image est recopiée en mémoire, et le rendu logiciel la dessine comme avant : la surface, les tests sans écran et les captures du manuel ne changent pas. Sur une machine sans carte, mpv décode sur le processeur sans rien dire. Un réglage de `Preferences…`, `video.hardware-decoding` (actif par défaut), le coupe, à l'instant et pour les films suivants.

**Mesuré à travers libmpv**, sous la limite de deux cœurs (`limit-cores.sh`), sur les films de dix secondes de la relecture de fin de phase 14, 100 échantillons par cas, machine non au repos (charge de 2 à 3). Chercher une image puis la rendre, moyenne, avec la carte puis sans :

| Film | à 25 images de l'image-clé | à 100 images | à 225 images |
| :--- | -------------------------: | -----------: | -----------: |
| H.264 1080p | 0,10 s contre 0,36 s | 0,47 s contre 1,55 s | 1,11 s contre 3,49 s |
| HEVC 4K, 8 bits | 0,83 s contre 1,59 s | 2,66 s contre 4,66 s | 4,79 s contre 5,32 s |
| HEVC 4K, 10 bits | 0,72 s contre 0,98 s | 1,74 s contre 2,94 s | 3,83 s contre 5,15 s |

Le pas d'une image donne les mêmes ordres de grandeur (même recherche exacte).

- **Le gain est réel et il est inégal.** Il va de 1,1 × (4K 8 bits, loin de l'image-clé) à 3,2 × (1080p). Il est **plus faible que les 2,1 × du décodage seul mesuré par ffmpeg** : la copie de l'image vers la mémoire, la conversion de couleur et la mise à l'échelle restent sur le processeur, et à 4K elles pèsent.
- **Le 4K lourd ne devient pas fluide.** À 225 images de l'image-clé, un pas coûte encore près de quatre secondes. Ce que la copie ne règle pas — et que seule l'API OpenGL sans copie réglerait, le déclencheur de l'ADR restant ce qu'il est — est le prix de la recherche exacte, qui décode depuis l'image-clé.
- **L'exactitude ne bouge pas** : les tests sur vidéos numérotées passent avec et sans (`seeking and stepping are exact with and without hardware decoding`). Sur une machine sans carte, les deux séries sont logicielles et ne prouvent que cela ; avec une carte, elles prouvent que l'image recopiée est celle qu'on a demandée.
- **Ce qui n'est toujours pas mesuré** : la lecture suivie avec comptage d'images perdues, le HDR, AV1, une machine sans carte.
