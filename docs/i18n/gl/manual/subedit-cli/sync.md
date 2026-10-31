# `sync`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

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

**Recalcula os tempos dun ficheiro de subtítulos segundo o son dunha película.** O caso para o que existe é o `.srt`
descargado de internet, desprazado ao principio e ao final, que un único desprazamento non devolve ao seu sitio: cada
subtítulo búscase no que se di e ocupa o lugar onde se di.

Escoita o son cun motor de transcrición — o mesmo ca o de [`transcribe`](transcribe.md) — e **fai coincidir as palabras
de cada subtítulo coas palabras que oíu**. Non se traduce nada: os subtítulos deben estar na lingua que se fala.

**Nada do que necesita vén co programa**: o motor, un modelo e `ffmpeg` son os de
[Instalar as ferramentas de transcrición](install-transcription.md), e a súa ausencia non rompe nada: a orde di o que
falta, co enderezo desa páxina, non escribe nada e sae cun código distinto de cero.

- **Só cambian os inicios e os finais.** Nunca o texto, o estilo, o número de subtítulos nin a súa orde.
- **Un subtítulo que o son non dá conserva os seus tempos** — uns créditos que ninguén di, unha canción, unha liña noutra
  lingua. **Nunca se inventa unha posición, e ningunha se adiviña a partir dos veciños**: a liña queda onde a tiña o
  ficheiro, e a orde dío. *Non* é un fallo, e o código de saída é `0`.
- **Un subtítulo considérase atopado cando se oíu como mínimo a metade das súas palabras** (`--min-match`,
  `alignment.min-match`), e polo menos unha. Non importan as maiúsculas, os acentos, a puntuación nin o marcado; unha
  palabra cunha letra de diferenza nunha palabra longa conta como oída.
- **O final é onde remata a última palabra.** O motor sitúa as palabras de forma aproximada, uns centésimos de segundo
  antes do final propio do subtítulo, polo que un subtítulo pode saír algo máis curto do que era.
- **Un subtítulo que chocaría co seguinte recórtase ata onde comeza o seguinte.** O que aínda se solapa co seguinte, e
  que recortar non pode curar, déixase e dise.
- **Un ficheiro no que non se atopou ningún subtítulo rexéitase e non se escribe**: case seguro que é o son equivocado
  ou a lingua equivocada.
- **Pódese escoitar só unha parte da película**, con `--start` e `--end`, como para
  [`transcribe`](transcribe.md#transcribir-unha-parte): só se buscan e se tocan os subtítulos que **comezan** no
  intervalo, os demais déixanse como están e non se contan.
- **O son escóitase unha vez por cada ficheiro dado.** Para recalcular os tempos de varios ficheiros sobre a mesma
  película, hai que esperar por cada un.

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

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :------- | :----- | :------ |
| `<files>` | si | os ficheiros de subtítulos aos que recalcular os tempos; con `-r`, directorios | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--video` | un de `--video` e `--audio` | a película que escoitar | — |
| `--audio` | un de `--video` e `--audio` | un ficheiro de son que escoitar, en vez dunha película | — |
| `--audio-track` | non | un número enteiro desde 0: a posición entre as pistas de son da película | `0`, a primeira |
| `--language` | non | dúas letras, un código ISO 639-1: `en`, `fr` | o axuste `transcription.language`; baleiro: detectada |
| `--start`, `--end` | non | un tempo: `2:00`, `1:02:03`, `2:00.5`, ou un número de segundos; `--end` debe vir despois de `--start` | todo o son |
| `--model` | non | o ficheiro de modelo | o axuste `transcription.model` |
| `--vad-model` | non | un modelo de actividade de voz, que fai que o motor omita os silencios; con el as palabras seguen máis de preto a fala | o axuste `transcription.vad-model` |
| `--engine` | non | o nome do motor que escoita: `whisper.cpp` | o axuste `transcription.engine` |
| `--keep-audio` | non | un ficheiro onde conservar o son extraído; o seu directorio créase se falta | o son non se conserva en ningures |
| `--min-match` | non | un número enteiro de 1 a 100: a parte das palabras dun subtítulo que se deben oír, en porcentaxe | o axuste `alignment.min-match`, `50` |
| `--doubt-gap` | non | un número enteiro de milisegundos, 0 ou máis: canto pode diferir o desprazamento dun subtítulo do dos veciños antes de ser marcado | o axuste `alignment.doubt-gap`, `2000` |
| `--report` | non | un ficheiro onde escribir os subtítulos que hai que revisar, un por liña: o número do subtítulo, un tabulador e a razón; un ficheiro de subtítulos cada vez | non se escribe nada |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

O destino — `--output`, `--output-dir`, `--in-place`, `--dry-run` — e `-r` son os das outras ordes que reescriben un
ficheiro: véxase [o destino](invocation.md#o-destino). O ficheiro conserva o seu formato, a súa codificación e os seus
finais de liña.

Os **axustes** léense do ficheiro de configuración — véxase
[o ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) — e cada un ten aquí a súa opción. O motor, o
modelo e o modelo de actividade de voz son os de `transcribe`.

| Clave | Significado | Predeterminado |
| :-- | :------ | :------ |
| `alignment.engine` | o motor de aliñamento; hai un, `transcript-matching`, que escoita e fai coincidir | `transcript-matching` |
| `alignment.min-match` | a parte das palabras dun subtítulo que se deben oír para que se atope, en porcentaxe | `50` |
| `alignment.doubt-gap` | canto, en milisegundos, pode afastarse o desprazamento dun subtítulo do dos veciños antes de ser marcado | `2000` |

## O que se marca

A orde di que subtítulos hai que revisar, e por que. Un subtítulo márcase pola **primeira** destas razóns que se dea:

| Por que | Significado |
| :-- | :------ |
| non se atopou no son, deixado como estaba | oíronse menos de `--min-match` das súas palabras: conserva os seus tempos |
| só se oíu en parte | atopado, pero oíronse menos do 80 % das súas palabras |
| moveuse de xeito distinto dos veciños | atopado, pero o seu desprazamento afástase do desprazamento mediano dos dous subtítulos de cada lado en máis de `--doubt-gap` |

Un subtítulo tamén pode ser **recortado ata o inicio do subtítulo seguinte**, ou **aínda solaparse co subtítulo seguinte**.

Dúas cifras contan a historia do ficheiro no seu conxunto. **O desprazamento mediano** é o valor do medio de canto se
moveron os subtítulos: un ficheiro que simplemente ía tarde le aquí, e en ningures máis. **A deriva** é o desprazamento
mediano da última décima parte dos subtítulos menos o da primeira: é case cero para un desprazamento constante, e grande
para un ficheiro que ía tarde cada vez máis ao longo da película.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se di na saída de erro |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | o paso e o progreso, como para [`transcribe`](transcribe.md#saída) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto do ficheiro leva `counts`: `subtitles`
(buscados), `placed` (que recibiron un lugar), `moved` (cuxos tempos cambiaron), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` e `drift_ms`; e `flagged`, unha lista de `{"subtitle": N, "kind": "…"}` onde o número é o lugar do
subtítulo no ficheiro e `kind` é un de `not-found`, `partial`, `out-of-line`, `cut-back` e `overlaps`.
Os ids están garantidos; as frases non.

## Exemplo

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Códigos de saída

Os da ferramenta: `0` se se escribiu o ficheiro, haxa ou non subtítulos marcados, `2` se non se puido — falta unha
ferramenta ou un modelo, unha extracción ou unha escoita fallida, ningún subtítulo atopado —, `1` nun erro de uso, como
ningún dos dous `--video` e `--audio` (ou os dous), unha opción fóra do seu rango, ou un intervalo que non se pode usar.

## Erros

| Causa | Mensaxe |
| :---- | :------ |
| nin `--video` nin `--audio`, ou os dous | `sync needs the audio to listen to: give --video or --audio, and not both` |
| non se atopa o programa do motor, ou o configurado non está | `the transcription engine was not found`, despois o enderezo da guía |
| non se indicou modelo, ou non existe | `no model was given, or the model file is not there`, despois o enderezo da guía |
| non se atopa `ffmpeg` para unha película | `ffmpeg was not found, so the audio of a video cannot be extracted`, despois o enderezo |
| unha pista que a película non ten | `the video has no audio track N`, despois as pistas que ten |
| unha lingua que o modelo non pode transcribir | `the model cannot transcribe the language xx` |
| un motor que ninguén rexistrou | `no transcription engine is called NAME`, ou `there is no alignment engine called NAME` |
| ningún subtítulo atopado | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` fóra de 1 a 100, `--doubt-gap` negativo | o erro de uso da opción |
| `--report` con máis dun ficheiro de subtítulos | `--report names one file: give one subtitle file when you use it` |
