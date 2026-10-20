# `shift`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Move **todas** as posições do ficheiro da mesma quantidade, num sentido ou
no outro. Nenhuma legenda fica no ecrã mais nem menos tempo:
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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--by` | **uma das duas** | uma duração com sinal, ver abaixo | — |
| `--to-grid` | **uma das duas** | indicador: a quantidade é medida, ver abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

O formato do ficheiro lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

## `--to-grid`: a quantidade medida em vez de dada

Um ficheiro cujas posições estão numa grelha de fotogramas **a menos de uma
constante** foi deslocado, e essa constante pode ser medida nas próprias posições.
`--to-grid` mede-a e aplica-a, sem que ninguém precise de a conhecer.

A quantidade é **própria de cada ficheiro**: dois ficheiros deslocados da mesma
grelha por duas quantidades diferentes regressam cada um pela sua. É
escrita na narração, com a taxa de fotogramas encontrada.

**O caminho é o mais curto.** Para além de meio fotograma, a linha da grelha
mais próxima é a seguinte: a correção avança em vez de recuar quase
um fotograma inteiro.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Um ficheiro que não está em nenhuma grelha conhecida é recusado, e diz-se porquê.** Não
há nada a repor, e uma fase medida sobre ruído moveria o ficheiro
ao acaso. Ver [`inspect`](inspect.md#o-que-frame-rate-grid-comunica) para
saber o que um ficheiro dá antes de tentar a operação.

`--by` e `--to-grid` dizem ambos de quanto deslocar: indicá-los em conjunto
é um erro de utilização, e indicar nenhum também.

**`--to-grid` não é [`snap`](snap.md).** Desloca o ficheiro inteiro
da mesma quantidade e mantém a sincronização relativa **exatamente**; `snap`
move cada posição de forma independente e absorve pelo caminho as que tinham
sido corrigidas à mão. Num ficheiro limpo os dois dão o mesmo
resultado; num ficheiro parcial, não.

## Escrever uma duração

Duas formas, e só duas:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de tempo | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Ambas dizem o mesmo: `--by 7.001` e `--by 00:00:07.001` produzem
o mesmo ficheiro.

**O separador decimal é o ponto**, pois a linha de comandos fala inglês.
Uma vírgula é recusada em vez de interpretada: metade do mundo escreve `7,001`
para sete mil e um, e lê-lo como sete segundos seria uma adivinha —
errada para essa metade.

**A resolução é o milissegundo.** O que for mais fino é recusado em vez de arredondado
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

## Um deslocamento anterior à origem é recusado

Recuar para além de zero dá o código `2` e **não escreve nada**. A mensagem nomeia
a legenda que não pôde aceitar o deslocamento, o que diz ao mesmo tempo
até onde se pode ir.

A razão não é um princípio: uma posição anterior à origem escreve-se
`-00:00:01,000`, uma forma que nenhum dos nove formatos define e nenhum leitor
aceita. O núcleo admite tais posições — recusá-las faria de uma
operação de edição um caso especial — mas um ficheiro é outra coisa que não um
modelo em memória.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.subtitles` (todas as legendas do ficheiro) e `counts.shifted_by_ms` (o deslocamento, em milissegundos inteiros, **com sinal**). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` num erro de utilização.

## Erros

| O que o desencadeia | Mensagem |
| :------------------ | :------ |
| vírgula decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| mais fino do que o milissegundo | `"…" is not a time: positions are held to the millisecond, no finer` |
| texto que não é uma duração | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| deslocamento anterior à origem | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nada é escrito sem uma das três opções de destino**, e os seus
erros são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
