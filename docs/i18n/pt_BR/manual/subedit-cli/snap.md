# `snap`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Coloca **todas as marcas de tempo** — início e fim — no quadro mais próximo da
taxa de quadros informada. Nada é ressincronizado: cada posição se move meio quadro
no máximo.

## `snap` não é `framerate`, e confundi-los não aparece

Os dois comandos têm a mesma forma — arquivos, um destino, uma taxa de quadros — e fazem coisas opostas.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| o que calcula | `t' = t x input rate / output rate` | o quadro mais próximo, na grade de `--rate` |
| quanto move | em proporção ao tempo — **vários segundos** em um longa-metragem | meio quadro no máximo, **em toda parte** |
| para qual arquivo | um arquivo cuja **sincronização está errada** | um arquivo cuja sincronização está **certa** e cuja grade está errada |

**O caso de uso de `snap`**, por extenso: um filme a 25 quadros por
segundo, um arquivo de legendas escrito em uma grade a 24, com as linhas já
mais ou menos no lugar, a poucos milissegundos. Não há nada a ressincronizar
— apenas as marcas de tempo a recolocar nos quadros certos.

**O caso de uso de `framerate`** é o outro: um arquivo sincronizado para uma matriz a
25 que se quer reproduzir em uma cópia a 23,976, e cuja sincronização deriva em
vários segundos ao final do filme.

Tomar um pelo outro **não causa nenhum erro**: ambos
aceitam o arquivo e gravam um resultado. A linha de narração é o que
os distingue — `snap` sempre diz quanto moveu as posições.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativado — veja [Processar uma árvore de diretórios](lots.md) |
| `--rate` | **sim** | uma taxa de quadros em quadros por segundo, escrita como em [`framerate`](framerate.md#escrever-uma-taxa-de-quadros) | — |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

`--rate` aceita **qualquer taxa de quadros válida**, e não apenas as oito
padrão: é escolha do usuário, como em `framerate`. Uma taxa de quadros nula ou negativa
é recusada.

O formato do arquivo lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há arquivo, e a linha de narração o diz — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

**Os dois números são o ponto da linha.** Meio quadro a 25 quadros por
segundo são vinte milissegundos, e nada maior pode ser escrito ali. Quem
queria `framerate` e digitou `snap` lê "by at most 20 ms" onde esperava
segundos, e percebe de imediato.

## Um exemplo

Uma linha em `00:00:01,010`, alinhada a 25 quadros por segundo — portanto a um
múltiplo de quarenta milissegundos:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Duas propriedades

**Alinhar duas vezes não muda nada na segunda.** Uma posição que já está em um
quadro permanece nele: é a mesma operação aplicada ao mesmo resultado.

**A ordem das legendas é mantida.** Arredondar para o quadro mais próximo é
monotônico, de modo que dois inícios separados por pelo menos um quadro continuam assim. Mais
próximos, podem se fundir — o que é uma anomalia que
[`inspect`](inspect.md#o-que-anomalies-relata) sabe nomear, e não uma
inversão.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.subtitles`, `counts.moved` (as posições movidas) e `counts.furthest_ms` (o maior deslocamento, em milissegundos). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

Os de uma taxa de quadros, palavra por palavra os de [`framerate`](framerate.md#erros)
— ambos leem o mesmo valor pelo mesmo caminho:

| O que o provoca | Mensagem |
| :------------------ | :------ |
| taxa de quadros nula ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| vírgula decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que não é um número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de quadros absurda | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nada é gravado sem uma das três opções de destino**, e os erros delas
são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
