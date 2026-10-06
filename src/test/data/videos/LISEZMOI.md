# Fixtures vidéo

Deux conteneurs minuscules, dont on ne lit jamais une image : ce qu'on y lit,
c'est **une fréquence d'image et une durée**, les deux métadonnées dont la
phase 6 a besoin et qu'un fichier de sous-titres ne déclare pas.

| Fichier | Fréquence | Durée | Poids |
| :------ | :-------- | ----: | ----: |
| `cadence-25.mp4` | `25/1` — PAL, entière | 2,000 s | 1 504 o |
| `cadence-23-976.mp4` | `24000/1001` — NTSC, fractionnaire | 2,002 s | 1 519 o |

La seconde est celle qui dit quelque chose. Une fixture à fréquence entière
seule laisserait croire à une lecture juste là où le noyau manipule des
rationnels exacts depuis la phase 1.

**Ne pas les éditer à la main, et ne pas les croire sur parole.**
[`src/scripts/video-fixtures.sh`](../../../scripts/video-fixtures.sh) porte la
commande qui les fabrique et la table de ce qu'on en attend :

```console
$ ./src/scripts/video-fixtures.sh --check      # ce que ffprobe en dit, contre la table
$ ./src/scripts/video-fixtures.sh --generate   # les refabriquer
```

`--check` tourne dans `make check-local`. C'est là qu'est la garantie : un
conteneur est illisible dans un diff, et personne ne relira ces 3 Ko.

## Les images numérotées — #610

Deux autres fixtures, d'un autre usage : **chaque image y porte son propre numéro**, et c'est ce qu'on lit.

| Fichier | Fréquence | Images | Image-clé | Poids |
| :------ | :-------- | -----: | :-------- | ----: |
| `images-25.mp4` | `25/1` | 250 | **une seule, au début** | 13 275 o |
| `images-23-976.mp4` | `24000/1001` | 240 | **une seule, au début** | 13 062 o |

128×64, dix secondes. **Le numéro est écrit en huit barres de 16 pixels**, chacune claire pour un bit à 1
et sombre pour un bit à 0, le bit de poids faible à gauche : de 0 à 255. Un test qui a placé la lecture à une
image **lit l'image que le lecteur affiche** (`MpvPlayer::picture()`) et compare le numéro à celui qu'il avait demandé —
un oracle qui ne repose pas sur ce que le lecteur dit de lui-même, car sa position et son compteur d'images
viennent du même endroit que l'image, et un lecteur qui montrerait la mauvaise image en annonçant la bonne heure les
satisferait.

**Une seule image-clé** : c'est le cas difficile, celui d'un film réel — la position 249 se décode depuis la 0.
L'encodeur `mpeg4` n'en donne pas une seule par défaut : malgré `-g 250` il en place une toutes les 32 images quand
le contenu change à chaque image (huit sur 250, mesuré), d'où `-keyint_min 250 -sc_threshold 1000000000 -bf 0`.

**La seconde est à 24000/1001** : l'image 10 y occupe 417,08 à 458,79 ms, que la milliseconde entière n'écrit pas.
C'est ce qui montre que la finesse du `Timestamp` n'est pas le défaut — toutes les images démarrent juste — et
que **`seek` place la lecture sur l'image la plus proche**, non sur celle qui est affichée à cet instant.

`--check` vérifie, pour chacune, la fréquence, le nombre d'images, **le nombre d'images-clés**, la taille, et que
**la première image, la deuxième, celle du milieu et la dernière portent leur numéro — lu par ffmpeg**, sans libmpv : ce
qu'un test lira dans l'image ne vaut que si la fixture qu'il lit est honnête.

## Les pistes audio — #614

Deux fixtures encore, qui ne montrent rien (un écran noir de 16×16, deux secondes) et **portent du son** : ce qu'on y
lit, c'est la liste des pistes que le lecteur rend au menu `Audio`.

| Fichier | Pistes (langue : titre) | Poids |
| :------ | :---------------------- | ----: |
| `audio-1.mkv` | `fra` : « Original » | 6 124 o |
| `audio-2.mkv` | `fra` : « Original », `eng` : « Commentary » | 10 900 o |

**Une vidéo sans piste** est `cadence-25.mp4`, qui n'en a jamais eu : c'est le cas « ne casse rien » du lecteur.

`sound-only.mkv` (5 072 o) est l'inverse : **du son et aucune image**. Il n'a pas de fréquence d'image, donc rien à avancer
d'un pas — le cas où `stepFrames` ne fait rien plutôt que de se tromper.

**En Matroska, et non en MP4 comme les autres** : le MP4 n'a pas de titre de piste — ffmpeg y écrit un `handler_name` —,
et le titre est précisément ce que ces fixtures doivent porter. Les horodatages que Matroska arrondit à la milliseconde
ne gênent pas ici : aucune image n'y est lue.

`--check` vérifie, pour chacune, la langue et le titre de chaque piste, dans l'ordre, et la taille.

## La vidéo du banc — #611

**Une dernière vidéo, qui n'est pas dans ce répertoire** : `make bench` la fabrique dans l'arbre de construction
(`build/release/bench-film.mp4`, `video-fixtures.sh --film`), parce qu'un film de cette taille n'a pas sa place dans un
dépôt. Elle suit la logique des images numérotées : **1280×720, 250 images à 25 par seconde, une seule image-clé**,
chaque image portant son numéro (huit barres de 160 pixels). `--film` vérifie qu'elle est honnête comme `--check` le fait
des autres, et ne la refait pas si elle l'est déjà. Sans ffmpeg, le banc du saut et du pas le dit et s'abstient.
