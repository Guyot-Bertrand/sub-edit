# `join-repeats`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Faz **uma única legenda** das legendas que mostram o mesmo conteúdo umas a seguir às outras: um texto cortado em pedaços
que se sucedem, tal como o deixam algumas ferramentas de segmentação. É o `Join Repeated Subtitles` da janela —
[o mesmo motor](../subedit-gui/lines.md#unir-legendas-repetidas).

**Uma repetição** é uma legenda que tem **o mesmo texto, a mesma tradução e as mesmas definições de formato** (um
estilo, uma posição, definições de cue) que outra, **e que começa, o mais tardar, quando essa termina**: tocam-se
ou sobrepõem-se. Uma sequência de repetições passa a ser **a que começa primeiro**, cujo fim
é levado até ao fim mais tardio da sequência; as outras desaparecem.

- **A ordem do ficheiro não faz diferença, e não é alterada**: a legenda mantida é a que começa primeiro,
  estejam as outras onde estiverem. Um ficheiro ordenado continua ordenado.
- **Nada é unido por engano**: outro texto, outra tradução, outras definições, um espaço entre dois
  pedaços, uma legenda sem texto ou cujo fim vem antes do início — em todos estes casos ficam separadas.
- **Convém fazê-lo antes do resto**: a ordenação e [cortar as sobreposições](disjoint.md) leem melhor uma sequência limpa.
  [`repair`](repair.md) encadeia as três, e [`convert`](convert.md#tornar-os-intervalos-disjuntos) aceita-o como opção (`--join-repeats`, `--repair`).

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

| Opção | Obrigatória | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e comunica, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

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

Os três pedaços de `Alpha`, que se sucedem de 1 s a 6 s, passam a formar apenas um. `Beta`, separado por um espaço,
ficou. Uma segunda passagem não altera nada e di-lo: `no repeated subtitles`.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, ou `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, e depois **cada diagnóstico de leitura** |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), os `counts` de cada ficheiro: `subtitles` (as do ficheiro
escrito), `joined` (as legendas removidas porque outra as cobre) e `chains` (as sequências unidas).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro sem repetições conta como tal —, `2` se nenhum, `3` se
apenas alguns, `1` perante um erro de utilização.

## Erros

Nenhum específico de `join-repeats`: um ficheiro ilegível falha como em qualquer subcomando, e os erros do
destino são comuns aos subcomandos que escrevem: ver [Invocação](invocation.md#o-destino).
