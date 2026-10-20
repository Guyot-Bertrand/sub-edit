# Processar uma árvore de diretórios

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Todo subcomando que lê um lote de arquivos — `inspect` incluído, e não [`append`](append.md), [`split-file`](split-file.md) nem [`pair`](pair.md), cuja aridade é a de um par ou
de um filme — aceita um **diretório** no lugar de um arquivo, com `--recursive` (`-r`).
Esta página diz o que a varredura abrange, em que ordem, e onde ela grava.

**Nada do que segue muda o que é feito com um arquivo**: cada um é lido,
processado e gravado de forma independente, a falha de um não interrompe os outros, e os
quatro códigos de saída são os de [a invocação](invocation.md#códigos-de-saída).
A varredura produz uma lista de caminhos, que o restante do lote consome como se
tivessem sido digitados.

## Sem `--recursive`, um diretório é recusado

Erro de uso, código `1`, nenhum arquivo processado nem gravado:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## O que a varredura abrange

A varredura é **recursiva, determinística e sem surpresas**:

- **a ordem** — as entradas de cada diretório são tomadas na ordem de seus
  nomes, **byte a byte** (maiúsculas antes de minúsculas, qualquer que seja o idioma
  da máquina), e um subdiretório é percorrido em seu lugar, em profundidade. Duas execuções dão os mesmos bytes, em todas
  as máquinas;
- **links simbólicos não são seguidos** — nem para um arquivo, nem para um
  diretório: uma árvore que contivesse um link para um de seus pais não teria
  fim;
- **arquivos e diretórios ocultos são ignorados** — aqueles cujo nome começa com
  um ponto;
- **só as extensões dos formatos conhecidos são mantidas**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sem distinção de maiúsculas e minúsculas. **`.txt` fica de fora**: ele
  nomeia dois formatos, mas sobretudo todo `LISEZMOI.txt` do mundo, e um lote que
  falhasse em cada um deles seria inutilizável.

**Um arquivo nomeado na linha de comando nunca é filtrado.** Nomeá-lo é
querê-lo: `subedit-cli shift --by 1 --in-place notes.txt` o processa, qualquer que seja
sua extensão, e só reclama se o seu conteúdo não estiver em nenhum formato.

**O que foi deixado de lado é contado**, e informado no nível `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Um diretório que não contém nada a percorrer informa isso no nível padrão
(`<directory>: holds no file in a format this tool walks for`) e não é um erro.

## Onde a varredura grava

**A árvore é mantida sob `--output-dir`**: o caminho de um arquivo,
**relativo ao diretório informado**, é reproduzido sob o diretório de saída. O nome
do diretório informado não faz parte dele — é a semântica do `rsync` com uma
barra final, não a do `cp -r`: nomeia-se o destino e acrescenta-se a ele o
nível desejado (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

O diretório de saída e os da árvore são **criados** se não existirem. Com
`--in-place`, cada arquivo mantido é gravado de volta em seu lugar. Uma extensão que
`convert` altera também muda na árvore: `films/a/x.srt` convertido
para WebVTT é gravado como `sortie/a/x.vtt`.

**O diretório de saída, se estiver dentro da árvore percorrida, é excluído da varredura**:
uma segunda execução não lê de novo o que a primeira gravou.

## Dois diretórios e colisões

A regra de [o destino](invocation.md#o-destino) decide: dois arquivos
que acabariam no mesmo caminho são recusados **antes de qualquer gravação**, código `1`, com as
duas entradas e o destino nomeados.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## O que isso custa

O lote continua sequencial: sua duração é a soma das de seus arquivos. O
benchmark `make bench` mede a varredura e o mesmo trabalho em um único arquivo; os números
estão no [registro de desempenho](../../../../mesures/performances.md).
