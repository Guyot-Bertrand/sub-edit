# `transcribe`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Grava **a fala de um vídeo, ou de um arquivo de áudio, sob a forma de arquivo de legendas**. O áudio é
preparado, entregue a um motor que encontra o que é dito e quando, e os pedaços que o motor corta são unidos em
legendas que respeitam uma duração e um comprimento.

**Nada do que é necessário é fornecido com o programa**: o motor (whisper.cpp), um modelo e o `ffmpeg`
são instalados pelo usuário, que indica onde está o modelo. [Instalar as ferramentas de
transcrição](install-transcription.md) diz onde encontrar cada um e que modelo escolher. **A sua ausência não
estraga nada**: o comando diz o que falta, com a remissão para essa página, não escreve nada e
termina com um código diferente de zero.

- **A entrada é um vídeo ou um arquivo de áudio.** De um vídeo, o `ffmpeg` extrai uma faixa de áudio; sem
  o `ffmpeg`, um arquivo de áudio que o motor lê tal como está (`wav`, `mp3`, `flac`, `ogg`) é ainda assim
  transcrito, mas um vídeo não, nem nenhum arquivo quando se pede uma faixa.
- **Um único arquivo de cada vez**, e o destino é `--output`: não há `--output-dir` nem
  `--in-place`, pois o resultado é um arquivo novo e não uma reescrita.
- **O idioma é detectado** a menos que `--language` o force, e o idioma usado é dito. Um
  modelo feito só para inglês não detecta nada e não transcreve outra coisa: um idioma que não possa
  aceitar é recusado *antes* de qualquer trabalho, indicando o limite do modelo.
- **Um idioma se digita com duas letras — `fr`, `en`, `de` —, ou com o código de três letras que um arquivo declara, `fra`,
  `eng`, lido como o de duas letras.** Tudo o resto é recusado antes de qualquer trabalho. **A detecção
  ouve apenas os primeiros trinta segundos**: um filme que abre com música ou silêncio pode ser tomado por
  outro idioma, e o motor escreve então a fala *traduzida* para esse idioma — um filme francês tomado
  por inglês sai em inglês, ou como `[SPEAKING FRENCH]`. Quando você souber o idioma, informe-o.
- **Pode-se transcrever só uma parte do filme**, com `--start` e `--end` — veja
  [Transcrever uma parte](#transcrever-uma-parte). É assim que se experimenta um modelo em dois minutos antes de o lançar
  em duas horas.
- **Os sons não são escritos como notas**: o motor recebe ordem de não fazer isso (a opção própria dele é `--suppress-nst`), de modo
  que `(Sonnerie de téléphone)` ou `[MUSIC PLAYING]` não apareçam nas legendas, entre parênteses como entre colchetes.
- **Os segmentos são unidos e cortados** segundo os limites das configurações (`transcription.merge-gap-ms`,
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
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<input>` | sim | um vídeo ou um arquivo de áudio | — |
| `--output` | sim | o arquivo de legendas a escrever; sua pasta é criada se faltar | — |
| `--to` | não | o nome curto de um formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | o formato que a extensão de `--output` indica, senão `srt` |
| `--audio-track` | não | um número inteiro a partir de 0: a posição entre as faixas de áudio do vídeo | `0`, a primeira |
| `--language` | não | duas letras, um código ISO 639-1: `en`, `fr`; um código de três letras como `fra` é lido como `fr` | a configuração `transcription.language`; vazia: detectado |
| `--start` | não | um tempo: `2:00`, `1:02:03`, `2:00.5`, ou um número de segundos, `90` | o início do áudio |
| `--end` | não | um tempo, escrito como para `--start`; precisa vir depois de `--start` | o fim do áudio |
| `--model` | não | o arquivo do modelo | a configuração `transcription.model` |
| `--engine` | não | o nome de um motor: `whisper.cpp` | a configuração `transcription.engine` |
| `--keep-audio` | não | um arquivo onde guardar o áudio extraído, em PCM mono de 16 kHz e 16 bits num arquivo WAV; sua pasta é criada se faltar | o áudio não é guardado em lugar nenhum |
| `--raw` | não | um indicador: escrever os segmentos do motor sem união nem corte | desativado |

As **configurações** são lidas do arquivo de configurações como as de [`adjust`](adjust.md) — veja
[o arquivo de configurações](invocation.md#o-arquivo-de-configurações) — e cada uma tem aqui a sua opção:

| Chave | Significado | Padrão |
| :-- | :------ | :------ |
| `transcription.engine` | o motor | `whisper.cpp` |
| `transcription.executable` | o programa do motor, um caminho absoluto; vazio: procurado no `PATH` | vazio |
| `transcription.model` | o arquivo do modelo, um caminho absoluto | vazio |
| `transcription.language` | o idioma do áudio, duas letras; vazio: detectado | vazio |
| `transcription.merge-gap-ms` | os pedaços de uma mesma frase mais próximos do que este número de milissegundos são unidos | `400` |
| `transcription.max-duration-ms` | o tempo máximo que uma legenda fica na tela, em milissegundos | `7000` |
| `transcription.max-characters` | o número máximo de caracteres de uma legenda, quebras de linha não contadas | `84` |

## Escolher o áudio

Um vídeo pode ter várias faixas de áudio — o original, um comentário, outro idioma. `--audio-track`
recebe a **posição entre as faixas de áudio** do arquivo, a partir de 0, pela ordem em que o arquivo as lista.
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
`--start` sozinho vai até o fim, um `--end` sozinho começa no início.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Um tempo se escreve `M:SS` ou `H:MM:SS`, ambos com uma parte decimal depois de um ponto (`2:00.5`), ou como um número de
  segundos (`90`, `90.5`)**. Uma vírgula é recusada, como em todo lugar na ferramenta.
- **As legendas mantêm o tempo do filme**: a fala dita aos 2:10 do filme é escrita em `00:02:10`, não em
  `00:00:10`. O arquivo pode ser colocado tal como está sobre o filme inteiro.
- **Os limites são aproximados**: o motor trabalha em janelas de trinta segundos, por isso o que devolve é cortado
  ao intervalo depois; uma frase que fica a cavalo num limite é cortada aí.
- **Nada começa se o intervalo não puder ser usado**: um limite que não é um tempo, ou um `--end` que não vem depois de
  `--start`, é um erro de uso que nomeia o problema, não escreve nada e não lança nem o `ffmpeg` nem o
  motor.

O áudio do filme inteiro é extraído na mesma: é a transcrição, muito mais longa, que o intervalo poupa.

## Saída

**Saída padrão** — nada: o resultado é o arquivo escrito.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | a etapa e o progresso, uma linha por cento: `extracting the audio…`, depois `transcribing…`, depois `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — a porcentagem, a posição atingida no áudio, o tempo que falta e o tempo decorrido; **a linha é dita de novo a cada dez segundos quando nada de novo chegou**, para que uma espera longa não seja tomada por um bloqueio |
| 3 | e cada novo texto que o motor obtém, `last text: …` |

**A porcentagem é a posição do motor no áudio, não uma contagem das suas próprias janelas**: o motor escreve o que transcreveu à medida que avança, e o comando lê até onde isso chegou, de modo que o valor se mexe desde o primeiro minuto. (O progresso próprio do motor vem em passos de 5 % do áudio, depois de uma janela de trinta segundos: sozinho, pode ficar a 0 % durante um quarto de hora num filme longo com um modelo grande num processador.) **O tempo que falta fica mudo no início**: é dado quando há trabalho feito que chegue para extrapolar, e segue a velocidade do último minuto, não a média de toda a execução.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler) (a opção global, escrita antes do
subcomando), o objeto do arquivo traz o idioma usado em `language`, `counts.subtitles` (as
legendas escritas) e `counts.segments` (os pedaços do motor de que foram feitas), e `audio` quando
`--keep-audio` guardou o áudio.

## Avisos

Dois avisos são ditos no primeiro nível, porque mudam o que esperar:

- **Antes do trabalho, com um modelo só para inglês**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Um modelo cujo nome de arquivo termina
  em `.en` é um.
- **Depois do trabalho, quando o motor escreveu sobretudo notas entre colchetes** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —,
  pelo menos dois e um terço dos seus fragmentos: `N of the M pieces are only notes in brackets, such as [MUSIC PLAYING]: the
  language used (xx) is probably not the one spoken, and the engine may have translated the speech. Run it again with
  the language typed.` As legendas são escritas mesmo assim; o aviso diz o que suspeitar.

## Códigos de saída

Os da ferramenta: `0` se o arquivo foi escrito, `2` se não pôde ser — uma ferramenta ou um modelo em falta, um
idioma que o modelo não aceita, uma extração ou uma transcrição que falha —, `1` num erro de uso, como um
motor que ninguém registrou ou um intervalo que não pode ser usado.

## Erros

| Causa | Mensagem |
| :---- | :------ |
| nenhum programa de motor encontrado, ou o configurado não existe | `the transcription engine was not found`, depois a remissão para o guia |
| nenhum modelo indicado, ou não existe | `no model was given, or the model file is not there`, depois a remissão para o guia |
| `ffmpeg` não encontrado para um vídeo | `ffmpeg was not found, so the audio of a video cannot be extracted`, depois a remissão |
| uma faixa que o vídeo não tem | `the video has no audio track N`, depois as faixas que tem |
| um idioma que o modelo não sabe transcrever | `the model cannot transcribe the language xx` |
| nenhum idioma, e um modelo que não sabe detectar um | `the model cannot detect the language: give one` |
| um motor que ninguém registrou | `no transcription engine is called NAME` |
| um idioma que não é um código | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| um limite do intervalo que não é um tempo | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` não depois de `--start` | `the end of the range must come after its start` |
| qualquer outra falha | `the transcription failed`, com o que o motor disse |

A remissão para o guia é `See the manual page "Installing the transcription tools"
(install-transcription).`
