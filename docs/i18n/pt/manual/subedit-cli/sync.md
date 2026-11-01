# `sync`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Reajusta o tempo de um ficheiro de legendas pelo som de um filme.** O caso para que existe é o `.srt` obtido na
internet, deslocado no início e no fim, que um único deslocamento não repõe. Dois métodos o fazem, e `--method` (ou a
definição `alignment.method`) escolhe entre eles:

- **`speech`, o predefinido, assenta o ficheiro nos momentos em que alguém fala.** Pergunta ao som apenas onde há fala, e
  encontra o deslocamento que põe as legendas de acordo com ela, troço de filme a troço de filme: um desvio constante, uma
  deriva, uma segunda metade atrasada e um ficheiro criado para outra taxa de fotogramas (25 em vez de 23,976, e afins)
  ficam todos corrigidos. **Não depende do idioma** — as legendas podem estar em francês e o filme em inglês —, não precisa
  de transcrição, e demora segundos. Precisa do `ffmpeg`, do programa `whisper-vad-speech-segments` que vem com o
  whisper.cpp e de um modelo de atividade de voz (`--vad-model`, a definição `transcription.vad-model`).
- **`words` faz corresponder as palavras de cada legenda às palavras que um motor ouviu.** Escuta com um motor de
  transcrição — o mesmo que [`transcribe`](transcribe.md) — e cada legenda fica no lugar onde as suas palavras são ditas.
  Nada é traduzido: as legendas têm de estar no idioma que se fala. Consegue colocar uma única legenda fora do lugar num
  ficheiro que de resto está certo, e demora o tempo de uma transcrição.

**O que `speech` não faz é dito de cada vez que corre**, na saída de erros e no registo: corrige o ficheiro **no seu
conjunto**, nunca uma legenda de cada vez. Uma legenda fora do lugar por si só num ficheiro que de resto está certo, ou um
título mostrado sobre um silêncio (um cartão de capítulo, uma legenda que traduz algo no ecrã), não é procurada e pode
continuar errada, e os primeiros minutos podem ficar desajustados quando o filme abre com música ou títulos. Verifique o
resultado, ou use `--method words` quando o texto está no idioma que se fala.

**Nada do que é necessário é fornecido com o programa**: os programas, o modelo e o `ffmpeg` são os de
[Instalar as ferramentas de transcrição](install-transcription.md), e a sua ausência não estraga nada — o comando diz
o que falta, com a remissão para essa página, não escreve nada e termina com um código diferente de zero.

- **Só mudam os inícios e os fins.** Nunca o texto, o estilo, o número de legendas nem a sua ordem.
- **Uma legenda que o áudio não dá mantém os seus tempos** — um crédito que ninguém diz, uma canção, uma fala noutro idioma.
  **Nenhuma posição é inventada, e nenhuma é adivinhada a partir das vizinhas**: a linha fica onde o ficheiro a tinha,
  e o comando di-lo. *Não* é uma falha, e o código de saída é `0`.
- **Com `words`, uma legenda é encontrada quando pelo menos metade das suas palavras é ouvida** (`--min-match`,
  `alignment.min-match`), e pelo menos uma. Maiúsculas, acentos, pontuação e etiquetas não contam; uma palavra com uma letra de
  diferença numa palavra longa conta como ouvida. `speech` coloca todas as legendas, e não aceita nenhuma de `--language`,
  `--model`, `--engine` nem `--min-match`.
- **O fim é onde termina a última palavra.** O motor situa as palavras de forma aproximada, alguns centésimos de segundo antes do
  fim da própria legenda, pelo que uma legenda pode sair ligeiramente mais curta do que era.
- **Uma legenda que passaria para cima da seguinte é recuada até onde a seguinte começa.** Uma que ainda se sobrepõe à
  seguinte, o que o recuo não cura, é deixada e dita.
- **Um ficheiro que não se ajusta ao som é recusado e não é escrito**: com `words`, nenhuma das legendas foi encontrada, o
  que é quase sempre o áudio errado ou o idioma errado — `speech` não depende do idioma; com `speech`, as legendas e a fala
  têm demasiado pouco em comum, o que é quase sempre o filme errado.
- **Pode-se escutar só uma parte do filme**, com `--start` e `--end`, como para
  [`transcribe`](transcribe.md#transcrever-uma-parte): só as legendas que **começam** no intervalo são procuradas e
  alteradas, as outras ficam como estão e não são contadas.
- **O áudio é escutado uma vez por cada ficheiro indicado.** Para reajustar vários ficheiros pelo mesmo filme, conte com
  esperar por cada um.

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
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :------- | :----- | :------ |
| `<files>` | sim | os ficheiros de legendas a reajustar; com `-r`, pastas | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--video` | um de `--video` e `--audio` | o filme a escutar | — |
| `--audio` | um de `--video` e `--audio` | um ficheiro de áudio a escutar, em vez de um filme | — |
| `--audio-track` | não | um número inteiro a partir de 0: a posição entre as faixas de áudio do filme | `0`, a primeira |
| `--language` | não | duas letras, um código ISO 639-1: `en`, `fr` | a definição `transcription.language`; vazia: detetado |
| `--start`, `--end` | não | um tempo: `2:00`, `1:02:03`, `2:00.5`, ou um número de segundos; `--end` tem de vir depois de `--start` | todo o áudio |
| `--model` | não | o ficheiro do modelo | a definição `transcription.model` |
| `--vad-model` | não | um modelo de atividade de voz, que faz o motor ignorar os silêncios; com ele, as palavras seguem mais de perto a fala | a definição `transcription.vad-model` |
| `--engine` | não | o nome do motor que escuta: `whisper.cpp` | a definição `transcription.engine` |
| `--keep-audio` | não | um ficheiro onde guardar o áudio extraído; a sua pasta é criada se faltar | o áudio não é guardado em lado nenhum |
| `--method` | não | `speech`: assenta o ficheiro nos momentos em que há fala, em qualquer idioma; `words`: faz corresponder as palavras ao que foi ouvido, no idioma falado | a definição `alignment.method`, `speech` |
| `--min-match` | não | um número inteiro de 1 a 100: a parte das palavras de uma legenda que tem de ser ouvida, em percentagem | a definição `alignment.min-match`, `50` |
| `--doubt-gap` | não | um número inteiro de milissegundos, 0 ou mais: quanto o deslocamento de uma legenda pode diferir do das vizinhas antes de ser assinalada | a definição `alignment.doubt-gap`, `2000` |
| `--report` | não | um ficheiro onde escrever as legendas a verificar, uma por linha: o número da legenda, uma tabulação, e o motivo; um ficheiro de legendas de cada vez | nada é escrito |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

O destino — `--output`, `--output-dir`, `--in-place`, `--dry-run` — e `-r` são os dos outros comandos que
reescrevem um ficheiro: ver [o destino](invocation.md#o-destino). O ficheiro mantém o seu formato, a sua codificação e
as suas quebras de linha.

As **definições** são lidas do ficheiro de definições — ver [o ficheiro de definições](invocation.md#o-ficheiro-de-definições) — e cada
uma tem aqui a sua opção. O motor, o modelo e o modelo de atividade de voz são os de `transcribe`.

| Chave | Significado | Predefinição |
| :-- | :------ | :------ |
| `alignment.engine` | o motor de alinhamento do método `words`; há um, `transcript-matching`, que escuta e faz corresponder | `transcript-matching` |
| `alignment.method` | como as legendas são postas em linha: `speech` ou `words` | `speech` |
| `alignment.min-match` | a parte das palavras de uma legenda que tem de ser ouvida para ela ser encontrada, em percentagem | `50` |
| `alignment.doubt-gap` | quanto, em milissegundos, o deslocamento de uma legenda se pode afastar do das vizinhas antes de ser assinalada | `2000` |

## O que é assinalado

O comando diz quais as legendas a verificar, e porquê. Uma legenda é assinalada pelo **primeiro** destes motivos que se verifique:

| Porquê | Significado |
| :-- | :------ |
| not found in the audio, left as it was | foram ouvidas menos de `--min-match` das suas palavras: mantém os seus tempos |
| only partly heard | encontrada, mas foram ouvidas menos de 80 % das suas palavras |
| moved differently from its neighbours | encontrada, mas o seu deslocamento afasta-se do deslocamento mediano das duas legendas de cada lado em mais de `--doubt-gap` |

Com `speech` todas as legendas são colocadas, e só o último destes motivos pode aplicar-se: onde o ficheiro muda o seu deslocamento em
mais do que `--doubt-gap` de uma legenda para a seguinte (uma segunda metade atrasada), as legendas de cada lado do degrau são
assinaladas.

Uma legenda também pode ser **recuada até ao início da legenda seguinte**, ou **continuar a sobrepor-se à legenda seguinte**.

Duas cifras contam a história do ficheiro no seu conjunto. **O deslocamento mediano** é o valor do meio de quanto as legendas se
moveram: um ficheiro que estava simplesmente atrasado lê-se aqui, e em mais lado nenhum. **A deriva** é o deslocamento mediano do
último décimo das legendas menos o do primeiro décimo: é quase zero para um desvio constante, e grande para um ficheiro que estava
cada vez mais atrasado ao longo do filme.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se diz na saída de erros |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` — e, quando uma taxa de fotogramas foi corrigida, a razão; com `speech`, **o aviso do que não faz**, numa linha própria |
| 2 | a etapa e o progresso, como para [`transcribe`](transcribe.md#saída) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto do ficheiro traz `counts`: `subtitles`
(procuradas), `placed` (às quais foi dado um lugar), `moved` (cujos tempos mudaram), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` e `drift_ms`; e `flagged`, uma lista de `{"subtitle": N, "kind": "…"}` em que o número é o
lugar da legenda no ficheiro e `kind` é um de `not-found`, `partial`, `out-of-line`, `cut-back` e `overlaps`.
Com `speech`, `warnings` traz uma entrada de `kind` `speech-only`. Os identificadores são prometidos; as frases não.

## Exemplo

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Para colocar cada legenda pelas suas palavras, com um texto no idioma falado:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Códigos de saída

Os da ferramenta: `0` se o ficheiro foi escrito, com ou sem legendas assinaladas, `2` se não pôde ser — uma ferramenta ou um modelo
em falta, uma extração ou uma escuta que falha, um ficheiro que não se ajusta ao som (nenhuma das suas legendas encontrada, ou
demasiado pouco em comum com a fala) —, `1` num erro de utilização, como nem `--video` nem `--audio` (ou ambos), uma opção
fora do seu intervalo, ou um intervalo que não pode ser usado.

## Erros

| Causa | Mensagem |
| :---- | :------ |
| nem `--video` nem `--audio`, ou ambos | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nenhum programa de motor encontrado, ou o configurado não existe | `the transcription engine was not found`, depois a remissão para o guia |
| nenhum modelo indicado, ou não existe | `no model was given, or the model file is not there`, depois a remissão para o guia |
| `ffmpeg` não encontrado para um filme | `ffmpeg was not found, so the audio of a video cannot be extracted`, depois a remissão |
| uma faixa que o filme não tem | `the video has no audio track N`, depois as faixas que tem |
| um idioma que o modelo não sabe transcrever | `the model cannot transcribe the language xx` |
| um motor que ninguém registou | `no transcription engine is called NAME`, ou `there is no alignment engine called NAME` |
| nenhuma das legendas encontrada (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, depois `The method based on speech does not depend on the language.` |
| o modelo de atividade de voz não indicado, ou o programa que encontra a fala não encontrado (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` ou `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, depois a remissão para o guia |
| as legendas e a fala têm demasiado pouco em comum (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` fora de 1 a 100, `--doubt-gap` negativo | o erro de utilização da opção |
| `--report` com mais de um ficheiro de legendas | `--report names one file: give one subtitle file when you use it` |
