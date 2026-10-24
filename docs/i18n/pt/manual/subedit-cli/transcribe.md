# `transcribe`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

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

Escreve **a fala de um vídeo, ou de um ficheiro de áudio, sob a forma de ficheiro de legendas**. O áudio é
preparado, entregue a um motor que encontra o que é dito e quando, e os pedaços que o motor corta são unidos em
legendas que respeitam uma duração e um comprimento.

**Nada do que é necessário é fornecido com o programa**: o motor (whisper.cpp), um modelo e o `ffmpeg`
são instalados pelo utilizador, que indica onde está o modelo. [Instalar as ferramentas de
transcrição](install-transcription.md) diz onde encontrar cada um e que modelo escolher. **A sua ausência não
estraga nada**: o comando diz o que falta, com a remissão para essa página, não escreve nada e
termina com um código diferente de zero.

- **A entrada é um vídeo ou um ficheiro de áudio.** De um vídeo, o `ffmpeg` extrai uma faixa de áudio; sem
  o `ffmpeg`, um ficheiro de áudio que o motor lê tal como está (`wav`, `mp3`, `flac`, `ogg`) é ainda assim
  transcrito, mas um vídeo não, nem nenhum ficheiro quando se pede uma faixa.
- **Um único ficheiro de cada vez**, e o destino é `--output`: não há `--output-dir` nem
  `--in-place`, pois o resultado é um ficheiro novo e não uma reescrita.
- **O idioma é detetado** a menos que `--language` o force, e o idioma utilizado é dito. Um
  modelo feito só para inglês não deteta nada e não transcreve outra coisa: um idioma que não possa
  aceitar é recusado *antes* de qualquer trabalho, indicando o limite do modelo.
- **Os segmentos são unidos e cortados** segundo os limites das definições (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dois pedaços de uma mesma frase que estão
  próximos formam uma só legenda, e um pedaço demasiado longo ou demasiado largo é cortado numa palavra. **`--raw`
  escreve os segmentos do motor tal como chegaram**, um por legenda, o que é também a maneira de ver o que
  faz a união.

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

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<input>` | sim | um vídeo ou um ficheiro de áudio | — |
| `--output` | sim | o ficheiro de legendas a escrever; a sua pasta é criada se faltar | — |
| `--to` | não | o nome curto de um formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | o formato que a extensão de `--output` indica, senão `srt` |
| `--audio-track` | não | um número inteiro a partir de 0: a posição entre as faixas de áudio do vídeo | `0`, a primeira |
| `--language` | não | duas letras minúsculas, um código ISO 639-1: `en`, `fr` | a definição `transcription.language`; vazia: detetado |
| `--model` | não | o ficheiro do modelo | a definição `transcription.model` |
| `--engine` | não | o nome de um motor: `whisper.cpp` | a definição `transcription.engine` |
| `--keep-audio` | não | um ficheiro onde guardar o áudio extraído, em PCM mono de 16 kHz e 16 bits num ficheiro WAV; a sua pasta é criada se faltar | o áudio não é guardado em lado nenhum |
| `--raw` | não | um indicador: escrever os segmentos do motor sem união nem corte | desativado |

As **definições** são lidas do ficheiro de definições como as de [`adjust`](adjust.md) — ver
[o ficheiro de definições](invocation.md#o-ficheiro-de-definições) — e cada uma tem aqui a sua opção:

| Chave | Significado | Predefinição |
| :-- | :------ | :------ |
| `transcription.engine` | o motor | `whisper.cpp` |
| `transcription.executable` | o programa do motor, um caminho absoluto; vazio: procurado no `PATH` | vazio |
| `transcription.model` | o ficheiro do modelo, um caminho absoluto | vazio |
| `transcription.language` | o idioma do áudio, duas letras; vazio: detetado | vazio |
| `transcription.merge-gap-ms` | os pedaços de uma mesma frase mais próximos do que este número de milissegundos são unidos | `400` |
| `transcription.max-duration-ms` | o tempo máximo que uma legenda fica no ecrã, em milissegundos | `7000` |
| `transcription.max-characters` | o número máximo de carateres de uma legenda, quebras de linha não contadas | `84` |

## Escolher o áudio

Um vídeo pode ter várias faixas de áudio — o original, um comentário, outro idioma. `--audio-track`
recebe a **posição entre as faixas de áudio** do ficheiro, a partir de 0, pela ordem em que o ficheiro as lista.
Uma faixa que o vídeo não tem é recusada, **com as que ele tem**, para que a chamada seguinte seja
a certa:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | o progresso do motor, `transcribing: 25%`, tal como ele o comunica — em passos de cerca de trinta segundos de áudio |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler) (a opção global, escrita antes do
subcomando), o objeto do ficheiro traz o idioma utilizado em `language`, `counts.subtitles` (as
legendas escritas) e `counts.segments` (os pedaços do motor de que foram feitas), e `audio` quando
`--keep-audio` guardou o áudio.

## Códigos de saída

Os da ferramenta: `0` se o ficheiro foi escrito, `2` se não pôde ser — uma ferramenta ou um modelo em falta, um
idioma que o modelo não aceita, uma extração ou uma transcrição que falha —, `1` num erro de utilização, como um
motor que ninguém registou.

## Erros

| Causa | Mensagem |
| :---- | :------ |
| nenhum programa de motor encontrado, ou o configurado não existe | `the transcription engine was not found`, depois a remissão para o guia |
| nenhum modelo indicado, ou não existe | `no model was given, or the model file is not there`, depois a remissão para o guia |
| `ffmpeg` não encontrado para um vídeo | `ffmpeg was not found, so the audio of a video cannot be extracted`, depois a remissão |
| uma faixa que o vídeo não tem | `the video has no audio track N`, depois as faixas que tem |
| um idioma que o modelo não sabe transcrever | `the model cannot transcribe the language xx` |
| nenhum idioma, e um modelo que não sabe detetar um | `the model cannot detect the language: give one` |
| um motor que ninguém registou | `no transcription engine is called NAME` |
| qualquer outra falha | `the transcription failed`, com o que o motor disse |

A remissão para o guia é `See the manual page "Installing the transcription tools"
(install-transcription).`
