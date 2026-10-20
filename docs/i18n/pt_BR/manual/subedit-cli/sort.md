# `sort`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Recoloca as legendas **na ordem de seu início**. Nada mais muda: nem os textos, nem as
posições, nem o formato do arquivo. As legendas são **renumeradas** a partir de 1 pela gravação,
como em qualquer arquivo gravado de novo.

**O núcleo nunca ordena por conta própria**: ler um arquivo deixa sua ordem como estava, e
[`inspect`](inspect.md#o-que-anomalies-relata) relata uma quebra de ordem com
`subtitle N starts before the previous one starts`. `sort` é **o gesto feito de propósito**, para um arquivo
ou um lote cuja ordem está quebrada — e pode ser desfeito, na janela, como qualquer operação.

- **A ordenação é estável**: duas legendas que começam no mesmo instante **mantêm a ordem que o
  arquivo lhes dava**. Nenhuma vem antes da outra; movê-las seria uma decisão que ninguém
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

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativado — veja [Processar uma árvore de diretórios](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

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

**A contagem é a dos lugares que mudaram de legenda**, como o núcleo a relata — o que
o histórico da janela chamaria de movido. Uma única legenda fora do lugar desloca várias: no
exemplo acima, nenhum dos três lugares conserva sua legenda, e a frase diz `3 subtitles moved`.

**Um arquivo que já está em ordem é gravado mesmo assim**, e o informa: um destino informado é um
destino gravado, e fazer de `sort` a exceção obrigaria um script a saber quais
subcomandos às vezes produzem um arquivo e às vezes nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há
arquivo, e a linha de narração o diz — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, ou `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz
`counts.subtitles` (todas as legendas do arquivo) e `counts.moved` (os lugares que mudaram;
`0` para um arquivo já em ordem).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo já em ordem conta como processado —, `2` se
nenhum, `3` se apenas alguns, `1` em caso de erro de uso.

## Erros

Nenhum específico de `sort`: um arquivo ilegível falha como em qualquer subcomando, e os
erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
