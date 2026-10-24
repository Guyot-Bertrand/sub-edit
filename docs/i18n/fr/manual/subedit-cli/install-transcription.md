# Installer les outils de transcription

Transcrire de l'audio en sous-titres — [`transcribe`](transcribe.md) en ligne de commande,
[`Transcribe Audio…`](../subedit-gui/video.md#transcrire-laudio) dans la fenêtre — demande trois
choses que **subedit ne livre pas et ne télécharge pas** : un moteur, un modèle pour ce moteur, et
`ffmpeg`. Vous les installez, et vous dites à subedit où se trouve le modèle.

**Rien ne casse sans eux.** Le reste du programme fonctionne comme avant ; seule la transcription
se désactive, avec un message qui renvoie ici.

## Le moteur : whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) est le moteur que subedit lance. C'est un programme,
`whisper-cli`, qui tourne sur un processeur ordinaire — une carte graphique ne fait que l'accélérer. Construisez-le
comme le dit sa propre documentation (un compilateur C++ et CMake suffisent) :

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Le programme est alors `build/bin/whisper-cli`. Construit avec `-DBUILD_SHARED_LIBS=OFF`, comme ci-dessus, c'est
un seul fichier qui n'a besoin d'aucune bibliothèque à côté de lui, et que vous pouvez donc copier n'importe où. Mettez-le là où votre `PATH` le trouve, ou
**donnez son chemin** à subedit avec le réglage `transcription.executable` — dans la fenêtre, les
[préférences](../subedit-gui/preferences.md#transcription) ont un champ pour cela. Une construction pour carte graphique ajoute
une option à la première ligne `cmake` : `-DGGML_CUDA=1` pour une carte NVIDIA, `-DGGML_VULKAN=1` pour la plupart des autres ;
subedit lance celle que vous avez installée et ne choisit rien à votre place.

Pour ne garder que le programme, **installez-le** puis supprimez le clone :

```bash
cmake --install build --prefix ~/.local
```

`--prefix` dit où. `~/.local` n'exige aucun droit d'administrateur, et son répertoire `bin` doit être dans votre `PATH` ;
sans l'option, le défaut est `/usr/local`, qui demande `sudo`. `whisper-cli` se retrouve dans le répertoire `bin`
de ce préfixe, à côté d'outils que subedit n'utilise pas ; une fois qu'il y est, le clone et son répertoire `build` peuvent partir.

## Un modèle

Le moteur lit un **modèle**, un fichier de plusieurs centaines de mégaoctets à quelques gigaoctets que vous
téléchargez une fois. Les modèles officiels sont listés sur
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), et le
dépôt du moteur a un script qui en récupère un :

```bash
sh ./models/download-ggml-model.sh small
```

Donnez ensuite son chemin avec `--model` en ligne de commande, ou avec le réglage `transcription.model`,
ou dans les [préférences](../subedit-gui/preferences.md#transcription) de la fenêtre, qui s'en souviennent.

### Quel modèle pour quelle langue

Whisper ne change pas de modèle une fois la langue trouvée : **le modèle que vous donnez décide de ce qu'il
sait faire.**

| Modèle | Langues | Détecte la langue | Taille |
| :----- | :------ | :---------------- | :----- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | une centaine | oui | d'environ 75 Mo à environ 3 Go |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | l'anglais seul | non | comme la même taille ci-dessus |

- **Pour tout autre chose que l'anglais, prenez un modèle multilingue** — dont le nom n'a pas de `.en`. Un modèle
  réservé à l'anglais refuse une autre langue : la commande le dit avant de faire le moindre travail.
- **Les plus petits modèles sont nettement moins bons hors de l'anglais.** Pour le français, l'allemand, l'espagnol
  et leurs semblables, `small` est un minimum, et `medium` ou `large-v3-turbo` donnent de bien meilleurs résultats au prix
  de plus de mémoire et de plus de temps.
- **La détection de la langue vaut ce que vaut le modèle.** Un petit modèle peut se tromper sur un début fait de
  musique ou de silence ; dites la langue avec `--language` quand vous la connaissez.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extrait la piste audio d'une vidéo, et `ffprobe`, qui l'accompagne,
liste les pistes. Installez-le avec le gestionnaire de paquets de votre distribution. Sans lui, un fichier
audio que le moteur lit tel quel (`wav`, `mp3`, `flac`, `ogg`) peut encore être transcrit, mais pas une vidéo.

## Vérifier que cela marche

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Si quelque chose manque, le message dit quoi, et renvoie à cette page.
