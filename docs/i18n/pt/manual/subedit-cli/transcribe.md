# `transcribe`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **Um idioma digita-se com duas letras — `fr`, `en`, `de` —, ou com o código de três letras que um ficheiro declara, `fra`,
  `eng`, lido como o de duas letras.** Tudo o resto é recusado antes de qualquer trabalho. **A deteção
  ouve apenas os primeiros trinta segundos**: um filme que abre com música ou silêncio pode ser tomado por
  outro idioma, e o motor escreve então a fala *traduzida* para esse idioma — um filme francês tomado
  por inglês sai em inglês, ou como `[SPEAKING FRENCH]`. Quando souber o idioma, indique-o.
- **Pode-se transcrever só uma parte do filme**, com `--start` e `--end` — veja
  [Transcrever uma parte](#transcrever-uma-parte). É assim que se experimenta um modelo em dois minutos antes de o lançar
  em duas horas.
- **Os idiomas que um motor oferece podem ser listados**, com `--list-languages` — veja
  [Listar os idiomas](#listar-os-idiomas).
- **Por predefinição, os sons não são escritos como notas**: o motor recebe a ordem de o não fazer (`--suppress-notes`; a sua própria opção é
  `--suppress-nst`), de modo que `(Sonnerie de téléphone)` ou `[MUSIC PLAYING]` não surjam nas legendas, nem entre parênteses nem entre
  retos. `--keep-notes` escreve-os, para legendas para surdos e deficientes auditivos. Proibidas as notas, o motor escreve `...` para uma passagem
  sem fala, e **o pós-processamento descarta os pedaços que não têm nem letra nem algarismo** — `...`, `…`, `♪` — antes de unir e cortar; `--raw` conserva-os,
  tal como os pedaços do motor vieram.
- **Os segmentos são unidos e cortados** segundo os limites das definições (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dois pedaços de uma mesma frase que estão
  próximos formam uma só legenda, e um pedaço demasiado longo ou demasiado largo é cortado — primeiro num fim de frase, depois numa vírgula, num ponto e vírgula ou nos dois pontos, e a meio de uma frase só quando não há pontuação onde cortar —, evitando um corte que deixe uma legenda demasiado pequena para ser lida. Os três passos fazem-se por ordem, cada um sobre toda a transcrição: descartar, unir, cortar. Uma legenda que começa um pedaço mantém o seu início, e a que o termina mantém o seu fim. **`--raw`
  escreve os segmentos do motor tal como chegaram**, um por legenda, o que é também a maneira de ver o que
  faz a união.

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
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<input>` | sim, exceto com `--list-languages` | um vídeo ou um ficheiro de áudio | — |
| `--output` | sim, exceto com `--list-languages` | o ficheiro de legendas a escrever; a sua pasta é criada se faltar | — |
| `--to` | não | o nome curto de um formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | o formato que a extensão de `--output` indica, senão `srt` |
| `--audio-track` | não | um número inteiro a partir de 0: a posição entre as faixas de áudio do vídeo | `0`, a primeira |
| `--language` | não | duas letras, um código ISO 639-1: `en`, `fr`; um código de três letras como `fra` é lido como `fr` | a definição `transcription.language`; vazia: detetado |
| `--suppress-notes` | não | uma opção sem valor: não escrever os sons como notas; recusada para um motor a quem isso não se pode dizer | a definição `transcription.suppress-notes` |
| `--keep-notes` | não | uma opção sem valor: escrevê-los, contra a definição; o oposto de `--suppress-notes`, e as duas juntas são recusadas | a definição `transcription.suppress-notes` |
| `--list-languages` | não | uma opção sem valor: listar os idiomas que o motor oferece para o modelo, e não transcrever nada | desativada |
| `--start` | não | um tempo: `2:00`, `1:02:03`, `2:00.5`, ou um número de segundos, `90` | o início do áudio |
| `--end` | não | um tempo, escrito como para `--start`; tem de vir depois de `--start` | o fim do áudio |
| `--model` | não | o ficheiro do modelo | a definição `transcription.model` |
| `--vad-model` | não | um modelo de atividade de voz (um modelo Silero para o whisper.cpp), que faz o motor ignorar os silêncios antes de descodificar: os seus segmentos seguem então a fala, e o primeiro começa onde a fala começa em vez de em 0. O ficheiro tem de existir, e um motor que não o possa usar recusa a opção pelo seu nome | a definição `transcription.vad-model` |
| `--engine` | não | o nome de um motor: `whisper.cpp` | a definição `transcription.engine` |
| `--keep-audio` | não | um ficheiro onde guardar o áudio extraído, como PCM mono de 16 bits num ficheiro WAV, à taxa de amostragem que o motor pede (16 kHz para whisper.cpp); a sua pasta é criada se faltar, e o nome pode levar `{rate}`, substituído por essa taxa em hertz (`film.{rate}.wav` escreve-se `film.16000.wav`) | o áudio não é guardado em lado nenhum |
| `--raw` | não | um indicador: escrever os segmentos do motor sem união nem corte | desativado |

As **definições** são lidas do ficheiro de definições como as de [`adjust`](adjust.md) — ver
[o ficheiro de definições](invocation.md#o-ficheiro-de-definições) — e cada uma tem aqui a sua opção:

| Chave | Significado | Predefinição |
| :-- | :------ | :------ |
| `transcription.engine` | o motor | `whisper.cpp` |
| `transcription.executable` | o programa do motor, um caminho absoluto; vazio: procurado no `PATH` | vazio |
| `transcription.model` | o ficheiro do modelo, um caminho absoluto | vazio |
| `transcription.vad-model` | o modelo de atividade de voz, um caminho absoluto; vazio: nenhum, e os silêncios não são ignorados. Uma definição que um motor não pode usar é descartada sem uma palavra | vazio |
| `transcription.language` | o idioma do áudio, duas letras; vazio: detetado | vazio |
| `transcription.suppress-notes` | não escrever os sons como notas | `true` |
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

## Transcrever uma parte

`--start` e `--end` limitam o trabalho à parte do filme entre os dois. Qualquer um pode ser dado sozinho: um
`--start` sozinho vai até ao fim, um `--end` sozinho começa no início.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Um tempo escreve-se `M:SS` ou `H:MM:SS`, ambos com uma parte decimal depois de um ponto (`2:00.5`), ou como um número de
  segundos (`90`, `90.5`)**. Uma vírgula é recusada, como em todo o lado na ferramenta.
- **As legendas mantêm o tempo do filme**: a fala dita aos 2:10 do filme é escrita em `00:02:10`, não em
  `00:00:10`. O ficheiro pode ser colocado tal como está sobre o filme inteiro.
- **Os limites são aproximados**: o motor trabalha em janelas de trinta segundos, por isso o que devolve é cortado
  ao intervalo depois; uma frase que fica a cavalo num limite é cortada aí.
- **Nada começa se o intervalo não puder ser usado**: um limite que não é um tempo, ou um `--end` que não vem depois de
  `--start`, é um erro de utilização que nomeia o problema, não escreve nada e não lança nem o `ffmpeg` nem o
  motor.

O áudio do filme inteiro é extraído na mesma: é a transcrição, muito mais longa, que o intervalo poupa.

## Listar os idiomas

`--list-languages` escreve os idiomas que o motor oferece **para o modelo**, um por linha: o código que `--language` aceita, uma tabulação, e o nome
do idioma no idioma da interface. Não precisa de entrada nem de saída, e não transcreve nada.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

A lista depende do motor e do modelo: uma centena de idiomas para um modelo whisper.cpp multilingue, só `en` para um modelo só para
inglês. Um motor que não sabe listar os seus idiomas não lista nenhum. Um modelo não indicado ou ausente é um erro, com a remissão para o guia; apenas a
forma de texto, nada é escrito com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler).

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | a etapa e o progresso, uma linha por cento: `extracting the audio…`, depois `transcribing…`, depois `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — a percentagem, a posição atingida no áudio, o tempo que falta e o tempo decorrido; **a linha é dita de novo a cada dez segundos quando nada de novo chegou**, para que uma espera longa não seja tomada por um bloqueio |
| 3 | e cada novo texto que o motor obtém, `last text: …` |

**A percentagem é a posição do motor no áudio, não uma contagem das suas próprias janelas**: o motor escreve o que transcreveu à medida que avança, e o comando lê até onde isso chegou, de modo que o valor se mexe desde o primeiro minuto. (O progresso próprio do motor vem em passos de 5 % do áudio, depois de uma janela de trinta segundos: sozinho, pode ficar a 0 % durante um quarto de hora num filme longo com um modelo grande num processador.) **O tempo que falta fica mudo no início**: é dado quando há trabalho feito que chegue para extrapolar, e segue a velocidade do último minuto, não a média de toda a execução.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler) (a opção global, escrita antes do
subcomando), o objeto do ficheiro traz o idioma utilizado em `language`, `counts.subtitles` (as
legendas escritas) e `counts.segments` (os pedaços do motor de que foram feitas), e `audio` quando
`--keep-audio` guardou o áudio.

## Avisos

Um aviso é dito no primeiro nível, antes do trabalho, porque muda o que há a esperar:

- **Com um modelo só para inglês**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Um modelo cujo nome de ficheiro termina
  em `.en` é um deles.

## Códigos de saída

Os da ferramenta: `0` se o ficheiro foi escrito, `2` se não pôde ser — uma ferramenta ou um modelo em falta, um
idioma que o modelo não aceita, uma extração ou uma transcrição que falha —, `1` num erro de utilização, como um
motor que ninguém registou ou um intervalo que não pode ser usado.

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
| `<input>` ou `--output` em falta | `input is required` ou `--output is required` |
| `--suppress-notes` com `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` para um motor a quem isso não se pode dizer | `the engine NAME cannot be told not to write sounds as notes` |
| um idioma que não é um código | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| um limite do intervalo que não é um tempo | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` não depois de `--start` | `the end of the range must come after its start` |
| qualquer outra falha | `the transcription failed`, com o que o motor disse |

A remissão para o guia é `See the manual page "Installing the transcription tools"
(install-transcription).`
