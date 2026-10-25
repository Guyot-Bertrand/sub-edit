# `transcribe`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

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

Escriu **la parla d'un vídeo, o d'un fitxer d'àudio, en forma de fitxer de subtítols**. L'àudio es prepara,
es confia a un motor que troba què es diu i quan, i els trossos que talla es fusionen en subtítols que
respecten una durada i una llargada.

**Res del que necessita no es lliura amb el programa**: el motor (whisper.cpp), un model i `ffmpeg`
els instal·la l'usuari, que diu on és el model. [Instal·lació de les eines de
transcripció](install-transcription.md) diu on trobar-ne cadascun, i quin model triar. **La seva absència no trenca
res**: l'ordre diu què falta, amb el punter cap a aquesta pàgina, no escriu res i
surt amb un codi diferent de zero.

- **L'entrada és un vídeo o un fitxer d'àudio.** D'un vídeo, `ffmpeg` n'extreu una pista d'àudio; sense
  `ffmpeg`, un fitxer d'àudio que el motor llegeix tal qual (`wav`, `mp3`, `flac`, `ogg`) es transcriu igualment,
  però no un vídeo, ni cap fitxer quan es demana una pista.
- **Un sol fitxer alhora**, i la destinació és `--output`: no hi ha ni `--output-dir` ni
  `--in-place`, ja que el resultat és un fitxer nou i no una reescriptura.
- **La llengua es detecta** tret que `--language` la imposi, i es diu la llengua emprada. Un
  model fet només per a l'anglès no detecta res i no transcriu res més: una llengua que no pot
  acceptar es refusa *abans* de començar cap feina, indicant el límit del model.
- **Una llengua s'escriu amb dues lletres — `fr`, `en`, `de` —, o amb el codi de tres lletres que declara un fitxer, `fra`,
  `eng`, que es llegeix com el de dues lletres.** Tota la resta es refusa abans de començar cap feina. **La detecció
  només escolta els primers trenta segons**: una pel·lícula que comença amb música o silenci pot ser presa per
  una altra llengua, i el motor escriu llavors la parla *traduïda* a aquesta llengua — una pel·lícula francesa presa
  per anglesa surt en anglès, o com a `[SPEAKING FRENCH]`. Quan coneixeu la llengua, doneu-la.
- **Es pot transcriure una part de la pel·lícula tota sola**, amb `--start` i `--end` — vegeu
  [Transcriure una part](#transcriure-una-part). Així es prova un model en dos minuts abans d'executar-lo en
  dues hores.
- **Les llengües que ofereix un motor es poden llistar**, amb `--list-languages` — vegeu
  [Llistar les llengües](#llistar-les-llengües).
- **Els sons no s'escriuen com a notes, per defecte**: al motor se li diu que no ho faci (`--suppress-notes`; la seva opció és
  `--suppress-nst`), de manera que `(Sonnerie de téléphone)` o `[MUSIC PLAYING]` no surten als subtítols, ni entre parèntesis ni entre claudàtors.
  `--keep-notes` els escriu, per a subtítols per a persones sordes o amb dificultats auditives. Prohibides les notes, el motor escriu `...` per a un
  passatge sense parla, i **el postprocessament descarta els trossos que no tenen cap lletra ni cap xifra** — `...`, `…`, `♪` —
  abans de fusionar i tallar; `--raw` els conserva, tal com han vingut els trossos del motor.
- **Els segments es fusionen i es tallen** segons els límits dels paràmetres (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dos trossos d'una mateixa frase que són
  propers fan un sol subtítol, i un tros massa llarg o massa ample es talla en una paraula. **`--raw`
  escriu els segments del motor tal com han vingut**, un per subtítol, que és també la manera de veure què
  fa la fusió.

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

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :------- | :----- | :------ |
| `<input>` | sí, tret de `--list-languages` | un vídeo o un fitxer d'àudio | — |
| `--output` | sí, tret de `--list-languages` | el fitxer de subtítols que cal escriure; el seu directori es crea si falta | — |
| `--to` | no | el nom curt d'un format: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | el format que diu l'extensió de `--output`, si no `srt` |
| `--audio-track` | no | un nombre enter des de 0: el lloc entre les pistes d'àudio del vídeo | `0`, la primera |
| `--language` | no | dues lletres, un codi ISO 639-1: `en`, `fr`; un codi de tres lletres com `fra` es llegeix com `fr` | el paràmetre `transcription.language`; buit: es detecta |
| `--suppress-notes` | no | un indicador: no escriguis els sons com a notes; refusat per a un motor al qual no se li pot dir | el paràmetre `transcription.suppress-notes` |
| `--keep-notes` | no | un indicador: escriu-los, contra el paràmetre; el contrari de `--suppress-notes`, i els dos junts es refusen | el paràmetre `transcription.suppress-notes` |
| `--list-languages` | no | un indicador: llista les llengües que el motor ofereix per al model, i no transcriu res | desactivat |
| `--start` | no | un temps: `2:00`, `1:02:03`, `2:00.5`, o un nombre de segons, `90` | el principi de l'àudio |
| `--end` | no | un temps, escrit com per a `--start`; ha de venir després de `--start` | el final de l'àudio |
| `--model` | no | el fitxer de model | el paràmetre `transcription.model` |
| `--engine` | no | el nom d'un motor: `whisper.cpp` | el paràmetre `transcription.engine` |
| `--keep-audio` | no | un fitxer on conservar l'àudio extret, com a PCM mono de 16 bits en un fitxer WAV, a la freqüència de mostreig que demana el motor (16 kHz per a whisper.cpp); el seu directori es crea si falta, i el nom pot portar `{rate}`, substituït per aquesta freqüència en hertz (`film.{rate}.wav` s'escriu `film.16000.wav`) | l'àudio no es conserva enlloc |
| `--raw` | no | un indicador: escriu els segments del motor sense fusionar ni tallar | desactivat |

Els **paràmetres** es llegeixen del fitxer de configuració com els d'[`adjust`](adjust.md) — vegeu
[el fitxer de configuració](invocation.md#el-fitxer-de-configuració) — i cadascun té aquí la seva opció:

| Clau | Significat | Per defecte |
| :-- | :------ | :------ |
| `transcription.engine` | el motor | `whisper.cpp` |
| `transcription.executable` | el programa del motor, un camí absolut; buit: es cerca al `PATH` | buit |
| `transcription.model` | el fitxer de model, un camí absolut | buit |
| `transcription.language` | la llengua de l'àudio, dues lletres; buit: es detecta | buit |
| `transcription.suppress-notes` | no escriguis els sons com a notes | `true` |
| `transcription.merge-gap-ms` | els trossos d'una mateixa frase més propers que aquests mil·lisegons s'uneixen | `400` |
| `transcription.max-duration-ms` | el màxim que un subtítol es queda a la pantalla, en mil·lisegons | `7000` |
| `transcription.max-characters` | el màxim de caràcters d'un subtítol, sense comptar els salts de línia | `84` |

## Triar l'àudio

Un vídeo pot portar diverses pistes d'àudio — l'original, un comentari, una altra llengua. `--audio-track`
pren el **lloc entre les pistes d'àudio** del fitxer, des de 0, en l'ordre en què el fitxer les llista. Una
pista que el vídeo no té es refusa, **amb les pistes que sí que té**, de manera que la crida següent
sigui l'encertada:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Transcriure una part

`--start` i `--end` restringeixen la feina a la part de la pel·lícula que hi ha entre tots dos. Es pot donar només un:
un `--start` sol va fins al final, un `--end` sol comença pel principi.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Un temps s'escriu `M:SS`, `H:MM:SS`, tots dos amb una part decimal després d'un punt (`2:00.5`), o com un nombre de
  segons (`90`, `90.5`)**. Una coma es refusa, com a tota l'eina.
- **Els subtítols conserven el temps de la pel·lícula**: la rèplica dita a 2:10 de la pel·lícula s'escriu a `00:02:10`, no
  a `00:00:10`. El fitxer es pot superposar tal qual a tota la pel·lícula.
- **Els límits són aproximats**: el motor treballa per finestres de trenta segons, de manera que el que retorna es talla
  després a l'interval; una frase a cavall d'un límit s'hi talla.
- **No comença res si l'interval no es pot fer servir**: un límit que no és un temps, o un `--end` que no ve després de
  `--start`, és un error d'ús que anomena el problema, no escriu res i no executa ni `ffmpeg` ni el
  motor.

L'àudio de tota la pel·lícula s'extreu igualment: és la transcripció, molt més llarga, la que l'interval estalvia.

## Llistar les llengües

`--list-languages` escriu les llengües que el motor ofereix **per al model**, una per línia: el codi que pren `--language`, un tabulador, i el nom
de la llengua en la llengua de la interfície. No necessita ni entrada ni sortida, i no transcriu res.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

La llista depèn del motor i del model: un centenar de llengües per a un model whisper.cpp multilingüe, només `en` per a un model només per a l'anglès. Un motor que no pot llistar les seves llengües no en llista cap. Un model que no s'ha donat o que no hi és és un error, amb el punter a la guia; només
en forma de text, no s'escriu res amb [`--format json`](invocation.md#sortida-llegible-per-un-script).

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | el pas i el progrés, una línia per cada tant per cent: `extracting the audio…`, després `transcribing…`, després `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — el tant per cent, la posició assolida a l'àudio, el temps que queda i el temps transcorregut; **la línia es torna a dir cada deu segons quan no ha arribat res de nou**, perquè una espera llarga no es prengui per un bloqueig |
| 3 | i cada text nou que obté el motor, `last text: …` |

**El tant per cent és la posició del motor a l'àudio, no un recompte de les seves finestres**: el motor escriu el que ha
transcrit a mesura que avança, i l'ordre llegeix fins on ha arribat, de manera que la xifra es mou des del primer
minut. (El progrés propi del motor ve a intervals del 5 % de l'àudio, un cop feta una finestra de trenta segons:
sol, pot quedar-se al 0 % durant un quart d'hora en una pel·lícula llarga amb un model gran en un processador.) **El
temps que queda és en silenci al principi**: es dona quan s'ha fet prou feina per extrapolar, i segueix la velocitat
del darrer minut, no la mitjana de tota l'execució.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script) (l'opció global, escrita abans de la
subordre), l'objecte del fitxer porta la llengua emprada a `language`, `counts.subtitles` (els
subtítols escrits) i `counts.segments` (els trossos del motor a partir dels quals s'han fet), i `audio` quan
`--keep-audio` ha conservat l'àudio.

## Avisos

Un avís es diu al primer nivell, abans de la feina, perquè canvia el que cal esperar:

- **Amb un model només per a l'anglès**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Un model
  el nom de fitxer del qual acaba en `.en` n'és un.

## Codis de sortida

Els de l'eina: `0` si s'ha escrit el fitxer, `2` si no s'ha pogut — una eina o un model que falta, una
llengua que el model no pot acceptar, una extracció o una transcripció fallida —, `1` en un error d'ús, com
un motor que ningú no ha registrat o un interval que no es pot fer servir.

## Errors

| Causa | Missatge |
| :---- | :------ |
| no s'ha trobat cap programa de motor, o el configurat no hi és | `the transcription engine was not found`, i després el punter a la guia |
| no s'ha donat cap model, o no hi és | `no model was given, or the model file is not there`, i després el punter a la guia |
| `ffmpeg` no trobat per a un vídeo | `ffmpeg was not found, so the audio of a video cannot be extracted`, i després el punter |
| una pista que el vídeo no té | `the video has no audio track N`, i després les pistes que té |
| una llengua que el model no pot transcriure | `the model cannot transcribe the language xx` |
| cap llengua, i un model que no en pot detectar cap | `the model cannot detect the language: give one` |
| un motor que ningú no ha registrat | `no transcription engine is called NAME` |
| `<input>` o `--output` que falta | `input is required` o `--output is required` |
| `--suppress-notes` amb `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` per a un motor al qual no se li pot dir | `the engine NAME cannot be told not to write sounds as notes` |
| una llengua que no és un codi | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| un límit de l'interval que no és un temps | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` no posterior a `--start` | `the end of the range must come after its start` |
| qualsevol altre error | `the transcription failed`, amb el que ha dit el motor |

El punter a la guia és `See the manual page "Installing the transcription tools"
(install-transcription).`
