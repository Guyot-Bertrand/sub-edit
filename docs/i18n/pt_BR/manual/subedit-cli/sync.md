# `sync`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

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

**Reajusta o tempo de um arquivo de legendas sobre o áudio de um filme.** O caso para o qual existe é o `.srt` baixado da internet,
deslocado no início e no fim, que um único deslocamento não conserta: cada legenda é procurada no que é dito e
toma o lugar em que é dita.

Ele escuta o áudio com um motor de transcrição — o mesmo de [`transcribe`](transcribe.md) — e **compara
as palavras de cada legenda com as palavras que ouviu**. Nada é traduzido: as legendas precisam estar no idioma
que é falado.

**Nada do que é necessário é fornecido com o programa**: o motor, um modelo e o `ffmpeg` são os de
[Instalar as ferramentas de transcrição](install-transcription.md), e a sua ausência não estraga nada — o comando
diz o que falta, com a remissão para essa página, não escreve nada e termina com um código diferente de zero.

- **Só os inícios e os fins mudam.** Nunca o texto, o estilo, o número de legendas ou a sua ordem.
- **Uma legenda que o áudio não fornece mantém os seus tempos** — um crédito que ninguém diz, uma canção, uma fala em outro idioma.
  **Nenhuma posição é jamais inventada, nem adivinhada a partir das vizinhas**: a linha fica onde o arquivo a tinha,
  e o comando o diz. Isso *não* é uma falha, e o código de saída é `0`.
- **Uma legenda é encontrada quando pelo menos metade das suas palavras é ouvida** (`--min-match`, `alignment.min-match`), e pelo
  menos uma. Maiúsculas, acentos, pontuação e marcação não contam; uma palavra com uma letra de diferença numa palavra longa conta como ouvida.
- **O fim é onde termina a última palavra.** O motor marca o tempo das palavras de forma aproximada, alguns centésimos de segundo antes do
  fim próprio da legenda, de modo que uma legenda pode sair um pouco mais curta do que era.
- **Uma legenda que invadiria a seguinte é recuada até onde a seguinte começa.** Uma que ainda se sobrepõe à
  seguinte, o que o recuo não cura, é deixada e dita.
- **Um arquivo em que nenhuma legenda foi encontrada é recusado e não é escrito**: é quase certamente o áudio errado ou o
  idioma errado.
- **Pode-se escutar só uma parte do filme**, com `--start` e `--end`, como para
  [`transcribe`](transcribe.md#transcrever-uma-parte): somente as legendas que **começam** no intervalo são procuradas e
  tocadas, as outras ficam como estão e não são contadas.
- **O áudio é escutado uma vez para cada arquivo informado.** Para reajustar vários arquivos sobre o mesmo filme, conte com a espera
  por cada um.

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

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :------- | :----- | :------ |
| `<files>` | sim | os arquivos de legenda a reajustar; com `-r`, pastas | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativado — veja [Processar uma árvore de diretórios](lots.md) |
| `--video` | um de `--video` e `--audio` | o filme a escutar | — |
| `--audio` | um de `--video` e `--audio` | um arquivo de áudio a escutar, em vez de um filme | — |
| `--audio-track` | não | um número inteiro a partir de 0: a posição entre as faixas de áudio do filme | `0`, a primeira |
| `--language` | não | duas letras, um código ISO 639-1: `en`, `fr` | a configuração `transcription.language`; vazia: detectado |
| `--start`, `--end` | não | um tempo: `2:00`, `1:02:03`, `2:00.5`, ou um número de segundos; `--end` precisa vir depois de `--start` | o áudio inteiro |
| `--model` | não | o arquivo do modelo | a configuração `transcription.model` |
| `--vad-model` | não | um modelo de atividade de voz, que faz o motor ignorar os silêncios; com ele as palavras seguem mais de perto a fala | a configuração `transcription.vad-model` |
| `--engine` | não | o nome do motor que escuta: `whisper.cpp` | a configuração `transcription.engine` |
| `--keep-audio` | não | um arquivo onde guardar o áudio extraído; sua pasta é criada se faltar | o áudio não é guardado em lugar nenhum |
| `--min-match` | não | um número inteiro de 1 a 100: a parte das palavras de uma legenda que deve ser ouvida, em porcentagem | a configuração `alignment.min-match`, `50` |
| `--doubt-gap` | não | um número inteiro de milissegundos, 0 ou mais: quanto o deslocamento de uma legenda pode diferir do das vizinhas antes de ser sinalizada | a configuração `alignment.doubt-gap`, `2000` |
| `--report` | não | um arquivo onde escrever as legendas a verificar, uma por linha: o número da legenda, uma tabulação e o motivo; um arquivo de legenda de cada vez | nada é escrito |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

O destino — `--output`, `--output-dir`, `--in-place`, `--dry-run` — e `-r` são os dos outros comandos que
reescrevem um arquivo: veja [o destino](invocation.md#o-destino). O arquivo mantém o seu formato, a sua codificação e
as suas quebras de linha.

As **configurações** são lidas do arquivo de configurações — veja [o arquivo de configurações](invocation.md#o-arquivo-de-configurações) — e cada
uma tem aqui a sua opção. O motor, o modelo e o modelo de atividade de voz são os de `transcribe`.

| Chave | Significado | Padrão |
| :-- | :------ | :------ |
| `alignment.engine` | o motor de alinhamento; há um, `transcript-matching`, que escuta e compara | `transcript-matching` |
| `alignment.min-match` | a parte das palavras de uma legenda que deve ser ouvida para que seja encontrada, em porcentagem | `50` |
| `alignment.doubt-gap` | quanto, em milissegundos, o deslocamento de uma legenda pode se afastar do das vizinhas antes de ser sinalizada | `2000` |

## O que é sinalizado

O comando diz quais legendas verificar, e por quê. Uma legenda é sinalizada pelo **primeiro** destes motivos que valer:

| Motivo | Significado |
| :-- | :------ |
| not found in the audio, left as it was | menos de `--min-match` das suas palavras foram ouvidas: ela mantém os seus tempos |
| only partly heard | encontrada, mas menos de 80 % das suas palavras foram ouvidas |
| moved differently from its neighbours | encontrada, mas o seu deslocamento se afasta do deslocamento mediano das duas legendas de cada lado em mais de `--doubt-gap` |

Uma legenda também pode ser **recuada até o início da legenda seguinte**, ou **ainda se sobrepor à legenda seguinte**.

Dois números contam a história do arquivo como um todo. **O deslocamento mediano** é o valor do meio de quanto as legendas se moveram: um
arquivo que estava simplesmente atrasado se lê aqui, e em nenhum outro lugar. **A deriva** é o deslocamento mediano do último décimo das
legendas menos o do primeiro décimo: é próxima de zero para um deslocamento constante, e grande para um arquivo que estava atrasado
cada vez mais ao longo do filme.

## Saída

**Saída padrão** — nada: o resultado é o arquivo escrito.

| Nível | O que se diz na saída de erro |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | a etapa e o progresso, como para [`transcribe`](transcribe.md#saída) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto do arquivo traz `counts`: `subtitles`
(procuradas), `placed` (que receberam um lugar), `moved` (cujos tempos mudaram), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` e `drift_ms`; e `flagged`, uma lista de `{"subtitle": N, "kind": "…"}` em que o número é o
lugar da legenda no arquivo e `kind` é um entre `not-found`, `partial`, `out-of-line`, `cut-back` e `overlaps`.
Os identificadores são prometidos; as frases não.

## Exemplo

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Códigos de saída

Os da ferramenta: `0` se o arquivo foi escrito, com legendas sinalizadas ou não, `2` se não pôde ser — uma ferramenta ou um modelo
ausente, uma extração ou uma escuta que falhou, nenhuma legenda encontrada —, `1` num erro de uso, como nem
`--video` nem `--audio` (ou os dois), uma opção fora do seu intervalo, ou um intervalo que não pode ser usado.

## Erros

| Causa | Mensagem |
| :---- | :------ |
| nem `--video` nem `--audio`, ou os dois | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nenhum programa de motor encontrado, ou o configurado não está lá | `the transcription engine was not found`, depois a remissão para o guia |
| nenhum modelo informado, ou ele não está lá | `no model was given, or the model file is not there`, depois a remissão para o guia |
| `ffmpeg` não encontrado para um filme | `ffmpeg was not found, so the audio of a video cannot be extracted`, depois a remissão |
| uma faixa que o filme não tem | `the video has no audio track N`, depois as faixas que ele tem |
| um idioma que o modelo não pode transcrever | `the model cannot transcribe the language xx` |
| um motor que ninguém registrou | `no transcription engine is called NAME`, ou `there is no alignment engine called NAME` |
| nenhuma das legendas encontrada | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` fora de 1 a 100, `--doubt-gap` negativo | o erro de uso da opção |
| `--report` com mais de um arquivo de legenda | `--report names one file: give one subtitle file when you use it` |
