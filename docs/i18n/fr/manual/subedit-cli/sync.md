# `sync`

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Recale un fichier de sous-titres sur le son d'un film.** Le cas qui l'appelle est le `.srt` récupéré en ligne, décalé
au début et à la fin, qu'un simple décalage ne ramène pas : chaque sous-titre est cherché dans ce qui est dit, et prend
la place où il est dit.

Elle écoute le son avec un moteur de transcription — le même que [`transcribe`](transcribe.md) — et **apparie les mots
de chaque sous-titre aux mots qu'elle a entendus**. Rien n'est traduit : les sous-titres doivent être dans la langue
qui est parlée.

**Rien de ce dont elle a besoin n'est livré avec le programme** : le moteur, un modèle et `ffmpeg` sont ceux de
[Installer les outils de transcription](install-transcription.md), et leur absence ne casse rien — la commande dit ce
qui manque, avec le renvoi vers cette page, n'écrit rien et sort avec un code non nul.

- **Seuls les débuts et les fins changent.** Jamais le texte, le style, le nombre de sous-titres ni leur ordre.
- **Un sous-titre que le son ne donne pas garde ses temps** — un générique que personne ne dit, une chanson, une
  réplique dans une autre langue. **Aucune position n'est jamais inventée, ni devinée d'après les voisins** : la ligne
  reste là où le fichier l'avait mise, et la commande le dit. Ce n'est *pas* un échec, et le code de sortie est `0`.
- **Un sous-titre est trouvé quand la moitié au moins de ses mots est entendue** (`--min-match`,
  `alignment.min-match`), et un mot au moins. La casse, les accents, la ponctuation et le balisage ne comptent pas ; un
  mot qui s'écarte d'une lettre d'un mot long compte comme entendu.
- **La fin est là où le dernier mot finit.** Le moteur date les mots approximativement, quelques centièmes de seconde
  avant la fin propre du sous-titre : un sous-titre peut donc sortir un peu plus court qu'il n'était.
- **Un sous-titre qui rentrerait dans le suivant est raccourci jusqu'au début du suivant.** Celui qui chevauche encore
  le suivant, ce qu'un raccourcissement ne guérit pas, est laissé et signalé.
- **Un fichier dont aucun sous-titre n'a été trouvé est refusé et n'est pas écrit** : c'est presque sûrement le mauvais
  son ou la mauvaise langue.
- **On peut n'écouter qu'une partie du film**, avec `--start` et `--end`, comme pour
  [`transcribe`](transcribe.md#transcrire-une-partie) : seuls les sous-titres qui **commencent** dans la plage sont
  cherchés et touchés, les autres sont laissés tels quels et ne sont pas comptés.
- **Le son est écouté une fois pour chaque fichier donné.** Pour recaler plusieurs fichiers sur le même film, comptez
  l'attente de chacun.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments et options

| Option | Obligatoire | Valeurs | Défaut |
| :----- | :---------- | :------ | :----- |
| `<files>` | oui | les fichiers de sous-titres à recaler ; avec `-r`, des répertoires | — |
| `--recursive`, `-r` | non | un drapeau | désactivé — voir [Traiter un arbre](lots.md) |
| `--video` | l'une de `--video` et `--audio` | le film à écouter | — |
| `--audio` | l'une de `--video` et `--audio` | un fichier audio à écouter, à la place d'un film | — |
| `--audio-track` | non | un nombre entier depuis 0 : la position parmi les pistes audio du film | `0`, la première |
| `--language` | non | deux lettres, un code ISO 639-1 : `en`, `fr` | le réglage `transcription.language` ; vide : détectée |
| `--start`, `--end` | non | un temps : `2:00`, `1:02:03`, `2:00.5`, ou un nombre de secondes ; `--end` doit venir après `--start` | tout le son |
| `--model` | non | le fichier de modèle | le réglage `transcription.model` |
| `--vad-model` | non | un modèle d'activité vocale, qui fait sauter les silences au moteur ; avec lui les mots suivent la parole de plus près | le réglage `transcription.vad-model` |
| `--engine` | non | le nom du moteur qui écoute : `whisper.cpp` | le réglage `transcription.engine` |
| `--keep-audio` | non | un fichier où garder le son extrait ; son répertoire est créé s'il manque | le son n'est gardé nulle part |
| `--min-match` | non | un nombre entier de 1 à 100 : la part des mots d'un sous-titre qui doit être entendue, en pourcentage | le réglage `alignment.min-match`, `50` |
| `--doubt-gap` | non | un nombre entier de millisecondes, 0 ou plus : de combien le décalage d'un sous-titre peut différer de celui de ses voisins avant d'être signalé | le réglage `alignment.doubt-gap`, `2000` |
| `--report` | non | un fichier où écrire les sous-titres à vérifier, un par ligne : le numéro du sous-titre, une tabulation, et pourquoi ; un seul fichier de sous-titres à la fois | rien n'est écrit |
| `--output` / `--output-dir` / `--in-place` | **l'une des trois**, sauf avec `--dry-run` | voir [Invocation](invocation.md#la-destination) | — |
| `--dry-run` | non | un drapeau : calcule et dit, n'écrit rien — voir [Voir avant d'écrire](invocation.md#voir-avant-décrire) | désactivé |

La destination — `--output`, `--output-dir`, `--in-place`, `--dry-run` — et `-r` sont ceux des autres commandes qui
réécrivent un fichier : voir [la destination](invocation.md#la-destination). Le fichier garde son format, son
encodage et ses fins de ligne.

Les **réglages** se lisent dans le fichier de réglages — voir [le fichier de réglages](invocation.md#le-fichier-de-réglages) —
et chacun a son option ici. Le moteur, le modèle et le modèle d'activité vocale sont ceux de `transcribe`.

| Clé | Sens | Défaut |
| :-- | :--- | :----- |
| `alignment.engine` | le moteur d'alignement ; il y en a un, `transcript-matching`, qui écoute et apparie | `transcript-matching` |
| `alignment.min-match` | la part des mots d'un sous-titre qui doit être entendue pour qu'il soit trouvé, en pourcentage | `50` |
| `alignment.doubt-gap` | de combien, en millisecondes, le décalage d'un sous-titre peut s'écarter de celui de ses voisins avant d'être signalé | `2000` |

## Ce qui est signalé

La commande dit quels sous-titres vérifier, et pourquoi. Un sous-titre est signalé pour la **première** de ces raisons
qui tient :

| Pourquoi | Sens |
| :------- | :--- |
| introuvable dans le son, laissé tel quel | moins de `--min-match` de ses mots ont été entendus : il garde ses temps |
| entendu en partie seulement | trouvé, mais moins de 80 % de ses mots ont été entendus |
| déplacé autrement que ses voisins | trouvé, mais son décalage s'écarte du décalage médian des deux sous-titres de chaque côté de plus de `--doubt-gap` |

Un sous-titre peut aussi être **raccourci jusqu'au début du sous-titre suivant**, ou **chevaucher encore le sous-titre
suivant**.

Deux chiffres racontent l'histoire du fichier entier. **Le décalage médian** est le milieu de ce dont les sous-titres
ont bougé : un fichier simplement en retard se lit ici, et nulle part ailleurs. **La dérive** est le décalage médian du
dernier dixième des sous-titres moins celui du premier dixième : elle est proche de zéro pour un décalage constant, et
grande pour un fichier de plus en plus en retard le long du film.

## Sortie

**Sortie standard** — rien : le résultat est le fichier écrit.

| Niveau | Ce qui est dit sur la sortie d'erreur |
| :----- | :------------------------------------ |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | l'étape et la progression, comme pour [`transcribe`](transcribe.md#sortie) |

### En JSON

Avec [`--format json`](invocation.md#sortie-lisible-par-un-script), l'objet du fichier porte `counts` : `subtitles`
(cherchés), `placed` (ayant reçu une place), `moved` (dont les temps ont changé), `partial`, `out_of_line`,
`not_found`, `median_shift_ms` et `drift_ms` ; et `flagged`, une liste de `{"subtitle": N, "kind": "…"}` où le
numéro est la place du sous-titre dans le fichier et `kind` l'un de `not-found`, `partial`, `out-of-line`, `cut-back`
et `overlaps`. Les identifiants sont promis ; les phrases ne le sont pas.

## Exemple

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Codes de sortie

Ceux de l'outil : `0` si le fichier a été écrit, sous-titres signalés ou non, `2` s'il n'a pas pu l'être — un outil ou
un modèle manquant, une extraction ou une écoute qui échoue, aucun sous-titre trouvé —, `1` sur une erreur d'usage,
comme ni `--video` ni `--audio` (ou les deux), une option hors de son intervalle, ou une plage inutilisable.

## Erreurs

| Cause | Message |
| :---- | :------ |
| ni `--video` ni `--audio`, ou les deux | `sync needs the audio to listen to: give --video or --audio, and not both` |
| aucun programme de moteur trouvé, ou celui qui est configuré n'est pas là | `the transcription engine was not found`, puis le renvoi vers le guide |
| aucun modèle donné, ou il n'est pas là | `no model was given, or the model file is not there`, puis le renvoi vers le guide |
| `ffmpeg` introuvable pour un film | `ffmpeg was not found, so the audio of a video cannot be extracted`, puis le renvoi |
| une piste que le film n'a pas | `the video has no audio track N`, puis les pistes qu'elle a |
| une langue que le modèle ne sait pas transcrire | `the model cannot transcribe the language xx` |
| un moteur que personne n'a enregistré | `no transcription engine is called NAME`, ou `there is no alignment engine called NAME` |
| aucun sous-titre trouvé | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` hors de 1 à 100, `--doubt-gap` négatif | l'erreur d'usage de l'option |
| `--report` avec plus d'un fichier de sous-titres | `--report names one file: give one subtitle file when you use it` |
