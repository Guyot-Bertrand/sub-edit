# `shift`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Move **todas** as posições do arquivo pela mesma quantidade, para um lado ou
para o outro. Nenhuma legenda fica na tela mais ou menos tempo:
é isso que faz de um deslocamento um deslocamento.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **um dos dois** | uma duração com sinal, veja abaixo | — |
| `--to-grid` | **um dos dois** | opção sem valor: a quantidade é medida, veja abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

O formato do arquivo lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

## `--to-grid`: a quantidade medida em vez de informada

Um arquivo cujas posições estão em uma grade de quadros **a menos de uma
constante** foi deslocado, e essa constante pode ser medida nas próprias posições.
`--to-grid` a mede e a aplica, sem que ninguém precise conhecê-la.

A quantidade é **própria de cada arquivo**: dois arquivos deslocados para fora da mesma
grade por duas quantidades diferentes voltam cada um pela sua. Ela é
escrita na narração, junto com a taxa de quadros encontrada.

**O caminho é o mais curto.** Passada a metade de um quadro, a linha da grade
mais próxima é a seguinte: a correção avança em vez de recuar por
quase um quadro inteiro.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Um arquivo em nenhuma grade conhecida é recusado, com o motivo.** Não há
nada a reencontrar, e uma fase medida sobre ruído moveria o arquivo
ao acaso. Veja [`inspect`](inspect.md#o-que-frame-rate-grid-relata) para
saber o que um arquivo oferece antes de tentar a operação.

`--by` e `--to-grid` dizem ambos quanto deslocar: informá-los juntos
é um erro de uso, e informar nenhum dos dois também.

**`--to-grid` não é [`snap`](snap.md).** Ele desloca o arquivo inteiro
pela mesma quantidade e mantém a sincronização relativa **exatamente**; `snap`
move cada posição de forma independente e absorve no caminho aquelas que haviam
sido corrigidas à mão. Em um arquivo limpo os dois dão o mesmo
resultado; em um arquivo parcial, não.

## Escrever uma duração

Duas formas, e somente duas:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de tempo | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

As duas dizem a mesma coisa: `--by 7.001` e `--by 00:00:07.001` produzem
o mesmo arquivo.

**O separador decimal é o ponto**, pois a linha de comando fala inglês.
Uma vírgula é recusada em vez de interpretada: metade do mundo escreve `7,001`
para sete mil e um, e lê-la como sete segundos seria um palpite —
errado para essa metade.

**A resolução é o milissegundo.** Algo mais fino é recusado em vez de arredondado
em silêncio: um arredondamento tácito daria um resultado que não corresponde a nada do que
foi pedido.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Um deslocamento antes da origem é recusado

Recuar além de zero dá o código `2` e **não grava nada**. A mensagem nomeia
a legenda que não pôde receber o deslocamento, o que diz ao mesmo tempo
até onde se pode ir.

O motivo não é um princípio: uma posição antes da origem se escreveria
`-00:00:01,000`, uma forma que nenhum dos nove formatos define e que nenhum leitor
aceita. O núcleo admite tais posições — recusá-las faria de uma operação de
edição um caso especial —, mas um arquivo é algo diferente de um
modelo na memória.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há arquivo, e a linha de narração o diz — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.subtitles` (todas as legendas do arquivo) e `counts.shifted_by_ms` (o deslocamento, em milissegundos inteiros, **com sinal**). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| vírgula decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| mais fino que o milissegundo | `"…" is not a time: positions are held to the millisecond, no finer` |
| texto que não é uma duração | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| deslocamento antes da origem | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nada é gravado sem uma das três opções de destino**, e os erros delas
são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
