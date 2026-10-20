# `framerate`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

> **Isto não é [`snap`](snap.md).** Este comando **muda a escala
> do arquivo**: serve para quando a sincronização está errada e se afasta vários
> segundos ao final do filme. `snap` devolve as marcas de tempo à grade mais
> próxima sem ressincronizar nada, e serve para quando a sincronização já está
> certa. Os dois têm a mesma forma, uma taxa de quadros ou duas, e confundi-los não provoca
> nenhum erro.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Ressincroniza um arquivo sincronizado para uma taxa de quadros em outra. É
a operação de que um arquivo de legendas precisa quando foi obtido para uma cópia que
não é a do filme que se tem — um DVD PAL em relação a uma transferência NTSC, por
exemplo.

O formato do arquivo lido é **mantido**: mudar o formato é trabalho de
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

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--from` | **sim** | uma taxa de quadros, veja abaixo | — |
| `--to` | **sim** | uma taxa de quadros, veja abaixo | — |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

`--from` é a taxa de quadros para a qual o arquivo **está** sincronizado, `--to` aquela
para a qual se o quer. Trocá-las desloca o arquivo no
sentido oposto — cerca de sete segundos em um longa-metragem entre 25 e
23.976 — e nada o sinaliza antes de se assistir ao filme.

## Escrever uma taxa de quadros

Em quadros por segundo, decimais incluídos, com o ponto como separador:

```
25      24      23.976      29.97      60
```

**Uma taxa de quadros zero ou negativa é recusada**: não é uma taxa de quadros.

### As três taxas de quadros NTSC não podem ser escritas exatamente

`23.976`, `29.97` e `59.94` são **rótulos**. As taxas de quadros que eles
designam são `24000/1001`, `30000/1001` e `60000/1001`, e nenhuma delas tem
escrita decimal finita. A ferramenta as reconhece como tais:

| O que se escreve | O que designa |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferido para vídeo NTSC |
| `24` | `24` — cinema |
| `25` | `25` — vídeo PAL e SECAM |
| `29.97` | `30000/1001` — vídeo NTSC |
| `30`, `50`, `60` | elas mesmas |
| `59.94` | `60000/1001` |

O reconhecimento vale para **o valor e não para as letras**: `23.9760`
designa a mesma taxa de quadros que `23.976`.

Qualquer outro decimal é tomado literalmente — `23.9` não é padrão de ninguém,
não há nada a ler nele. É isso também que torna a tabela acima
necessária: sem ela, `23.976` seria `23976/1000`, e a linha de comando
diria algo diferente da janela para as mesmas palavras.

## No que as posições se tornam

```
t′ = round( t × (from / to) )
```

Um filme masterizado a 23.976 e reproduzido a 25 quadros por segundo corre mais rápido,
e suas legendas chegam, portanto, mais cedo; no sentido oposto, chegam
mais tarde.

**O fator é um racional exato, e o arredondamento acontece uma única vez.** O
caminho que parece equivalente — converter a posição em número de quadro na
taxa de entrada e depois de volta em milissegundos na taxa de saída — não
o é: ele quantiza cada posição na grade de entrada e erra em até
meio quadro, 21 ms medidos. Veja
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Na prática, de 25 para 23.976, o fator é `1001/960`:

| Posição lida | Escrita | O que daria passar pelos quadros |
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

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há arquivo, e a linha de narração diz isso — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

**A narração nomeia a taxa de quadros exata, não o rótulo digitado.** Escrever
`23.976` nessa linha relataria uma conversão que não aconteceu; é
o único lugar onde se vê qual das duas foi usada. Uma taxa de quadros que
um decimal escreve exatamente aparece ali como decimal, as outras como fração.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.subtitles` (todas as legendas do arquivo). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

Nenhuma recusa depende do conteúdo de um arquivo: duas taxas de quadros sempre definem
um fator. É o único dos subcomandos que gravam cujas
recusas todas vêm antes da primeira leitura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| taxa de quadros zero ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| vírgula decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que não é um número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de quadros desproporcional | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nada é gravado sem uma das três opções de destino**, e seus
erros são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
