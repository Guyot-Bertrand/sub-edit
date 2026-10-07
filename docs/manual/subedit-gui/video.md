# La vidéo associée

Le menu **Video** porte d'abord deux commandes, et les gestes qui pilotent le film
— voir [Le lecteur](lecteur.md#se-déplacer-dans-le-film).

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Select Video…` | aucun | choisit le fichier vidéo que le document accompagne |
| `Play / Pause` | `Ctrl+P` | joue ou arrête la vidéo — voir [Le lecteur](lecteur.md#jouer-et-arrêter) |

La vidéo associée est **une propriété du document ouvert**, pas un réglage de
l'application : ouvrir un autre fichier de sous-titres repart de zéro. Elle
n'est écrite dans aucun fichier et disparaît avec la fenêtre.

## La piste audio

Un film peut porter plusieurs pistes sonores — la version originale, un doublage, un commentaire. Le
sous-menu **`Video ▸ Audio ▸ Language`** les liste, **une entrée par piste**, et **marque celle qui joue** :

| Ce que le fichier donne | Ce que l'entrée dit |
| :---------------------- | :------------------ |
| une langue et un titre | `1: fra — Original` |
| une langue seule | `2: eng` |
| un titre seul | `3: Commentary` |
| ni l'un ni l'autre | `1` |

Le numéro est **la place de la piste dans le film**, de un en un, dans l'ordre du fichier : deux pistes
peuvent dire `eng`, et un fichier muet sur ses étiquettes a tout de même une première et une seconde.

**Choisir une entrée joue cette piste.** La marque est la réponse du lecteur, non le clic : elle dit la
piste qui joue.

- **Le sous-menu est éteint** sans vidéo ou sans piste audio. Avec une seule piste, il reste actif et
  la montre, cochée : il dit la langue du film. Les deux gestes du volume, qui sont à côté, restent
  actifs quoi qu'il en soit : voir [Le volume](lecteur.md#le-volume).
- **La piste n'est pas retenue** d'une vidéo à l'autre, ni d'une session à l'autre : un numéro de piste est
  celui de son fichier, et le reporter sur le film suivant choisirait ce qui porte ce numéro là-bas.
  Revenir sur un onglet rouvre son film sur la piste que le lecteur choisit de lui-même.
- Aucun raccourci : on change de langue rarement, et le menu suffit.

## Choisir une vidéo

Le dialogue s'ouvre sur le répertoire du fichier de sous-titres et filtre sur
les extensions vidéo reconnues :

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Il propose aussi d'afficher tous les fichiers. Renoncer au dialogue ne change
rien.

Le fichier choisi est **ouvert aussitôt** : l'image apparaît dans la fenêtre, à
l'arrêt sur son premier instant. Voir [Le lecteur](lecteur.md).

**Glisser un fichier vidéo sur la fenêtre fait la même chose** — voir
[Glisser des fichiers sur la fenêtre](fichiers.md#glisser-des-fichiers-sur-la-fenêtre).

**La vidéo est celle du projet de l'onglet courant.** Chaque onglet garde la
sienne ; changer d'onglet change le film. **Revenir sur un onglet reprend son
film où on l'avait laissé**, en pause, comme toute ouverture. Un autre film
choisi pour ce projet entre-temps repart, lui, du début.

## La proposition automatique

À l'ouverture d'un fichier de sous-titres, **si aucune vidéo n'a été choisie**,
la fenêtre en cherche une dans le même répertoire : un fichier vidéo dont le nom
sans extension est un préfixe du nôtre, au point près.

| Fichier de sous-titres | Vidéo trouvée | Pourquoi |
| :--------------------- | :------------ | :------- |
| `film.fr.srt` | `film.mkv` | `film` est un préfixe de `film.fr` |
| `film.en.forced.srt` | `film.mkv` | idem, quel que soit le nombre de segments |
| `film.fr.srt` | `film.fr.mkv` plutôt que `film.mkv` | le nom le plus long l'emporte |
| `film.fr.srt` | aucune, si `film.mkv` **et** `film.mp4` sont là | deux candidats ne se départagent pas |
| `film.fr.srt` | aucune, si seul `fil.mkv` est là | `fil` ne s'arrête pas sur un point |

**Une proposition n'est pas un choix.** Elle se laisse remplacer par une autre
proposition — ouvrir un autre fichier, enregistrer sous un autre nom. Un choix
explicite, lui, n'est jamais réécrasé : une fois `Select Video…` utilisé, la
proposition automatique ne repasse plus derrière.

C'est ce qui rend la devinette acceptable : elle est corrigible, et sa
correction tient.

## Ce que la barre d'état montre

À droite de la barre d'état, une ligne dit ce que le document accompagne. Elle
est la dernière des trois : l'[encodage](fichiers.md#lencodage-dans-la-barre-détat)
du fichier lu vient en premier, la [grille d'images](grille.md) déduite des
positions ensuite.

| Situation | Ce qui est écrit |
| :-------- | :--------------- |
| une vidéo est associée | `Video: film.mkv` |
| … et `ffprobe` en donne la cadence | `Video: film.mkv, 24000/1001 fps` |
| aucune | `No video` |

**La cadence va avec le film plutôt qu'à côté.** Ce sont un seul fait — ce que
ce document accompagne — et une troisième mention la mettrait au même rang que
la [grille déduite des positions](grille.md), qui en est un autre.

## Quand il n'y a pas de film

La fenêtre montre un bouton **`Select Video…`** là où l'image irait, plutôt que
rien du tout. C'est la même commande que celle du menu `Video`, à portée de
souris : une absence sur laquelle on ne peut pas agir se distingue mal d'un
défaut.

Le bouton disparaît dès qu'un film est ouvert, et l'image prend **toute** sa
place : la hauteur que la table laisse au-dessus d'elle — celle que
[`window.table-share`](preferences.md) a retenue d'une session à l'autre —, et
jamais moins que la hauteur minimale de l'image, que la table cède au besoin.
Quand le film s'en va, le bouton reprend la même place.

**Le nom du fichier, jamais son chemin.** Un chemin de deux cents caractères
chasserait tout le reste de la barre ; le chemin complet est celui que le
dialogue a montré.

## `ffmpeg` n'est pas requis

`subedit-gui` interroge `ffprobe` — livré avec `ffmpeg` — pour une chose et une
seule : **la fréquence d'images que le conteneur déclare**. Elle est lue une fois
par film, au moment où il est associé.

| Avec `ffprobe` | Sans |
| :------------- | :--- |
| la barre d'état écrit la cadence à côté du nom du film | elle n'écrit que le nom |
| le dialogue de conversion propose la fréquence du film au champ du bas | il ne propose rien, et s'ouvre comme avant |
| le dialogue d'alignement s'ouvre sur elle | il s'ouvre sur la cadence du projet |

**Rien d'autre n'en dépend, et aucune opération ne se refuse.** Le lecteur, lui,
n'a pas besoin de `ffprobe` du tout : il s'appuie sur `libmpv`, qui est une
dépendance de compilation. La durée du film, celle qui sert à signaler ce qui
dépasse la fin, vient du lecteur et non de `ffprobe`.

Ce n'est donc pas un mode dégradé mais **le fonctionnement normal** d'une machine
qui n'a jamais installé `ffmpeg` : la donnée vient de l'utilisateur, et `ffprobe`
la propose quand il est là.

Une fréquence lue est un **rationnel exact** — `24000/1001`, et non `23.976`.
C'est la raison pour laquelle `ffprobe` est gardé pour cette réponse-là : le
lecteur connaît la même chose sous forme de nombre à virgule, et une fréquence
approchée re-calerait tout le fichier à côté.
