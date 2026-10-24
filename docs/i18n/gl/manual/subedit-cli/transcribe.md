# `transcribe`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

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

Escribe **a fala dun vídeo, ou dun ficheiro de son, como ficheiro de subtítulos**. O son prepárase,
entrégase a un motor que atopa o que se di e cando, e os anacos que corta únense en subtítulos
que respectan unha duración e unha lonxitude.

**Nada do que necesita vén co programa**: o motor (whisper.cpp), un modelo e `ffmpeg`
instálaos a persoa usuaria, que indica onde está o modelo. [Instalar as ferramentas de
transcrición](install-transcription.md) di onde atopar cada un e que modelo escoller. **A súa ausencia non
rompe nada**: a orde di o que falta, co enderezo desa páxina, non escribe nada
e sae cun código distinto de cero.

- **A entrada é un vídeo ou un ficheiro de son.** Dun vídeo, `ffmpeg` extrae unha pista de son; sen
  `ffmpeg`, un ficheiro de son que o motor le tal cal (`wav`, `mp3`, `flac`, `ogg`) aínda se
  transcribe, pero un vídeo non, e tampouco calquera ficheiro cando se pide unha pista.
- **Só un ficheiro de cada vez**, e o destino é `--output`: non hai `--output-dir` nin
  `--in-place`, porque o resultado é un ficheiro novo e non unha reescritura.
- **A lingua detéctase** a non ser que `--language` a force, e dise a lingua usada. Un
  modelo feito só para inglés non detecta nada e non transcribe outra cousa: unha lingua que non pode
  aceptar rexéitase *antes* de empezar calquera traballo, nomeando o límite do modelo.
- **Os segmentos únense e córtanse** segundo os límites dos axustes (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dous anacos dunha mesma frase que están
  próximos son un só subtítulo, e un anaco demasiado longo ou demasiado ancho córtase nunha palabra. **`--raw`
  escribe os segmentos do motor tal como saíron**, un por subtítulo, que tamén é a maneira de ver o que fai
  a unión.

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

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :------- | :----- | :------ |
| `<input>` | si | un vídeo ou un ficheiro de son | — |
| `--output` | si | o ficheiro de subtítulos que escribir; o seu directorio créase se falta | — |
| `--to` | non | o nome curto dun formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | o formato que indica a extensión de `--output`, se non `srt` |
| `--audio-track` | non | un número enteiro desde 0: a posición entre as pistas de son do vídeo | `0`, a primeira |
| `--language` | non | dúas letras minúsculas, un código ISO 639-1: `en`, `fr` | o axuste `transcription.language`; baleiro: detectada |
| `--model` | non | o ficheiro de modelo | o axuste `transcription.model` |
| `--engine` | non | o nome dun motor: `whisper.cpp` | o axuste `transcription.engine` |
| `--keep-audio` | non | un ficheiro onde conservar o son extraído, como PCM mono de 16 bits a 16 kHz nun ficheiro WAV; o seu directorio créase se falta | o son non se conserva en ningún lugar |
| `--raw` | non | unha marca: escribir os segmentos do motor sen unir nin cortar | desactivada |

Os **axustes** léense do ficheiro de configuración como os de [`adjust`](adjust.md) — véxase
[O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) — e cada un ten aquí a súa opción:

| Clave | Significado | Predeterminado |
| :-- | :------ | :------ |
| `transcription.engine` | o motor | `whisper.cpp` |
| `transcription.executable` | o programa do motor, unha ruta absoluta; baleiro: búscase no `PATH` | baleiro |
| `transcription.model` | o ficheiro de modelo, unha ruta absoluta | baleiro |
| `transcription.language` | a lingua do son, dúas letras; baleiro: detectada | baleiro |
| `transcription.merge-gap-ms` | os anacos dunha mesma frase máis próximos ca estes milisegundos únense | `400` |
| `transcription.max-duration-ms` | o máximo que un subtítulo permanece en pantalla, en milisegundos | `7000` |
| `transcription.max-characters` | o máximo de caracteres dun subtítulo, sen contar os saltos de liña | `84` |

## Escoller o son

Un vídeo pode levar varias pistas de son — a orixinal, un comentario, outra lingua. `--audio-track`
toma a **posición entre as pistas de son** do ficheiro, desde 0, na orde en que o ficheiro as lista. Unha
pista que o vídeo non ten rexéitase, **coas pistas que si ten**, para que a seguinte chamada
sexa a correcta:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | o progreso do motor, `transcribing: 25%`, tal como o comunica — a pasos duns trinta segundos de son |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script) (a opción global, escrita antes da
subordre), o obxecto do ficheiro leva a lingua usada en `language`, `counts.subtitles` (os
subtítulos escritos) e `counts.segments` (os anacos do motor cos que se fixeron), e `audio` cando
`--keep-audio` conservou o son.

## Códigos de saída

Os da ferramenta: `0` se se escribiu o ficheiro, `2` se non se puido — falta unha ferramenta ou un modelo, unha
lingua que o modelo non pode aceptar, unha extracción ou unha transcrición fallida —, `1` nun erro de uso, como un
motor que ninguén rexistrou.

## Erros

| Causa | Mensaxe |
| :---- | :------ |
| non se atopa o programa do motor, ou o configurado non está | `the transcription engine was not found`, despois o enderezo da guía |
| non se indicou modelo, ou non existe | `no model was given, or the model file is not there`, despois o enderezo da guía |
| non se atopa `ffmpeg` para un vídeo | `ffmpeg was not found, so the audio of a video cannot be extracted`, despois o enderezo |
| unha pista que o vídeo non ten | `the video has no audio track N`, despois as pistas que ten |
| unha lingua que o modelo non pode transcribir | `the model cannot transcribe the language xx` |
| sen lingua, e un modelo que non pode detectala | `the model cannot detect the language: give one` |
| un motor que ninguén rexistrou | `no transcription engine is called NAME` |
| calquera outro fallo | `the transcription failed`, co que dixo o motor |

O enderezo da guía é `See the manual page "Installing the transcription tools"
(install-transcription).`
