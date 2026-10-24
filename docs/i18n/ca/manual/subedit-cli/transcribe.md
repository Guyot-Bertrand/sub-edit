# `transcribe`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

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
- **Els segments es fusionen i es tallen** segons els límits dels paràmetres (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dos trossos d'una mateixa frase que són
  propers fan un sol subtítol, i un tros massa llarg o massa ample es talla en una paraula. **`--raw`
  escriu els segments del motor tal com han vingut**, un per subtítol, que és també la manera de veure què
  fa la fusió.

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

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :------- | :----- | :------ |
| `<input>` | sí | un vídeo o un fitxer d'àudio | — |
| `--output` | sí | el fitxer de subtítols que cal escriure; el seu directori es crea si falta | — |
| `--to` | no | el nom curt d'un format: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | el format que diu l'extensió de `--output`, si no `srt` |
| `--audio-track` | no | un nombre enter des de 0: el lloc entre les pistes d'àudio del vídeo | `0`, la primera |
| `--language` | no | dues lletres minúscules, un codi ISO 639-1: `en`, `fr` | el paràmetre `transcription.language`; buit: es detecta |
| `--model` | no | el fitxer de model | el paràmetre `transcription.model` |
| `--engine` | no | el nom d'un motor: `whisper.cpp` | el paràmetre `transcription.engine` |
| `--keep-audio` | no | un fitxer on conservar l'àudio extret, com a PCM mono de 16 kHz i 16 bits en un fitxer WAV; el seu directori es crea si falta | l'àudio no es conserva enlloc |
| `--raw` | no | un indicador: escriu els segments del motor sense fusionar ni tallar | desactivat |

Els **paràmetres** es llegeixen del fitxer de configuració com els d'[`adjust`](adjust.md) — vegeu
[el fitxer de configuració](invocation.md#el-fitxer-de-configuració) — i cadascun té aquí la seva opció:

| Clau | Significat | Per defecte |
| :-- | :------ | :------ |
| `transcription.engine` | el motor | `whisper.cpp` |
| `transcription.executable` | el programa del motor, un camí absolut; buit: es cerca al `PATH` | buit |
| `transcription.model` | el fitxer de model, un camí absolut | buit |
| `transcription.language` | la llengua de l'àudio, dues lletres; buit: es detecta | buit |
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

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | el progrés del motor, `transcribing: 25%`, tal com l'informa — a intervals d'uns trenta segons d'àudio |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script) (l'opció global, escrita abans de la
subordre), l'objecte del fitxer porta la llengua emprada a `language`, `counts.subtitles` (els
subtítols escrits) i `counts.segments` (els trossos del motor a partir dels quals s'han fet), i `audio` quan
`--keep-audio` ha conservat l'àudio.

## Codis de sortida

Els de l'eina: `0` si s'ha escrit el fitxer, `2` si no s'ha pogut — una eina o un model que falta, una
llengua que el model no pot acceptar, una extracció o una transcripció fallida —, `1` en un error d'ús, com
un motor que ningú no ha registrat.

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
| qualsevol altre error | `the transcription failed`, amb el que ha dit el motor |

El punter a la guia és `See the manual page "Installing the transcription tools"
(install-transcription).`
