# `transcribe`

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Écrit **la parole d'une vidéo, ou d'un fichier audio, sous forme de fichier de sous-titres**. L'audio est préparé,
confié à un moteur qui trouve ce qui est dit et quand, et les morceaux qu'il découpe sont fusionnés en
sous-titres qui respectent une durée et une longueur.

**Rien de ce dont elle a besoin n'est livré avec le programme** : le moteur (whisper.cpp), un modèle et `ffmpeg`
sont installés par l'utilisateur, qui dit où se trouve le modèle. [Installer les outils de
transcription](install-transcription.md) dit où trouver chacun, et quel modèle choisir. **Leur absence ne casse
rien** : la commande dit ce qui manque, avec le renvoi vers cette page, n'écrit rien et
sort avec un code non nul.

- **L'entrée est une vidéo ou un fichier audio.** D'une vidéo, `ffmpeg` extrait une piste audio ; sans
  `ffmpeg`, un fichier audio que le moteur lit tel quel (`wav`, `mp3`, `flac`, `ogg`) est tout de même
  transcrit, mais pas une vidéo, ni aucun fichier quand une piste est demandée.
- **Un seul fichier à la fois**, et la destination est `--output` : il n'y a ni `--output-dir` ni
  `--in-place`, puisque le résultat est un nouveau fichier et non une réécriture.
- **La langue est détectée** à moins que `--language` ne la force, et la langue employée est dite. Un
  modèle fait pour l'anglais seul ne détecte rien et ne transcrit rien d'autre : une langue qu'il ne peut
  prendre est refusée *avant* tout travail, en nommant la limite du modèle.
- **Les segments sont fusionnés et coupés** selon les limites des réglages (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`) : deux morceaux d'une même phrase qui sont
  proches font un seul sous-titre, et un morceau trop long ou trop large est coupé à un mot. **`--raw`
  écrit les segments du moteur tels qu'ils sont venus**, un par sous-titre, ce qui est aussi la façon de voir ce
  que fait la fusion.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<input>` | oui | une vidéo ou un fichier audio | — |
| `--output` | oui | le fichier de sous-titres à écrire ; son dossier est créé s'il manque | — |
| `--to` | non | le nom court d'un format : `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | le format que dit l'extension de `--output`, sinon `srt` |
| `--audio-track` | non | un nombre entier à partir de 0 : la position parmi les pistes audio de la vidéo | `0`, la première |
| `--language` | non | deux lettres minuscules, un code ISO 639-1 : `en`, `fr` | le réglage `transcription.language` ; vide : détectée |
| `--model` | non | le fichier de modèle | le réglage `transcription.model` |
| `--engine` | non | le nom d'un moteur : `whisper.cpp` | le réglage `transcription.engine` |
| `--keep-audio` | non | un fichier où garder l'audio extrait, en PCM 16 kHz mono 16 bits dans un fichier WAV ; son dossier est créé s'il manque | l'audio n'est gardé nulle part |
| `--raw` | non | un drapeau : écrire les segments du moteur sans fusion ni coupe | désactivé |

Les **réglages** sont lus dans le fichier de réglages comme ceux de [`adjust`](adjust.md) — voir
[le fichier de réglages](invocation.md#le-fichier-de-réglages) — et chacun a son option ici :

| Clé | Sens | Défaut |
| :-- | :--- | :----- |
| `transcription.engine` | le moteur | `whisper.cpp` |
| `transcription.executable` | le programme du moteur, un chemin absolu ; vide : cherché dans le `PATH` | vide |
| `transcription.model` | le fichier de modèle, un chemin absolu | vide |
| `transcription.language` | la langue de l'audio, deux lettres ; vide : détectée | vide |
| `transcription.merge-gap-ms` | les morceaux d'une même phrase plus proches que ce nombre de millisecondes sont joints | `400` |
| `transcription.max-duration-ms` | la durée maximale d'un sous-titre à l'écran, en millisecondes | `7000` |
| `transcription.max-characters` | le nombre maximal de caractères d'un sous-titre, sauts de ligne non comptés | `84` |

## Choisir l'audio

Une vidéo peut porter plusieurs pistes audio — l'originale, un commentaire, une autre langue. `--audio-track`
prend la **position parmi les pistes audio** du fichier, à partir de 0, dans l'ordre où le fichier les liste. Une
piste que la vidéo n'a pas est refusée, **avec celles qu'elle a**, pour que l'appel suivant soit
le bon :

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | la progression du moteur, `transcribing: 25%`, telle qu'il la rapporte — par pas d'environ trente secondes d'audio |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script) (l'option globale, écrite avant la
sous-commande), l'objet du fichier porte la langue employée dans `language`, `counts.subtitles` (les
sous-titres écrits) et `counts.segments` (les morceaux du moteur dont ils sont faits), et `audio` quand
`--keep-audio` a gardé l'audio.

## Codes de retour

Ceux de l'outil : `0` si le fichier a été écrit, `2` s'il n'a pas pu l'être — un outil ou un modèle absent, une
langue que le modèle ne peut prendre, une extraction ou une transcription qui échoue —, `1` sur une erreur d'usage, comme un
moteur que personne n'a enregistré.

## Erreurs

| Cause | Message |
| :---- | :------ |
| aucun programme de moteur trouvé, ou celui qui est configuré n'existe pas | `the transcription engine was not found`, puis le renvoi vers le guide |
| aucun modèle donné, ou il n'existe pas | `no model was given, or the model file is not there`, puis le renvoi vers le guide |
| `ffmpeg` introuvable pour une vidéo | `ffmpeg was not found, so the audio of a video cannot be extracted`, puis le renvoi |
| une piste que la vidéo n'a pas | `the video has no audio track N`, puis les pistes qu'elle a |
| une langue que le modèle ne peut pas transcrire | `the model cannot transcribe the language xx` |
| aucune langue, et un modèle qui ne sait pas en détecter une | `the model cannot detect the language: give one` |
| un moteur que personne n'a enregistré | `no transcription engine is called NAME` |
| tout autre échec | `the transcription failed`, avec ce que le moteur a dit |

Le renvoi vers le guide est `See the manual page "Installing the transcription tools"
(install-transcription).`
