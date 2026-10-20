# `join-repeats`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Faz **uma única legenda** das legendas que mostram a mesma coisa umas após as outras: um texto cortado em pedaços
que se seguem, como algumas ferramentas de segmentação o deixam. É o item `Join Repeated Subtitles` da janela —
[o mesmo motor](../subedit-gui/lines.md#unir-legendas-repetidas).

**Uma repetição** é uma legenda que tem **o mesmo texto, a mesma tradução e as mesmas configurações de formato** (um
estilo, uma posição, configurações de cue) que outra, **e que começa, no mais tardar, quando aquela termina**: elas
se tocam ou se sobrepõem. Uma sequência de repetições torna-se **a que começa primeiro**, cujo fim é
levado ao último fim da sequência; as outras desaparecem.

- **A ordem do arquivo não faz diferença, e não é alterada**: a legenda mantida é a que começa primeiro,
  onde quer que estejam as outras. Um arquivo em ordem continua em ordem.
- **Nada é unido por engano**: outro texto, outra tradução, outras configurações, um intervalo entre dois
  pedaços, uma legenda sem texto ou cujo fim vem antes do início — em todos esses casos elas continuam separadas.
- **A fazer antes do resto**: ordenar e [cortar as sobreposições](disjoint.md) leem melhor uma sequência limpa.
  [`repair`](repair.md) encadeia os três, e [`convert`](convert.md#tornar-os-intervalos-disjuntos) o aceita como opção (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Os três pedaços de `Alpha`, que se seguem de 1 s a 6 s, agora formam um só. `Beta`, separada por um intervalo,
continuou como estava. Uma segunda passagem não muda nada e diz isso: `no repeated subtitles`.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, ou `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), as `counts` de cada arquivo: `subtitles` (as do arquivo
gravado), `joined` (as legendas removidas porque outra as cobre) e `chains` (as sequências unidas).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo sem repetições conta —, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

Nenhum específico de `join-repeats`: um arquivo ilegível falha como em qualquer subcomando, e os erros do
destino são comuns aos subcomandos que gravam: veja [Invocação](invocation.md#o-destino).
