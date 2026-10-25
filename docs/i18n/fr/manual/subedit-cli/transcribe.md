# `transcribe`

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **Une langue se tape sur deux lettres — `fr`, `en`, `de` —, ou sur le code de trois lettres qu'un fichier déclare, `fra`,
  `eng`, lu comme celui de deux lettres.** Tout le reste est refusé avant tout travail. **La détection n'écoute que les
  trente premières secondes** : un film qui s'ouvre sur de la musique ou du silence peut être pris pour une autre langue, et le
  moteur écrit alors la parole *traduite* dans cette langue — un film français pris pour de l'anglais sort en anglais, ou en
  `[SPEAKING FRENCH]`. Quand vous connaissez la langue, donnez-la.
- **On peut transcrire une partie du film seule**, avec `--start` et `--end` — voir
  [Transcrire une partie](#transcrire-une-partie). C'est ainsi qu'on essaie un modèle sur deux minutes avant de le
  lancer sur deux heures.
- **Les langues qu'un moteur offre peuvent être listées**, avec `--list-languages` — voir
  [Lister les langues](#lister-les-langues).
- **Les sons ne sont pas écrits comme des notes, par défaut** : le moteur a ordre de ne pas le faire (`--suppress-notes` ; son option est
  `--suppress-nst`), de sorte que `(Sonnerie de téléphone)` ou `[MUSIC PLAYING]` ne remontent pas dans les sous-titres, entre parenthèses comme entre
  crochets. `--keep-notes` les écrit, pour des sous-titres pour sourds et malentendants. Les notes interdites, le moteur écrit `...` pour un passage
  sans parole, et **le post-traitement écarte les morceaux qui n'ont ni lettre ni chiffre** — `...`, `…`, `♪` — avant de fusionner et de couper ;
  `--raw` les garde, tels que les morceaux du moteur sont venus.
- **Les segments sont fusionnés et coupés** selon les limites des réglages (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`) : deux morceaux d'une même phrase qui sont
  proches font un seul sous-titre, et un morceau trop long ou trop large est coupé — à une fin de phrase d'abord, puis à une virgule, un point-virgule ou deux-points, et au milieu d'une phrase seulement quand il n'y a pas de ponctuation où couper —, en évitant une coupe qui laisse un sous-titre trop petit pour être lu. Les trois étapes se font dans l'ordre, chacune sur toute la transcription : écarter, fusionner, couper. Un sous-titre qui commence un morceau garde son début, et celui qui le termine garde sa fin. **`--raw`
  écrit les segments du moteur tels qu'ils sont venus**, un par sous-titre, ce qui est aussi la façon de voir ce
  que fait la fusion.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Arguments et options

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `<input>` | oui, sauf avec `--list-languages` | une vidéo ou un fichier audio | — |
| `--output` | oui, sauf avec `--list-languages` | le fichier de sous-titres à écrire ; son dossier est créé s'il manque | — |
| `--to` | non | le nom court d'un format : `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | le format que dit l'extension de `--output`, sinon `srt` |
| `--audio-track` | non | un nombre entier à partir de 0 : la position parmi les pistes audio de la vidéo | `0`, la première |
| `--language` | non | deux lettres, un code ISO 639-1 : `en`, `fr` ; un code de trois lettres comme `fra` est lu comme `fr` | le réglage `transcription.language` ; vide : détectée |
| `--suppress-notes` | non | un drapeau : ne pas écrire les sons comme des notes ; refusé pour un moteur à qui on ne peut le dire | le réglage `transcription.suppress-notes` |
| `--keep-notes` | non | un drapeau : les écrire, contre le réglage ; l'opposé de `--suppress-notes`, et les deux ensemble sont refusés | le réglage `transcription.suppress-notes` |
| `--list-languages` | non | un drapeau : lister les langues que le moteur offre pour le modèle, et ne rien transcrire | désactivé |
| `--start` | non | une durée : `2:00`, `1:02:03`, `2:00.5`, ou un nombre de secondes, `90` | le début de l'audio |
| `--end` | non | une durée, écrite comme pour `--start` ; elle doit venir après `--start` | la fin de l'audio |
| `--model` | non | le fichier de modèle | le réglage `transcription.model` |
| `--engine` | non | le nom d'un moteur : `whisper.cpp` | le réglage `transcription.engine` |
| `--keep-audio` | non | un fichier où garder l'audio extrait, en PCM mono 16 bits dans un fichier WAV, à la fréquence d'échantillonnage que demande le moteur (16 kHz pour whisper.cpp) ; son dossier est créé s'il manque, et le nom peut porter `{rate}`, remplacé par cette fréquence en hertz (`film.{rate}.wav` s'écrit `film.16000.wav`) | l'audio n'est gardé nulle part |
| `--raw` | non | un drapeau : écrire les segments du moteur sans fusion ni coupe | désactivé |

Les **réglages** sont lus dans le fichier de réglages comme ceux de [`adjust`](adjust.md) — voir
[le fichier de réglages](invocation.md#le-fichier-de-réglages) — et chacun a son option ici :

| Clé | Sens | Défaut |
| :-- | :--- | :----- |
| `transcription.engine` | le moteur | `whisper.cpp` |
| `transcription.executable` | le programme du moteur, un chemin absolu ; vide : cherché dans le `PATH` | vide |
| `transcription.model` | le fichier de modèle, un chemin absolu | vide |
| `transcription.language` | la langue de l'audio, deux lettres ; vide : détectée | vide |
| `transcription.suppress-notes` | ne pas écrire les sons comme des notes | `true` |
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

## Transcrire une partie

`--start` et `--end` limitent le travail à la partie du film qui les sépare. L'un peut être donné seul : un `--start`
seul va jusqu'à la fin, un `--end` seul commence au début.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Une durée s'écrit `M:SS` ou `H:MM:SS`, avec au besoin une partie décimale après un point (`2:00.5`), ou comme un nombre de
  secondes (`90`, `90.5`)**. Une virgule est refusée, comme partout dans l'outil.
- **Les sous-titres gardent le temps du film** : la phrase dite à 2:10 du film est écrite à `00:02:10`, non à
  `00:00:10`. Le fichier se pose tel quel sur le film entier.
- **Les bords sont approximatifs** : le moteur travaille par fenêtres de trente secondes, ce qu'il rend est donc coupé
  à la plage après coup ; une phrase à cheval sur un bord est coupée là.
- **Rien ne démarre si la plage est inutilisable** : une borne qui n'est pas une durée, ou un `--end` qui ne vient pas après
  `--start`, est une erreur d'usage qui nomme le problème, n'écrit rien et ne lance ni `ffmpeg` ni le moteur.

L'audio du film entier est quand même extrait : c'est la transcription, bien plus longue, que la plage économise.

## Lister les langues

`--list-languages` écrit les langues que le moteur offre **pour le modèle**, une par ligne : le code que `--language` prend, une tabulation, et le nom de
la langue dans la langue de l'interface. Il ne demande ni entrée ni sortie, et ne transcrit rien.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

La liste dépend du moteur et du modèle : une centaine de langues pour un modèle whisper.cpp multilingue, `en` seul pour un modèle pour l'anglais seul. Un moteur
qui ne peut lister ses langues n'en liste aucune. Un modèle non donné ou absent est une erreur, avec le renvoi vers le guide ; la forme texte seulement, rien n'est
écrit avec [`--format json`](invocation.md#sortie-lisible-par-un-script).

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui s'ajoute sur la sortie d'erreur |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | l'étape et la progression, une ligne par pour cent : `extracting the audio…`, puis `transcribing…`, puis `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — le pour cent, la position atteinte dans l'audio, le temps restant, et le temps écoulé ; **la ligne est redite toutes les dix secondes quand rien de neuf n'est venu**, pour qu'une longue attente ne soit pas prise pour un blocage |
| 3 | et chaque nouveau texte que le moteur obtient, `last text: …` |

**Le pour cent est la position du moteur dans l'audio, non un compte de ses propres fenêtres** : le moteur écrit ce qu'il a
transcrit au fur et à mesure, et la commande lit où il en est, de sorte que le chiffre bouge dès la première minute. (La
progression propre du moteur vient par pas de 5 % de l'audio, après une fenêtre de trente secondes : seule, elle peut rester
à 0 % un quart d'heure sur un long film avec un grand modèle sur processeur.) **Le temps restant est muet au début** : il est
donné quand assez de travail est fait pour extrapoler, et il suit la vitesse de la dernière minute, non la moyenne de toute la
course.

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script) (l'option globale, écrite avant la
sous-commande), l'objet du fichier porte la langue employée dans `language`, `counts.subtitles` (les
sous-titres écrits) et `counts.segments` (les morceaux du moteur dont ils sont faits), et `audio` quand
`--keep-audio` a gardé l'audio.

## Avertissements

Un avertissement est dit au premier niveau, avant le travail, car il change ce qu'on doit attendre :

- **Avec un modèle pour l'anglais seul** : `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Un modèle dont le
  nom de fichier se termine par `.en` en est un.

## Codes de retour

Ceux de l'outil : `0` si le fichier a été écrit, `2` s'il n'a pas pu l'être — un outil ou un modèle absent, une
langue que le modèle ne peut prendre, une extraction ou une transcription qui échoue —, `1` sur une erreur d'usage, comme un
moteur que personne n'a enregistré ou une plage inutilisable.

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
| `<input>` ou `--output` manquant | `input is required` ou `--output is required` |
| `--suppress-notes` avec `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` pour un moteur à qui on ne peut le dire | `the engine NAME cannot be told not to write sounds as notes` |
| une langue qui n'est pas un code | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| une borne de la plage qui n'est pas une durée | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` pas après `--start` | `the end of the range must come after its start` |
| tout autre échec | `the transcription failed`, avec ce que le moteur a dit |

Le renvoi vers le guide est `See the manual page "Installing the transcription tools"
(install-transcription).`
