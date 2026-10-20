# `snap`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Põe **todas as marcas de tempo** — início e fim — no fotograma mais próximo da
taxa de fotogramas indicada. Nada é resincronizado: cada posição move-se, no
máximo, meio fotograma.

## `snap` não é `framerate`, e confundi-los não se nota

Os dois comandos têm a mesma forma — ficheiros, um destino, uma taxa de fotogramas — e fazem coisas opostas.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| o que calcula | `t' = t x input rate / output rate` | o fotograma mais próximo, na grelha de `--rate` |
| de quanto move | proporcionalmente ao tempo — **vários segundos** num filme de longa duração | meio fotograma no máximo, **em todo o lado** |
| para que ficheiro | um ficheiro cuja **sincronização está errada** | um ficheiro cuja sincronização está **certa** e cuja grelha está errada |

**O caso de uso de `snap`**, explicitado: um filme a 25 fotogramas por
segundo, um ficheiro de legendas escrito numa grelha a 24, com as linhas já
mais ou menos no seu lugar, a poucos milissegundos. Não há nada a resincronizar
— apenas a repor as marcas de tempo nos fotogramas certos.

**O caso de uso de `framerate`** é o outro: um ficheiro sincronizado para uma matriz a
25 que se quer reproduzir numa cópia a 23,976, e cuja sincronização se desvia
vários segundos no fim do filme.

Tomar um pelo outro **não causa nenhum erro**: ambos
aceitam o ficheiro e escrevem um resultado. A linha de narração é o que
os distingue — `snap` diz sempre de quanto moveu as posições.

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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--rate` | **sim** | uma taxa de fotogramas em fotogramas por segundo, escrita como em [`framerate`](framerate.md#escrever-uma-taxa-de-fotogramas) | — |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

`--rate` aceita **qualquer taxa de fotogramas válida**, e não apenas as oito
habituais: a escolha é do utilizador, como em `framerate`. Uma taxa de
fotogramas nula ou negativa é recusada.

O formato do ficheiro lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

**Os dois números são o essencial da linha.** Meio fotograma a 25 fotogramas por
segundo são vinte milissegundos, e nada maior pode ser escrito aí. Quem
queria `framerate` e escreveu `snap` lê “by at most 20 ms” onde esperava
segundos, e percebe logo.

## Um exemplo

Uma linha em `00:00:01,010`, alinhada a 25 fotogramas por segundo — ou seja, num
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

**Alinhar duas vezes não altera nada à segunda vez.** Uma posição que já está num
fotograma fica lá: é a mesma operação aplicada ao mesmo resultado.

**A ordem das legendas é mantida.** O arredondamento para o fotograma mais próximo é
monótono, pelo que dois inícios separados por pelo menos um fotograma continuam assim. Mais
próximos, podem fundir-se — o que é uma anomalia que
[`inspect`](inspect.md#o-que-anomalies-comunica) sabe nomear, e não uma
inversão.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.subtitles`, `counts.moved` (as posições movidas) e `counts.furthest_ms` (a maior deslocação, em milissegundos). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` num erro de utilização.

## Erros

Os de uma taxa de fotogramas, palavra por palavra os de [`framerate`](framerate.md#erros)
— ambos leem o mesmo valor pelo mesmo caminho:

| O que o desencadeia | Mensagem |
| :------------------ | :------ |
| taxa de fotogramas nula ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| vírgula decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que não é um número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de fotogramas desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nada é escrito sem uma das três opções de destino**, e os seus erros
são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
