# `framerate`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

> **Isto não é o [`snap`](snap.md).** Este comando **muda a escala do
> ficheiro**: serve para quando a sincronização está errada e se afasta de vários
> segundos no fim do filme. O `snap` volta a pôr os instantes na grelha mais
> próxima sem ressincronizar nada, e serve para quando a sincronização já está
> certa. Os dois têm a mesma forma, uma taxa de fotogramas ou duas, e confundi-los não provoca
> nenhum erro.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Ressincroniza um ficheiro sincronizado para uma taxa de fotogramas com outra. É
a operação que um ficheiro de legendas pede quando foi obtido para uma cópia que
não é a do filme que se tem — um DVD PAL em vez de uma transferência NTSC, por
exemplo.

O formato do ficheiro lido é **mantido**: mudar de formato é trabalho de
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **sim** | uma taxa de fotogramas, ver abaixo | — |
| `--to` | **sim** | uma taxa de fotogramas, ver abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e comunica, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

`--from` é a taxa de fotogramas para a qual o ficheiro **está** sincronizado, `--to` aquela
para a qual se o quer. Trocá-las desloca o ficheiro no
sentido contrário — cerca de sete segundos num filme de longa-metragem entre 25 e
23,976 — e nada o assinala antes de se ver o filme.

## Escrever uma taxa de fotogramas

Em fotogramas por segundo, decimais incluídos, com o ponto como separador:

```
25      24      23.976      29.97      60
```

**Uma taxa de fotogramas nula ou negativa é recusada**: não é uma taxa de fotogramas.

### As três taxas de fotogramas NTSC não se podem escrever exatamente

`23.976`, `29.97` e `59.94` são **etiquetas**. As taxas de fotogramas que
designam são `24000/1001`, `30000/1001` e `60000/1001`, nenhuma das quais tem
uma escrita decimal finita. A ferramenta reconhece-as como tal:

| O que se escreve | O que designa |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferido para vídeo NTSC |
| `24` | `24` — cinema |
| `25` | `25` — vídeo PAL e SECAM |
| `29.97` | `30000/1001` — vídeo NTSC |
| `30`, `50`, `60` | elas próprias |
| `59.94` | `60000/1001` |

O reconhecimento incide sobre **o valor e não sobre as letras**: `23.9760`
designa a mesma taxa de fotogramas que `23.976`.

Qualquer outro decimal é tomado à letra — `23.9` não é padrão de ninguém,
não há nada a ler nele. É também isso que torna necessária a tabela acima:
sem ela, `23.976` seria `23976/1000`, e a linha de comandos
diria uma coisa diferente da janela para as mesmas palavras.

## O que as posições passam a ser

```
t′ = round( t × (from / to) )
```

Um filme masterizado a 23,976 e reproduzido a 25 fotogramas por segundo corre mais depressa,
pelo que as suas legendas chegam mais cedo; no sentido inverso, chegam
mais tarde.

**O fator é um racional exato, e o arredondamento faz-se uma só vez.** O
caminho que parece equivalente — converter a posição num número de fotograma à
taxa de entrada, depois de novo em milissegundos à taxa de saída — não
o é: quantifica cada posição na grelha de entrada e erra até
meio fotograma, 21 ms medidos. Ver
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Na prática, de 25 para 23,976, o fator é `1001/960`:

| Posição lida | Escrita | O que daria passar pelos fotogramas |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, e depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

**A narração indica a taxa de fotogramas exata, não a etiqueta escrita.** Escrever
`23.976` nessa linha comunicaria uma conversão que não aconteceu; é
o único sítio onde se vê qual das duas foi utilizada. Uma taxa de fotogramas que um
decimal escreve exatamente aparece aí como decimal, as outras como fração.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.subtitles` (todas as legendas do ficheiro). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` perante um erro de utilização.

Nenhuma recusa depende do conteúdo de um ficheiro: duas taxas de fotogramas definem sempre
um fator. É o único dos subcomandos que escrevem cujas recusas
vêm todas antes da primeira leitura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Erros

| O que a desencadeia | Mensagem |
| :------------------ | :------ |
| taxa de fotogramas nula ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| vírgula decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que não é um número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de fotogramas desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nada é escrito sem uma das três opções de destino**, e os respetivos
erros são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
