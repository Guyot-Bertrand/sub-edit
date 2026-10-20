# `sort`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Repõe as legendas **por ordem do seu início**. Nada mais muda: nem os textos, nem as
posições, nem o formato do ficheiro. As legendas são **renumeradas** a partir de 1 pela escrita,
como em qualquer ficheiro escrito de volta.

**O núcleo nunca ordena por iniciativa própria**: ler um ficheiro deixa a sua ordem como estava, e
[`inspect`](inspect.md#o-que-anomalies-comunica) comunica uma quebra de ordem com
`subtitle N starts before the previous one starts`. `sort` é **o gesto feito de propósito**, para um ficheiro
ou um lote cuja ordem está quebrada — e pode ser anulado, na janela, como qualquer operação.

- **A ordenação é estável**: duas legendas que começam no mesmo instante **mantêm a ordem que o
  ficheiro lhes deu**. Nenhuma passa à frente da outra; movê-las seria uma decisão que ninguém
  pediu.
- **`sort` não aceita nem intervalo nem documento**: não há nada a escolher, a ordem é a dos inícios.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## O que é contado

**A contagem é a dos lugares que mudaram de legenda**, tal como o núcleo a comunica — o que
o histórico da janela chamaria movido. Uma única legenda fora do lugar desloca várias: no
exemplo acima, nenhum dos três lugares conserva a sua legenda, e a frase diz `3 subtitles moved`.

**Um ficheiro já ordenado é escrito na mesma**, e di-lo: um destino indicado é um
destino escrito, e fazer de `sort` a exceção obrigaria um script a saber quais
subcomandos produzem por vezes um ficheiro e por vezes nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há
ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, ou `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz
`counts.subtitles` (todas as legendas do ficheiro) e `counts.moved` (os lugares que mudaram;
`0` para um ficheiro já ordenado).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro já ordenado
conta —, `2` se nenhum, `3` se apenas alguns, `1` num erro de utilização.

## Erros

Nenhum específico de `sort`: um ficheiro ilegível falha como em qualquer subcomando, e os
erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
